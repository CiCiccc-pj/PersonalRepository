#ifndef OUTPUTNODEMODEL_H
#define OUTPUTNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QLineEdit;                               // 前置声明：输入框

/// “输出整数”：输入执行流，输出“执行流 + 整数”合并口。
/// 执行流到达时，把内嵌输入框里的整数输出给下游，同时带上执行标记。
class OutputIntModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    OutputIntModel();                          // 构造函数

    static QString Name() { return QStringLiteral("输出整数"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收执行流

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流 + 整数

    QWidget *embeddedWidget() override;        // 内嵌整数输入框

private:
    QLineEdit *_edit = nullptr;                // 整数输入框（所有权归画布代理控件）
    int _value = 0;                            // 输入框数值缓存（工作线程只读，不碰控件）
    std::shared_ptr<FlowData> _outData;        // 当前输出数据
};

/// “输出浮点数”：输入执行流，输出“执行流 + 浮点数”合并口。
/// 执行流到达时，把内嵌输入框里的浮点数输出给下游，同时带上执行标记。
class OutputFloatModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    OutputFloatModel();                        // 构造函数

    static QString Name() { return QStringLiteral("输出浮点数"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收执行流

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流 + 浮点数

    QWidget *embeddedWidget() override;        // 内嵌浮点数输入框

private:
    QLineEdit *_edit = nullptr;                // 浮点数输入框（所有权归画布代理控件）
    double _value = 0.0;                       // 输入框数值缓存（工作线程只读，不碰控件）
    std::shared_ptr<FlowData> _outData;        // 当前输出数据
};

#endif // OUTPUTNODEMODEL_H