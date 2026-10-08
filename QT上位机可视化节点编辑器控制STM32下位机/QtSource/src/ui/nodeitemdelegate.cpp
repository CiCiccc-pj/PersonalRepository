#include "nodeitemdelegate.h"                  // 本类头文件

#include <QtGui/QFont>                          // 字体
#include <QtGui/QPainter>                      // 绘制
#include <QtGui/QPainterPath>                  // 圆角矩形路径
#include <QtGui/QPen>                          // 画笔
#include <QtNodes/StyleCollection>             // 画布主题样式
#include <QtWidgets/QStyle>                    // QStyle::State_* 状态标志

#include <algorithm>                           // std::max

namespace {

int const kBoxMargin = 6;                      // 分组框与面板左右边缘的距离
int const kBoxRadius = 8;                      // 分组框圆角半径
int const kItemRadius = 6;                     // 节点项圆角半径
int const kTitlePadding = 12;                  // 分类标题左边距
int const kItemInsetX = 6;                     // 节点项与分组框的左右间距
int const kItemInsetY = 4;                     // 节点项与分组框的上下间距

QColor const kTextColor("#e6e6e6");            // 节点文字颜色
QColor const kTitleColor("#cfcfcf");           // 分类标题颜色

// 分组框底色：比面板底色略深，形成下沉的“框”
QColor boxFillColor()
{
    return QtNodes::StyleCollection::flowViewStyle().BackgroundColor.darker(110);
}

// 分组框边框色
QColor boxBorderColor()
{
    return QtNodes::StyleCollection::flowViewStyle().FineGridColor.lighter(135);
}

// 节点项常态底色
QColor itemFillColor()
{
    return QtNodes::StyleCollection::flowViewStyle().FineGridColor;
}

// 节点项边框色
QColor itemBorderColor()
{
    return QtNodes::StyleCollection::flowViewStyle().FineGridColor.lighter(140);
}

// 节点项悬停底色
QColor itemHoverColor()
{
    return QtNodes::StyleCollection::flowViewStyle().FineGridColor.lighter(125);
}

// 节点项选中底色
QColor itemSelectedColor()
{
    return QtNodes::StyleCollection::flowViewStyle().FineGridColor.lighter(155);
}

} // namespace

// 构造函数
NodeItemDelegate::NodeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)              // 构造基类
{}

// 绘制整行：分类行与节点行分别处理
void NodeItemDelegate::paint(QPainter *painter,
                             QStyleOptionViewItem const &option,
                             QModelIndex const &index) const
{
    QRect const row = option.rect;                                  // 当前行矩形
    QRect const box(row.left() + kBoxMargin,                        // 分组框水平范围：
                    row.top(),                                      // 与行同高
                    row.width() - 2 * kBoxMargin,                   // 左右各留边距
                    row.height());                                  // 高度不变

    painter->save();                                                // 保存画笔状态
    painter->setRenderHint(QPainter::Antialiasing, true);           // 开启抗锯齿

    if (index.parent().isValid())                                   // 有父项：是节点项
        paintNode(painter, option, index, row, box);                // 画节点行
    else                                                            // 无父项：是分类
        paintCategory(painter, option, index, row, box);            // 画分类行

    painter->restore();                                             // 恢复画笔状态
}

// 行高：分类行矮一些，节点行高一些
QSize NodeItemDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option);                                               // 未使用该参数
    bool const isCategory = !index.parent().isValid();               // 是否分类行
    return QSize(0, isCategory ? CategoryRowHeight                   // 分类行高
                               : NodeRowHeight);                     // 节点行高
}

// 绘制分类行：分组框的顶部（上方圆角 + 左右边框）与分类标题
void NodeItemDelegate::paintCategory(QPainter *painter,
                                     QStyleOptionViewItem const &option,
                                     QModelIndex const &index,
                                     QRect const &row,
                                     QRect const &box) const
{
    // 造一个上下都带圆角的矩形，高度取两倍行高，再用裁剪只保留本行
    QRectF const rounded(box.left() + 0.5,                           // 左边界（考虑 1px 画笔居中）
                         row.top() + 0.5,                            // 上边界
                         box.width() - 1.0,                          // 宽度去掉画笔占位
                         row.height() * 2.0);                        // 高度溢出到下一行后被裁掉
    QPainterPath path;                                               // 圆角矩形路径
    path.addRoundedRect(rounded, kBoxRadius, kBoxRadius);            // 生成圆角矩形

    painter->save();                                                 // 保存状态
    painter->setClipRect(QRectF(row), Qt::IntersectClip);            // 裁剪到当前行
    painter->fillPath(path, boxFillColor());                         // 填充框体
    painter->setPen(QPen(boxBorderColor(), 1));                      // 边框画笔
    painter->setBrush(Qt::NoBrush);                                  // 不填充
    painter->drawPath(path);                                         // 画上方圆角与左右边框
    painter->restore();                                              // 恢复状态

    QFont titleFont = option.font;                                   // 复制面板字体
    titleFont.setPointSizeF(std::max(8.0, titleFont.pointSizeF() - 1.0)); // 标题略小
    titleFont.setBold(true);                                         // 加粗显示
    painter->setFont(titleFont);                                     // 应用字体
    painter->setPen(kTitleColor);                                    // 标题颜色
    painter->drawText(box.adjusted(kTitlePadding, 0, -kTitlePadding, 0), // 标题区域
                      Qt::AlignVCenter | Qt::AlignLeft,              // 垂直居中、左对齐
                      index.data(Qt::DisplayRole).toString());       // 画分类名
}

// 绘制节点行：分组框的左右边框（最后一行补底边圆角）+ 框内的节点项
void NodeItemDelegate::paintNode(QPainter *painter,
                                 QStyleOptionViewItem const &option,
                                 QModelIndex const &index,
                                 QRect const &row,
                                 QRect const &box) const
{
    bool const isLast = (index.row() == index.model()->rowCount(index.parent()) - 1); // 是否分类的最后一项

    if (isLast) {                                                    // 最后一项：补上圆角底边
        double const bottom = row.bottom() - 0.5;                    // 底边中线
        QRectF const rounded(box.left() + 0.5,                       // 左边界
                             bottom - row.height() * 2.0,            // 上边界（溢出到上一行后被裁掉）
                             box.width() - 1.0,                      // 宽度
                             row.height() * 2.0);                    // 高度
        QPainterPath path;                                           // 圆角矩形路径
        path.addRoundedRect(rounded, kBoxRadius, kBoxRadius);        // 生成圆角矩形

        painter->save();                                             // 保存状态
        painter->setClipRect(QRectF(row), Qt::IntersectClip);        // 裁剪到当前行
        painter->fillPath(path, boxFillColor());                     // 填充框体
        painter->setPen(QPen(boxBorderColor(), 1));                  // 边框画笔
        painter->setBrush(Qt::NoBrush);                              // 不填充
        painter->drawPath(path);                                     // 画下方圆角与左右边框
        painter->restore();                                          // 恢复状态
    } else {                                                         // 中间项：只画左右两条边框
        painter->fillRect(box, boxFillColor());                      // 填充框体
        painter->setPen(QPen(boxBorderColor(), 1));                  // 边框画笔
        painter->drawLine(QPointF(box.left() + 0.5, row.top()),      // 左边框（连到下一行）
                          QPointF(box.left() + 0.5, row.bottom() + 1.0));
        painter->drawLine(QPointF(box.right() + 0.5, row.top()),     // 右边框（连到下一行）
                          QPointF(box.right() + 0.5, row.bottom() + 1.0));
    }

    QRect const itemRect = box.adjusted(kItemInsetX, kItemInsetY,    // 节点项矩形：
                                        -kItemInsetX, -kItemInsetY); // 在框内四周留白
    QColor itemColor = itemFillColor();                              // 默认底色
    if (option.state & QStyle::State_Selected)                       // 处于选中状态
        itemColor = itemSelectedColor();                             // 换成选中底色
    else if (option.state & QStyle::State_MouseOver)                 // 处于悬停状态
        itemColor = itemHoverColor();                                // 换成悬停底色

    painter->setPen(QPen(itemBorderColor(), 1));                     // 节点项边框
    painter->setBrush(itemColor);                                    // 节点项底色
    painter->drawRoundedRect(QRectF(itemRect).adjusted(0.5, 0.5, -0.5, -0.5), // 居中的圆角矩形
                             kItemRadius, kItemRadius);              // 圆角半径

    painter->setFont(option.font);                                   // 使用面板字体
    painter->setPen(kTextColor);                                     // 文字颜色
    painter->drawText(itemRect,                                      // 文字区域
                      Qt::AlignCenter,                               // 水平垂直居中
                      index.data(Qt::DisplayRole).toString());       // 画节点名
}