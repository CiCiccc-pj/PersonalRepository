#include "outputnodemodel.h"                   // 本类头文件

#include <QtGui/QIntValidator>                 // 整数校验器
#include <QtGui/QDoubleValidator>              // 浮点校验器
#include <QtWidgets/QLineEdit>                 // 输入框

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// ===================== 输出整数 =====================

// 构造函数：初始输出整数 0（合并口，带执行标记）
OutputIntModel::OutputIntModel()
    : _outData(std::make_shared<FlowData>(FlowPortType::integer(), 0, true)) // 初始输出数据
{}

// 返回指定方向的端口数量
unsigned int OutputIntModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个执行流输入
    case PortType::Out:
        return 1;                              // 一个输出口（执行流 + 整数）
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType OutputIntModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 未使用该参数
    return (portType == PortType::In) ? FlowPortType::exec()      // 输入：执行流
                                      : FlowPortType::integer();  // 输出：整数（同时携带执行流）
}

// 返回端口说明：输出口标注它是“执行 + 数值”的合并口
QString OutputIntModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 每侧都只有一个端口
    return (portType == PortType::In) ? QStringLiteral("执行")      // 输入口：执行流
                                      : QStringLiteral("执行+整数"); // 输出口：合并口
}

// 创建并返回内嵌的整数输入框（由画布接管所有权）
QWidget *OutputIntModel::embeddedWidget()
{
    if (!_edit) {                              // 只在首次创建
        _edit = new QLineEdit(QStringLiteral("0"));              // 整数输入框
        _edit->setValidator(new QIntValidator(_edit));           // 限制只能输入整数
        _edit->setAlignment(Qt::AlignCenter);                    // 文字居中
        _edit->setFixedWidth(90);                                // 固定宽度
        _edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 固定尺寸
        connect(_edit, &QLineEdit::textChanged, this, [this](QString const &text) {
            _value = text.toInt();             // 缓存输入框数值，供工作线程读取
        });
    }
    return _edit;                              // 返回给画布嵌入
}

// 执行流到达：读取输入框，输出“执行流 + 整数”
void OutputIntModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略

    int const value = _value;                  // 读取缓存的整数
    _outData = std::make_shared<FlowData>(FlowPortType::integer(), value, true); // 更新输出数据（带执行标记）
    Q_EMIT dataUpdated(0);                     // 通知下游取新值
}

// 返回当前输出数据
std::shared_ptr<NodeData> OutputIntModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 只有一个输出口
    return _outData;                           // 返回当前合并数据
}

// ===================== 输出浮点数 =====================

// 构造函数：初始输出浮点数 0（合并口，带执行标记）
OutputFloatModel::OutputFloatModel()
    : _outData(std::make_shared<FlowData>(FlowPortType::real(), 0.0, true)) // 初始输出数据
{}

// 返回指定方向的端口数量
unsigned int OutputFloatModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个执行流输入
    case PortType::Out:
        return 1;                              // 一个输出口（执行流 + 浮点数）
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType OutputFloatModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 未使用该参数
    return (portType == PortType::In) ? FlowPortType::exec()     // 输入：执行流
                                      : FlowPortType::real();    // 输出：浮点（同时携带执行流）
}

// 返回端口说明：输出口标注它是“执行 + 数值”的合并口
QString OutputFloatModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 每侧都只有一个端口
    return (portType == PortType::In) ? QStringLiteral("执行")        // 输入口：执行流
                                      : QStringLiteral("执行+浮点");   // 输出口：合并口
}

// 创建并返回内嵌的浮点数输入框（由画布接管所有权）
QWidget *OutputFloatModel::embeddedWidget()
{
    if (!_edit) {                              // 只在首次创建
        _edit = new QLineEdit(QStringLiteral("0"));              // 浮点数输入框
        _edit->setValidator(new QDoubleValidator(_edit));        // 限制只能输入数字
        _edit->setAlignment(Qt::AlignCenter);                    // 文字居中
        _edit->setFixedWidth(90);                                // 固定宽度
        _edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 固定尺寸
        connect(_edit, &QLineEdit::textChanged, this, [this](QString const &text) {
            _value = text.toDouble();          // 缓存输入框数值，供工作线程读取
        });
    }
    return _edit;                              // 返回给画布嵌入
}

// 执行流到达：读取输入框，输出“执行流 + 浮点数”
void OutputFloatModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略

    double const value = _value;               // 读取缓存的浮点数
    _outData = std::make_shared<FlowData>(FlowPortType::real(), value, true); // 更新输出数据（带执行标记）
    Q_EMIT dataUpdated(0);                     // 通知下游取新值
}

// 返回当前输出数据
std::shared_ptr<NodeData> OutputFloatModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 只有一个输出口
    return _outData;                           // 返回当前合并数据
}