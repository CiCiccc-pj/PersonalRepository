#include "nodetreeview.h"                      // 本类头文件

#include "nodeitemdelegate.h"                  // 自定义代理：分组框绘制

#include <QtCore/QMimeData>                    // 拖拽携带的数据
#include <QtCore/QStringList>                  // 分类显示顺序表
#include <QtGui/QDrag>                         // 拖拽操作
#include <QtGui/QFont>                         // 字体设置
#include <QtGui/QFontMetrics>                  // 字体尺寸测量
#include <QtGui/QPainter>                      // 绘制拖拽预览图
#include <QtGui/QPixmap>                       // 拖拽预览图载体
#include <QtGui/QStandardItemModel>            // 树项数据模型
#include <QtNodes/NodeDelegateModelRegistry>   // 节点注册表
#include <QtNodes/StyleCollection>             // 画布主题样式

#include <algorithm>                           // std::sort
#include <map>                                 // 按分类归组
#include <vector>                              // 存放类型名

namespace {
int const kPanelPadding = 4;                   // 面板内边距（像素）
int const kItemFontSize = 14;                  // 节点项字号（磅）

// 分类的显示顺序权重：数值越小越靠前，未列出的分类排在最后
int categoryRank(QString const &category)
{
    static QStringList const order{QStringLiteral("流程节点"),   // 顺序表：流程节点置顶
                                   QStringLiteral("输出节点"),   // 其次输出节点
                                   QStringLiteral("显示节点"),   // 再次显示节点
                                   QStringLiteral("下位机节点")}; // 最后下位机节点
    int const index = order.indexOf(category);                 // 查顺序表
    return index < 0 ? order.size() : index;                   // 不在表中则排到最后
}

// 节点项的显示顺序权重：流程节点按“开始 → 判断 → 循环 → 休眠”排列，其余排最后
int nodeRank(QString const &typeName)
{
    static QStringList const order{QStringLiteral("开始节点"),   // 流程起点
                                   QStringLiteral("判断节点"),   // 分支
                                   QStringLiteral("循环节点"),   // 循环
                                   QStringLiteral("休眠节点")};  // 延时
    int const index = order.indexOf(typeName);                 // 查顺序表
    return index < 0 ? order.size() : index;                   // 不在表中则排到最后
}
} // namespace

// 构造函数：初始化树模型、视图行为与主题样式
NodeTreeView::NodeTreeView(QWidget *parent)
    : QTreeView(parent)                                      // 构造基类
    , _model(new QStandardItemModel(this))                   // 创建树项数据模型
{
    setModel(_model);                                        // 绑定数据模型
    setItemDelegate(new NodeItemDelegate(this));             // 使用自定义代理画分组框

    QFont itemFont = font();                                 // 取当前字体
    itemFont.setPointSize(kItemFontSize);                    // 放大字号
    setFont(itemFont);                                       // 应用到面板

    // —— 视角：看起来是分组的列表，而不是可展开的目录树 ——
    setHeaderHidden(true);                                   // 不显示表头
    setRootIsDecorated(false);                               // 不画展开箭头
    setIndentation(0);                                       // 子项与分类左对齐
    setItemsExpandable(false);                               // 禁止手动展开/收起
    setExpandsOnDoubleClick(false);                          // 双击也不展开
    setAnimated(false);                                      // 关闭展开动画
    setUniformRowHeights(false);                             // 分类行与节点行高度不同

    setDragEnabled(true);                                    // 允许拖拽
    setDragDropMode(QAbstractItemView::DragOnly);            // 仅拖出、不接受拖入
    setDefaultDropAction(Qt::CopyAction);                    // 默认复制语义
    setSelectionMode(QAbstractItemView::SingleSelection);    // 单选
    setEditTriggers(QAbstractItemView::NoEditTriggers);      // 禁止编辑
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);    // 关闭横向滚动条
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // 纵向按像素滚动
    setFocusPolicy(Qt::NoFocus);                             // 不抢占焦点
    viewport()->setMouseTracking(true);                      // 开启悬停跟踪（代理据此高亮）

    applyTheme();                                            // 应用画布同款主题
}

// 拖拽使用的 MIME 类型标识
char const *NodeTreeView::nodeMimeType()
{
    return "application/x-qtnodes-node-type";                // 自定义 MIME 名称
}

// 设置注册表并刷新分类与节点项
void NodeTreeView::setRegistry(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry)
{
    _registry = std::move(registry);                         // 保存注册表
    reload();                                                // 重新生成分类与节点项
}

// 依据注册表重新生成分类与节点项
void NodeTreeView::reload()
{
    _model->clear();                                         // 清空旧项

    if (!_registry)                                          // 未设置注册表
        return;                                              // 直接返回

    // 注册表底层是哈希表，这里先按分类归组并排序，让面板顺序稳定
    auto const &categoryOf = _registry->registeredModelsCategoryAssociation(); // 类型名 → 分类名
    std::map<QString, std::vector<QString>> grouped;         // 分类 → 该分类下的类型名
    for (auto const &entry : _registry->registeredModelCreators()) { // 遍历已注册类型
        QString const &typeName = entry.first;               // 类型名
        auto const it = categoryOf.find(typeName);           // 查它的分类
        QString const category = (it == categoryOf.end())    // 没有分类信息时
                                     ? QStringLiteral("未分类") // 归入“未分类”
                                     : it->second;           // 否则用注册时的分类
        grouped[category].push_back(typeName);               // 归入对应分类
    }

    // 分类顺序：流程节点置顶，其余按顺序表 / 名称排
    std::vector<QString> categories;                         // 分类名列表
    categories.reserve(grouped.size());                      // 预留空间
    for (auto const &pair : grouped)                         // 遍历归组结果
        categories.push_back(pair.first);                    // 收集分类名
    std::sort(categories.begin(), categories.end(),          // 按显示顺序排序
              [](QString const &a, QString const &b) {       // 比较两个分类
                  int const ra = categoryRank(a);            // a 的顺序权重
                  int const rb = categoryRank(b);            // b 的顺序权重
                  return ra != rb ? ra < rb : a < b;         // 权重小的靠前，同权重按名称
              });

    for (QString const &category : categories) {             // 逐个分类建一行
        std::vector<QString> types = grouped[category];      // 该分类下的类型名
        std::sort(types.begin(), types.end(),                // 分类内排序
                  [](QString const &a, QString const &b) {   // 比较两个类型名
                      int const ra = nodeRank(a);            // a 的显示权重
                      int const rb = nodeRank(b);            // b 的显示权重
                      return ra != rb ? ra < rb : a < b;     // 开始节点置顶，其余按名称
                  });

        auto *categoryItem = new QStandardItem(category);    // 分类项
        categoryItem->setEditable(false);                    // 禁止编辑
        categoryItem->setFlags(Qt::ItemIsEnabled);           // 分类行不可选、不可拖
        _model->appendRow(categoryItem);                     // 作为顶层行加入模型

        for (QString const &typeName : types) {              // 逐个加入该分类的节点项
            auto *item = new QStandardItem(typeName);        // 以类型名作为显示文本
            item->setData(typeName, Qt::UserRole);           // 附带存类型名供拖拽使用
            item->setEditable(false);                        // 禁止编辑该项
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled); // 项能力
            categoryItem->appendRow(item);                   // 挂到分类下面
        }
    }

    expandAll();                                             // 全部分类展开，所有节点项都可见
}

// 开始拖拽：把节点类型名写入 MIME 并交给画布
void NodeTreeView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);                              // 未使用该参数

    QModelIndex const index = currentIndex();                // 取当前项
    if (!index.isValid() || !index.parent().isValid())       // 无效项或分类行
        return;                                              // 分类行不允许拖拽

    QString const nodeType = index.data(Qt::UserRole).toString(); // 读出节点类型名
    if (nodeType.isEmpty())                                  // 类型名为空
        return;                                              // 直接返回

    auto *mimeData = new QMimeData;                          // 创建 MIME 数据
    mimeData->setData(nodeMimeType(), nodeType.toUtf8());    // 写入节点类型名

    auto *drag = new QDrag(this);                            // 创建拖拽对象
    drag->setMimeData(mimeData);                             // 绑定 MIME 数据
    drag->setPixmap(makeDragPixmap(index.data(Qt::DisplayRole).toString())); // 拖拽预览图
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2)); // 抓取点居中
    drag->exec(Qt::CopyAction);                              // 执行拖拽
}

// 应用与画布一致的主题配色
void NodeTreeView::applyTheme()
{
    // 与画布保持同一套主题色，使左侧面板与画布背景融为一体
    QString const panelColor = QtNodes::StyleCollection::flowViewStyle().BackgroundColor.name(); // 面板背景色

    // 样式表：只负责面板底色与留白，行内容全部由代理绘制
    setStyleSheet(QStringLiteral(R"(
        QTreeView {
            background-color: %1;                            /* 面板背景 */
            border: none;                                    /* 无边框 */
            outline: none;                                   /* 无焦点虚线框 */
            padding: %2px;                                   /* 内边距 */
        }
    )")
                      .arg(panelColor,                           // %1 面板背景
                           QString::number(kPanelPadding)));     // %2 内边距
}

// 生成拖拽时的预览图（与面板中的节点项同款样式）
QPixmap NodeTreeView::makeDragPixmap(QString const &text) const
{
    QColor const fineGridColor = QtNodes::StyleCollection::flowViewStyle().FineGridColor; // 取细网格色

    QFont font = this->font();                               // 使用面板字体
    QFontMetrics const metrics(font);                        // 测量字体
    int const width = metrics.horizontalAdvance(text) + 32;  // 依据文字计算宽度
    int const height = NodeItemDelegate::NodeRowHeight;      // 与面板中的节点行同高

    QPixmap pixmap(width, height);                           // 创建预览图
    pixmap.fill(Qt::transparent);                            // 先填透明

    QPainter painter(&pixmap);                               // 开始绘制
    painter.setRenderHint(QPainter::Antialiasing, true);     // 开启抗锯齿
    painter.setPen(QPen(fineGridColor.lighter(140), 1));     // 设置边框画笔
    painter.setBrush(fineGridColor);                         // 设置填充
    painter.drawRoundedRect(QRectF(0.5, 0.5, width - 1, height - 1), 6, 6); // 画圆角矩形
    painter.setPen(QColor("#e6e6e6"));                       // 文字颜色
    painter.setFont(font);                                   // 应用字体
    painter.drawText(QRectF(0, 0, width, height), Qt::AlignCenter, text); // 居中绘制文字
    painter.end();                                           // 结束绘制

    return pixmap;                                           // 返回预览图
}