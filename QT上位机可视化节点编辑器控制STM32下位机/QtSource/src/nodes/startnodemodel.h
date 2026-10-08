#ifndef STARTNODEMODEL_H
#define STARTNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

/// “开始节点”：流程入口，没有输入端口，只有一个输出端口。
class StartNodeModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    StartNodeModel();                          // 构造函数

    /// 注册到 NodeDelegateModelRegistry 时使用的唯一类型名。
    static QString Name() { return QStringLiteral("开始节点"); }

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口数据类型

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收输入数据

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 提供输出数据

    QWidget *embeddedWidget() override { return nullptr; } // 无内嵌控件

private:
    std::shared_ptr<FlowData> _outData;        // 默认输出数据
};

#endif // STARTNODEMODEL_H