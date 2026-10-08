#include "displaynodemodel.h"                  // 本类头文件

#include <QtWidgets/QLineEdit>                 // 只读文本框

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// ===================== 显示整数 =====================

// 构造函数
DisplayIntModel::DisplayIntModel() = default;

// 返回指定方向的端口数量
unsigned int DisplayIntModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个输入口（执行流 + 整数）
    case PortType::Out:
        return 0;                              // 没有输出
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType DisplayIntModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 只有一个输入口
    Q_UNUSED(portIndex);                       // 未使用该参数
    return FlowPortType::integer();            // 输入整数（同时携带执行流）
}

// 返回端口说明：输入口是“执行 + 数值”的合并口
QString DisplayIntModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 只有一个输入口
    Q_UNUSED(portIndex);                       // 未使用该参数
    return QStringLiteral("执行+整数");          // 输入口：合并口
}

// 连接策略：输入口允许多条连线，只显示最后到达的数值
ConnectionPolicy DisplayIntModel::portConnectionPolicy(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    return portType == PortType::In ? ConnectionPolicy::Many  // 输入口可接多条
                                    : ConnectionPolicy::One;  // 其他方向保持单条
}

// 创建并返回内嵌的只读文本框（由画布接管所有权）
QWidget *DisplayIntModel::embeddedWidget()
{
    if (!_view) {                              // 只在首次创建
        _view = new QLineEdit(QStringLiteral("-"));              // 文本框
        _view->setReadOnly(true);                                // 设为只读
        _view->setAlignment(Qt::AlignCenter);                    // 文字居中
        _view->setFixedWidth(90);                                // 固定宽度
        _view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 固定尺寸
        // 跨线程更新文本框：信号从工作线程发出，槽自动排队回 UI 线程执行
        connect(this, &DisplayIntModel::valueTextChanged, _view, &QLineEdit::setText);
    }
    return _view;                              // 返回给画布嵌入
}

// 收到整数：把文本通过信号交给 UI 线程显示
void DisplayIntModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口

    auto flow = std::dynamic_pointer_cast<FlowData>(nodeData);   // 转成流程数据
    if (!flow) {                               // 无有效数据（例如连线被断开）
        Q_EMIT valueTextChanged(QStringLiteral("-"));            // 显示占位符
        return;                                // 结束
    }

    Q_EMIT valueTextChanged(QString::number(flow->value().toInt())); // 显示整数值
}

// 无输出端口
std::shared_ptr<NodeData> DisplayIntModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 未使用该参数
    return nullptr;                            // 没有输出数据
}

// ===================== 显示浮点数 =====================

// 构造函数
DisplayFloatModel::DisplayFloatModel() = default;

// 返回指定方向的端口数量
unsigned int DisplayFloatModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个输入口（执行流 + 浮点数）
    case PortType::Out:
        return 0;                              // 没有输出
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType DisplayFloatModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 只有一个输入口
    Q_UNUSED(portIndex);                       // 未使用该参数
    return FlowPortType::real();               // 输入浮点（同时携带执行流）
}

// 返回端口说明：输入口是“执行 + 数值”的合并口
QString DisplayFloatModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 只有一个输入口
    Q_UNUSED(portIndex);                       // 未使用该参数
    return QStringLiteral("执行+浮点");          // 输入口：合并口
}

// 连接策略：输入口允许多条连线，只显示最后到达的数值
ConnectionPolicy DisplayFloatModel::portConnectionPolicy(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    return portType == PortType::In ? ConnectionPolicy::Many  // 输入口可接多条
                                    : ConnectionPolicy::One;  // 其他方向保持单条
}

// 创建并返回内嵌的只读文本框（由画布接管所有权）
QWidget *DisplayFloatModel::embeddedWidget()
{
    if (!_view) {                              // 只在首次创建
        _view = new QLineEdit(QStringLiteral("-"));              // 文本框
        _view->setReadOnly(true);                                // 设为只读
        _view->setAlignment(Qt::AlignCenter);                    // 文字居中
        _view->setFixedWidth(90);                                // 固定宽度
        _view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 固定尺寸
        // 跨线程更新文本框：信号从工作线程发出，槽自动排队回 UI 线程执行
        connect(this, &DisplayFloatModel::valueTextChanged, _view, &QLineEdit::setText);
    }
    return _view;                              // 返回给画布嵌入
}

// 收到浮点数：把文本通过信号交给 UI 线程显示
void DisplayFloatModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口

    auto flow = std::dynamic_pointer_cast<FlowData>(nodeData);   // 转成流程数据
    if (!flow) {                               // 无有效数据（例如连线被断开）
        Q_EMIT valueTextChanged(QStringLiteral("-"));            // 显示占位符
        return;                                // 结束
    }

    Q_EMIT valueTextChanged(QString::number(flow->value().toDouble())); // 显示浮点值
}

// 无输出端口
std::shared_ptr<NodeData> DisplayFloatModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 未使用该参数
    return nullptr;                            // 没有输出数据
}