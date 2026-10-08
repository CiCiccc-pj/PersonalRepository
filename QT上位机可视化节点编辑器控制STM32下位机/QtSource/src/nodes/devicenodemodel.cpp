#include "devicenodemodel.h"                   // 本类头文件

#include "serialportmanager.h"                 // 串口管理器

#include <QtWidgets/QComboBox>                 // 下拉框

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// ===================== 开关型节点基类 =====================

// 构造函数：记录外设位与开关文字，并准备执行流输出
DeviceSwitchModel::DeviceSwitchModel(int deviceBit, QString onText, QString offText)
    : _bit(deviceBit)                          // 命令位
    , _onText(std::move(onText))               // “开”文字
    , _offText(std::move(offText))             // “关”文字
    , _outData(std::make_shared<FlowData>(FlowPortType::exec())) // 输出执行流
{}

// 返回指定方向的端口数量：一个执行流输入口、一个执行流输出口
unsigned int DeviceSwitchModel::nPorts(PortType portType) const
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
NodeDataType DeviceSwitchModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 两侧类型相同
    Q_UNUSED(portIndex);                       // 每侧只有一个端口
    return FlowPortType::exec();               // 执行流
}

// 返回端口说明
QString DeviceSwitchModel::portCaption(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 两侧同名
    Q_UNUSED(portIndex);                       // 每侧只有一个端口
    return QStringLiteral("执行");              // 执行流
}

// 创建并返回内嵌的“开/关”下拉框（由画布接管所有权）
QWidget *DeviceSwitchModel::embeddedWidget()
{
    if (!_combo) {                             // 只在首次创建
        _combo = new QComboBox;                // 下拉框
        _combo->addItem(_onText);              // 索引 0：开
        _combo->addItem(_offText);             // 索引 1：关
        _combo->setCurrentIndex(0);            // 默认“开”
        _combo->setFixedWidth(70);             // 固定宽度
        _combo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // 固定尺寸

        _on = (_combo->currentIndex() == 0);   // 初始化开关缓存
        connect(_combo, &QComboBox::currentIndexChanged, this, [this](int index) {
            _on = (index == 0);                // 缓存开关选择，供工作线程读取
        });
    }
    return _combo;                             // 返回给画布嵌入
}

// 执行流到达：按下拉框选择下发全量状态命令，再把执行流转给下游
void DeviceSwitchModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略，不下发命令

    SerialPortManager::instance().setOutputBit(_bit, _on);   // 读取缓存的开关选择并全量下发
    Q_EMIT dataUpdated(0);                     // 把执行流转给下游节点
}

// 输出执行流
std::shared_ptr<NodeData> DeviceSwitchModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 只有一个输出口
    return _outData;                           // 返回执行流数据
}

// ===================== 红灯节点 =====================

// 构造函数：bit0，亮 / 灭
RedLedModel::RedLedModel()
    : DeviceSwitchModel(0, QStringLiteral("亮"), QStringLiteral("灭")) // 红灯对应 bit0
{}

// ===================== 蓝灯节点 =====================

// 构造函数：bit1，亮 / 灭
BlueLedModel::BlueLedModel()
    : DeviceSwitchModel(1, QStringLiteral("亮"), QStringLiteral("灭")) // 蓝灯对应 bit1
{}

// ===================== 蜂鸣器节点 =====================

// 构造函数：bit2，响 / 不响
BuzzerModel::BuzzerModel()
    : DeviceSwitchModel(2, QStringLiteral("响"), QStringLiteral("不响")) // 蜂鸣器对应 bit2
{}

// ===================== 温湿度节点 =====================

// 构造函数：准备三路默认输出（执行流、温度、湿度）
TempHumiModel::TempHumiModel()
    : _outData{std::make_shared<FlowData>(FlowPortType::exec()),         // 0 执行流
               std::make_shared<FlowData>(FlowPortType::real(), 0.0),    // 1 温度默认 0
               std::make_shared<FlowData>(FlowPortType::real(), 0.0)}    // 2 湿度默认 0
{}

// 返回指定方向的端口数量
unsigned int TempHumiModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 1;                              // 一个执行流输入
    case PortType::Out:
        return 3;                              // 三个输出：执行流、温度、湿度
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型
NodeDataType TempHumiModel::dataType(PortType portType, PortIndex portIndex) const
{
    if (portType == PortType::In)              // 输入口
        return FlowPortType::exec();           // 执行流
    return portIndex == 0 ? FlowPortType::exec()   // 输出 0：执行流
                          : FlowPortType::real();  // 输出 1/2：浮点
}

// 返回端口说明
QString TempHumiModel::portCaption(PortType portType, PortIndex portIndex) const
{
    if (portType == PortType::In)              // 输入口
        return QStringLiteral("执行");          // 执行流
    if (portIndex == 0)                        // 输出 0
        return QStringLiteral("执行");          // 执行流
    return portIndex == 1 ? QStringLiteral("温度") : QStringLiteral("湿度"); // 输出 1/2
}

// 执行流到达：请求一次温湿度并阻塞等待回复，最后把执行流转给下游
void TempHumiModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    Q_UNUSED(portIndex);                       // 只有一个输入口
    if (!nodeData)                             // 空数据（例如连线被断开）
        return;                                // 忽略

    double temperature = 0.0;                  // 温度结果
    double humidity = 0.0;                     // 湿度结果
    if (SerialPortManager::instance().requestTemperatureHumidity(temperature, humidity)) { // 读取成功
        _outData[1] = std::make_shared<FlowData>(FlowPortType::real(), temperature); // 更新温度
        _outData[2] = std::make_shared<FlowData>(FlowPortType::real(), humidity);    // 更新湿度
        Q_EMIT dataUpdated(1);                 // 通知下游取温度
        Q_EMIT dataUpdated(2);                 // 通知下游取湿度
    }

    Q_EMIT dataUpdated(0);                     // 无论读取成功与否，都继续往下执行
}

// 返回指定输出口的数据
std::shared_ptr<NodeData> TempHumiModel::outData(PortIndex port)
{
    if (port >= 0 && port < 3)                 // 端口序号合法
        return _outData[port];                 // 0 执行流，1 温度，2 湿度
    return nullptr;                            // 越界返回空
}
