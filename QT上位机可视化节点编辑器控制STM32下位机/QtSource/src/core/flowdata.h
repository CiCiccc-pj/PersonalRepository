#ifndef FLOWDATA_H
#define FLOWDATA_H

#include <QtCore/QVariant>                     // 承载任意类型的值
#include <QtNodes/NodeData>                    // 基类：节点数据

#include "flowporttypes.h"                     // 端口类型定义

#include <utility>                             // std::move

/// 流程图中在节点端口之间传递的数据。
/// 同时携带“类型”和“值”，类型用于连线校验，值用于下游节点计算。
/// 合并口（如输出节点的输出口）还会带上执行流标记，表示这份数据同时是一次执行触发。
class FlowData : public QtNodes::NodeData
{
public:
    FlowData() = default;                      // 默认构造：通用类型、空值、无执行流

    FlowData(QtNodes::NodeDataType type, QVariant value = {}, bool exec = false) // 指定类型、值与执行标记
        : _type(std::move(type))               // 保存类型
        , _value(std::move(value))             // 保存值
        , _exec(exec)                          // 保存执行流标记
    {}

    QtNodes::NodeDataType type() const override { return _type; } // 覆写：数据类型

    QVariant const &value() const { return _value; } // 读取数据值

    void setValue(QVariant value) { _value = std::move(value); } // 修改数据值

    bool hasExec() const { return _exec; }     // 是否同时携带执行流

    void setHasExec(bool exec) { _exec = exec; } // 修改执行流标记

private:
    QtNodes::NodeDataType _type = FlowPortType::any(); // 默认通用类型
    QVariant _value;                           // 实际存储的值
    bool _exec = false;                        // 合并口标记：是否携带执行流
};

#endif // FLOWDATA_H