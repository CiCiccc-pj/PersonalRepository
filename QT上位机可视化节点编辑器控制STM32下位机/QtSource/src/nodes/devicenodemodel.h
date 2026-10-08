#ifndef DEVICENODEMODEL_H
#define DEVICENODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QComboBox;                               // 前置声明：下拉框

/// 下位机“开关型”节点基类：一个执行流输入口、一个执行流输出口，以及一个“开/关”下拉框。
/// 执行流到达时，按《上位机通信协议.md》下发命令字节（全量状态），随后把执行流转给下游。
/// 子类只需给出外设对应的位与开关文字。
class DeviceSwitchModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    DeviceSwitchModel(int deviceBit, QString onText, QString offText); // 构造函数

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 执行流到达时下发命令

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流

    QWidget *embeddedWidget() override;        // 内嵌“开/关”下拉框

private:
    int _bit = 0;                              // 外设对应的命令位（bit0 红灯 / bit1 蓝灯 / bit2 蜂鸣器）
    QString _onText;                           // “开”状态的显示文字
    QString _offText;                          // “关”状态的显示文字
    QComboBox *_combo = nullptr;               // 下拉框（所有权归画布代理控件）
    bool _on = true;                           // 开关选择缓存（工作线程只读，不碰控件）
    std::shared_ptr<FlowData> _outData;        // 输出数据（执行流）
};

/// “红灯节点”：控制 PA7 红灯，对应命令位 bit0。
class RedLedModel : public DeviceSwitchModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    RedLedModel();                             // 构造函数

    static QString Name() { return QStringLiteral("红灯节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称
};

/// “蓝灯节点”：控制 PA6 蓝灯，对应命令位 bit1。
class BlueLedModel : public DeviceSwitchModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    BlueLedModel();                            // 构造函数

    static QString Name() { return QStringLiteral("蓝灯节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称
};

/// “蜂鸣器节点”：控制 PA5 蜂鸣器，对应命令位 bit2。
class BuzzerModel : public DeviceSwitchModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    BuzzerModel();                             // 构造函数

    static QString Name() { return QStringLiteral("蜂鸣器节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称
};

/// “温湿度节点”：一个执行流输入口、一个执行流输出口、两个浮点输出口（温度、湿度）。
/// 执行流到达时，按协议带 bit3 请求回发，并阻塞等待下位机回复。
/// 端口顺序：输出 0 = 执行流，1 = 温度，2 = 湿度。
class TempHumiModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    TempHumiModel();                           // 构造函数

    static QString Name() { return QStringLiteral("温湿度节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 执行流到达时读一次温湿度

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流 / 温度 / 湿度

    QWidget *embeddedWidget() override { return nullptr; } // 无内嵌控件

private:
    std::shared_ptr<FlowData> _outData[3];     // 输出数据：0 执行流，1 温度，2 湿度
};

#endif // DEVICENODEMODEL_H
