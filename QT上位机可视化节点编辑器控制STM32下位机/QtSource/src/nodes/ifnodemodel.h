#ifndef IFNODEMODEL_H
#define IFNODEMODEL_H

#include <QtNodes/NodeDelegateModel>           // 基类：节点代理模型

#include "flowdata.h"                          // 共享数据类型

class QLineEdit;                               // 前置声明：数值输入框
class QComboBox;                               // 前置声明：运算符下拉框

/// “判断”节点：只用数值口接收“执行流 + 数值”合并数据，按运算符比较后从两条分支之一输出执行流。
/// 端口约定：输入 0 = 数值（数据到达即视为一次执行触发）；输出 0 = 真分支，输出 1 = 假分支。
class IfModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    IfModel();                                 // 构造函数

    static QString Name() { return QStringLiteral("判断"); } // 注册用的唯一类型名

    QString caption() const override { return Name(); } // 节点显示标题

    QString name() const override { return Name(); }    // 节点唯一名称

    unsigned int nPorts(QtNodes::PortType portType) const override; // 端口数量

    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override; // 端口类型

    QString portCaption(QtNodes::PortType portType,
                        QtNodes::PortIndex portIndex) const override; // 端口说明文字

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; } // 显示端口说明

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                   QtNodes::PortIndex portIndex) override; // 接收数值（数据到达即触发判断）

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override; // 输出执行流分支

    QWidget *embeddedWidget() override;        // 内嵌运算符面板

private:
    void evaluate();                           // 执行比较并按结果输出执行流

    QWidget *_panel = nullptr;                 // 内嵌面板（所有权归画布代理控件）
    QLineEdit *_edit = nullptr;                // 参与比较的数值输入框
    QComboBox *_combo = nullptr;               // 比较运算符下拉框

    double _rhs = 0.0;                         // 比较常量缓存（工作线程只读，不碰控件）
    int _opIndex = 0;                          // 运算符索引缓存：0 > ，1 < ，2 >= ，3 <=

    std::shared_ptr<FlowData> _outData[2];     // 两条分支的执行流输出数据
    double _inputValue = 0.0;                  // 数值输入口收到的值
};

#endif // IFNODEMODEL_H