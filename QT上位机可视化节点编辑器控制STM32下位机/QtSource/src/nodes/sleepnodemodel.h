#ifndef SLEEPNODEMODEL_H
#define SLEEPNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QLineEdit;                               // 前置声明：输入框

/// “休眠节点”：一个执行流输入口、一个执行流输出口。
/// 执行流到达时，按内嵌输入框里的数值（毫秒）阻塞等待，然后把执行流转给下游。
class SleepModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    SleepModel();                              // 构造函数

    static QString Name() { return QStringLiteral("休眠节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 执行流到达时休眠

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流

    QWidget *embeddedWidget() override;        // 内嵌“毫秒”输入框

private:
    QWidget *_panel = nullptr;                 // 内嵌面板（输入框 + 单位标签）
    QLineEdit *_edit = nullptr;                // 休眠时长输入框（毫秒）
    int _ms = 1000;                            // 休眠时长缓存（工作线程只读，不碰控件）
    std::shared_ptr<FlowData> _outData;        // 输出数据（执行流）
};

#endif // SLEEPNODEMODEL_H
