#include "loopnodemodel.h"                     // 本类头文件

#include <QtGui/QIntValidator>                 // 整数校验器
#include <QtWidgets/QHBoxLayout>               // 水平布局
#include <QtWidgets/QLabel>                    // 单位标签
#include <QtWidgets/QLineEdit>                 // 输入框
#include <QtWidgets/QWidget>                   // 内嵌面板

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 构造函数：准备两路执行流输出
LoopModel::LoopModel()
    : _outData{std::make_shared<FlowData>(FlowPortType::exec()),   // 0 循环体
               std::make_shared<FlowData>(FlowPortType::exec())}   // 1 完成
{}

// 返回指定方向的端口数量：两个执行流输入、两个执行流输出
unsigned int LoopModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 2;                              // 进入 + 循环（回边）
    case PortType::Out:
        return 2;                              // 循环体 + 完成
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型：四个端口都是执行流
NodeDataType LoopModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 所有端口同为执行流
    Q_UNUSED(portIndex);                       // 未使用该参数
    return FlowPortType::exec();               // 执行流
}

// 返回端口说明
QString LoopModel::portCaption(PortType portType, PortIndex portIndex) const
{
    if (portType == PortType::In)              // 输入侧
        return portIndex == 0 ? QStringLiteral("进入") : QStringLiteral("循环"); // 启动 / 回边
    return portIndex == 0 ? QStringLiteral("循环体") : QStringLiteral("完成");   // 每轮 / 结束
}

// 创建并返回内嵌面板：次数输入框 + “次”单位标签（由画布接管所有权）
QWidget *LoopModel::embeddedWidget()
{
    if (!_panel) {                             // 只在首次创建
        _panel = new QWidget;                  // 面板容器
        _edit = new QLineEdit(QStringLiteral("3")); // 循环次数，默认 3 次
        _edit->setValidator(new QIntValidator(0, 100000, _edit)); // 限制 0~100000 次
        _edit->setAlignment(Qt::AlignCenter);  // 文字居中
        _edit->setFixedWidth(70);              // 固定宽度

        auto *layout = new QHBoxLayout(_panel); // 面板横向布局
        layout->setContentsMargins(0, 0, 0, 0); // 去掉边距
        layout->addWidget(_edit);              // 左侧输入框
        layout->addWidget(new QLabel(QStringLiteral("次"), _panel)); // 右侧单位标签

        _count = _edit->text().toInt();        // 初始化缓存
        connect(_edit, &QLineEdit::textChanged, this, [this](QString const &text) {
            _count = text.toInt();             // 缓存次数，供工作线程读取
        });

        _panel->adjustSize();                  // 按内容自适应尺寸
        _panel->setFixedSize(_panel->size());  // 之后固定，避免节点尺寸跳动
    }
    return _panel;                             // 返回给画布嵌入
}

// 执行流到达：输入 0 启动循环，输入 1 表示循环体跑完，推进到下一轮或结束
void LoopModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略，不推进循环

    if (portIndex == 0) {                      // 「进入」：启动循环
        _iter = 0;                             // 复位轮数
        if (_count <= 0) {                     // 次数为 0，直接完成
            Q_EMIT dataUpdated(1);             // 输出「完成」
            return;                            // 结束
        }
        _iter = 1;                             // 进入第一轮
        Q_EMIT dataUpdated(0);                 // 触发「循环体」
        return;                                // 结束
    }

    // 「循环」：循环体执行完回到本节点
    if (_iter < _count) {                      // 还没到指定次数
        ++_iter;                               // 轮数加一
        Q_EMIT dataUpdated(0);                 // 再触发一轮「循环体」
    } else {                                   // 已达到次数
        Q_EMIT dataUpdated(1);                 // 输出「完成」
    }
}

// 返回对应输出口的执行流数据
std::shared_ptr<NodeData> LoopModel::outData(PortIndex port)
{
    return _outData[port == 0 ? 0 : 1];        // 0 循环体，其余按完成处理
}