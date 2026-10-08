#ifndef DISPLAYNODEMODEL_H
#define DISPLAYNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QLineEdit;                               // 前置声明：文本框

/// “显示整数”：输入“执行流 + 整数”合并口，没有输出端口。
/// 输入口允许多条连线，只显示最后到达的数值。
class DisplayIntModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    DisplayIntModel();                         // 构造函数

    static QString Name() { return QStringLiteral("显示整数"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    QtNodes::ConnectionPolicy portConnectionPolicy(QtNodes::PortType portType,
                                                   QtNodes::PortIndex portIndex) const override; // 输入口允许多条连线

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收整数并显示

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 无输出

    QWidget *embeddedWidget() override;        // 内嵌只读文本框

Q_SIGNALS:
    void valueTextChanged(QString const &text); // 显示文本变化（跨线程通知 UI 更新）

private:
    QLineEdit *_view = nullptr;                // 只读文本框（所有权归画布代理控件）
};

/// “显示浮点数”：输入“执行流 + 浮点数”合并口，没有输出端口。
/// 输入口允许多条连线，只显示最后到达的数值。
class DisplayFloatModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    DisplayFloatModel();                       // 构造函数

    static QString Name() { return QStringLiteral("显示浮点数"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    QtNodes::ConnectionPolicy portConnectionPolicy(QtNodes::PortType portType,
                                                   QtNodes::PortIndex portIndex) const override; // 输入口允许多条连线

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收浮点数并显示

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 无输出

    QWidget *embeddedWidget() override;        // 内嵌只读文本框

Q_SIGNALS:
    void valueTextChanged(QString const &text); // 显示文本变化（跨线程通知 UI 更新）

private:
    QLineEdit *_view = nullptr;                // 只读文本框（所有权归画布代理控件）
};

#endif // DISPLAYNODEMODEL_H