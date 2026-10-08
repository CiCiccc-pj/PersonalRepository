#include "sleepnodemodel.h"                    // 本类头文件

#include <QtCore/QThread>                      // 线程休眠
#include <QtGui/QIntValidator>                 // 整数校验器
#include <QtWidgets/QHBoxLayout>               // 水平布局
#include <QtWidgets/QLabel>                    // 单位标签
#include <QtWidgets/QLineEdit>                 // 输入框
#include <QtWidgets/QWidget>                   // 内嵌面板

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 构造函数：准备输出执行流
SleepModel::SleepModel()
    : _outData(std::make_shared<FlowData>(FlowPortType::exec())) // 输出执行流
{}

// 返回指定方向的端口数量：一个执行流输入口、一个执行流输出口
unsigned int SleepModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个执行流输入
    case PortType::Out:
        return 1;                              // 一个执行流输出
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型：两侧都是执行流
NodeDataType SleepModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 两侧类型相同
    Q_UNUSED(portIndex);                       // 每侧只有一个端口
    return FlowPortType::exec();               // 执行流
}

// 返回端口说明
QString SleepModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 两侧同名
    Q_UNUSED(portIndex);                       // 每侧只有一个端口
    return QStringLiteral("执行");              // 执行流
}

// 创建并返回内嵌面板：输入框 + “毫秒”单位标签（由画布接管所有权）
QWidget *SleepModel::embeddedWidget()
{
    if (!_panel) {                             // 只在首次创建
        _panel = new QWidget;                  // 面板容器
        _edit = new QLineEdit(QStringLiteral("1000")); // 休眠时长，默认 1000 毫秒
        _edit->setValidator(new QIntValidator(0, 60000, _edit)); // 限制 0~60000 毫秒
        _edit->setAlignment(Qt::AlignCenter);  // 文字居中
        _edit->setFixedWidth(70);              // 固定宽度

        auto *layout = new QHBoxLayout(_panel); // 面板横向布局
        layout->setContentsMargins(0, 0, 0, 0); // 去掉边距
        layout->addWidget(_edit);              // 左侧输入框
        layout->addWidget(new QLabel(QStringLiteral("毫秒"), _panel)); // 右侧单位标签

        _ms = _edit->text().toInt();           // 初始化缓存
        connect(_edit, &QLineEdit::textChanged, this, [this](QString const &text) {
            _ms = text.toInt();                // 缓存输入框数值，供工作线程读取
        });

        _panel->adjustSize();                  // 按内容自适应尺寸
        _panel->setFixedSize(_panel->size());  // 之后固定，避免节点尺寸跳动
    }
    return _panel;                             // 返回给画布嵌入
}

// 执行流到达：按输入框数值阻塞等待，再把执行流转给下游
void SleepModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略，不休眠

    int const ms = _ms;                        // 读取缓存的休眠时长（毫秒）
    if (ms > 0)                                // 有效时长才休眠
        QThread::msleep(static_cast<unsigned long>(ms)); // 在工作线程里阻塞，不卡 UI

    Q_EMIT dataUpdated(0);                     // 休眠结束后把执行流转给下游
}

// 输出执行流
std::shared_ptr<NodeData> SleepModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 只有一个输出口
    return _outData;                           // 返回执行流数据
}
