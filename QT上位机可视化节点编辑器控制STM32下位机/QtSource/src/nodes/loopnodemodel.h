#ifndef LOOPNODEMODEL_H
#define LOOPNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QLineEdit;                               // 前置声明：次数输入框

/// “循环节点”：按内嵌输入框里的次数重复执行循环体。
/// 端口约定：
///   输入 0「进入」= 启动循环；输入 1「循环」= 循环体执行完回到本节点（回边）；
///   输出 0「循环体」= 每轮触发一次；输出 1「完成」= 循环结束后继续。
/// 使用时需把循环体末端接回输入 1，形成回边，循环才能逐轮推进。
class LoopModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    LoopModel();                               // 构造函数

    static QString Name() { return QStringLiteral("循环节点"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 执行流到达时推进循环

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流

    QWidget *embeddedWidget() override;        // 内嵌“次数”输入框

private:
    QWidget *_panel = nullptr;                 // 内嵌面板（输入框 + 单位标签）
    QLineEdit *_edit = nullptr;                // 循环次数输入框
    int _count = 3;                            // 循环次数缓存（工作线程只读，不碰控件）
    int _iter = 0;                             // 已触发的循环轮数（只在工作线程读写）
    std::shared_ptr<FlowData> _outData[2];     // 输出数据：0 循环体，1 完成
};

#endif // LOOPNODEMODEL_H