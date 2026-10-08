#include "ifnodemodel.h"                       // 本类头文件

#include <QtGui/QDoubleValidator>              // 浮点校验器
#include <QtWidgets/QComboBox>                 // 运算符下拉框
#include <QtWidgets/QHBoxLayout>               // 水平布局
#include <QtWidgets/QLineEdit>                 // 数值输入框

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 构造函数：准备两条分支的执行流数据
IfModel::IfModel()
{
    _outData[0] = std::make_shared<FlowData>(FlowPortType::exec()); // 真分支执行流
    _outData[1] = std::make_shared<FlowData>(FlowPortType::exec()); // 假分支执行流
}

// 返回指定方向的端口数量
unsigned int IfModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 输入：数值（合并口数据到达即触发）
    case PortType::Out:
        return 2;                              // 输出：真分支 + 假分支
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType IfModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 未使用该参数
    if (portType == PortType::In)              // 输入侧
        return FlowPortType::any();            // 数值口：通用型，可接整数或浮点

    return FlowPortType::exec();               // 输出侧：两条分支都是执行流
}

// 返回端口的说明文字
QString IfModel::portCaption(PortType portType, PortIndex portIndex) const
{
    if (portType == PortType::In)              // 输入侧
        return QStringLiteral("数值");          // 唯一的输入口

    return (portIndex == 0) ? QStringLiteral("真") : QStringLiteral("假");         // 分支说明
}

// 创建并返回内嵌的运算符面板（由画布接管所有权）
QWidget *IfModel::embeddedWidget()
{
    if (!_panel) {                             // 只在首次创建
        _panel = new QWidget();                // 面板容器
        _edit = new QLineEdit(QStringLiteral("0"));              // 比较用的数值输入框
        _edit->setValidator(new QDoubleValidator(_edit));        // 限制只能输入数字
        _edit->setAlignment(Qt::AlignCenter);                    // 文字居中
        _edit->setFixedWidth(70);                                // 固定宽度
        _combo = new QComboBox();                                // 运算符下拉框
        _combo->addItem(QStringLiteral(">"));                    // 大于
        _combo->addItem(QStringLiteral("<"));                    // 小于
        _combo->addItem(QStringLiteral(">="));                   // 大于等于
        _combo->addItem(QStringLiteral("<="));                   // 小于等于
        auto *layout = new QHBoxLayout(_panel);                  // 面板用水平布局
        layout->setContentsMargins(0, 0, 0, 0);                  // 去掉边距
        layout->setSpacing(4);                                   // 控件间距
        layout->addWidget(_edit);                                // 放入数值输入框
        layout->addWidget(_combo);                               // 放入运算符下拉框

        _rhs = _edit->text().toDouble();                         // 初始化常量缓存
        connect(_edit, &QLineEdit::textChanged, this, [this](QString const &text) {
            _rhs = text.toDouble();            // 缓存比较常量，供工作线程读取
        });
        _opIndex = _combo->currentIndex();                       // 初始化运算符缓存
        connect(_combo, &QComboBox::currentIndexChanged, this, [this](int index) {
            _opIndex = index;                  // 缓存运算符索引，供工作线程读取
        });

        _panel->adjustSize();                                    // 按内容算尺寸
        _panel->setFixedSize(_panel->size());                    // 固定为内容尺寸
    }
    return _panel;                             // 返回给画布嵌入
}

// 执行比较：把数值口收到的值与输入框的常量按运算符比较，从对应分支输出执行流
void IfModel::evaluate()
{
    double const rhs = _rhs;                   // 缓存的比较常量
    double const lhs = _inputValue;            // 数值口收到的值
    bool result = false;                       // 比较结果

    switch (_opIndex) {                        // 按缓存的运算符索引比较
    case 0:                                    // 大于
        result = lhs > rhs;
        break;
    case 1:                                    // 小于
        result = lhs < rhs;
        break;
    case 2:                                    // 大于等于
        result = lhs >= rhs;
        break;
    default:                                   // 剩下的小于等于
        result = lhs <= rhs;
        break;
    }

    Q_EMIT dataUpdated(result ? 0 : 1);        // 输出真/假分支的执行流
}

// 接收数值：合并口的数据到达即视为一次执行触发，直接判断
void IfModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口

    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略

    auto flow = std::dynamic_pointer_cast<FlowData>(nodeData); // 转成流程数据
    if (!flow)                                 // 类型不符
        return;                                // 忽略

    _inputValue = flow->value().toDouble();    // 记录收到的数值
    evaluate();                                // 立即判断并输出分支
}

// 返回对应分支的执行流数据
std::shared_ptr<NodeData> IfModel::outData(PortIndex port)
{
    return _outData[port == 0 ? 0 : 1];         // 0 真分支，其余按假分支处理
}