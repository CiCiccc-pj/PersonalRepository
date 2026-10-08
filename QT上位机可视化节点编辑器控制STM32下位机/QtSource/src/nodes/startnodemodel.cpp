#include "startnodemodel.h"                    // 本类头文件

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 构造函数：准备默认输出数据（执行流）
StartNodeModel::StartNodeModel()
    : _outData(std::make_shared<FlowData>(FlowPortType::exec())) // 初始化执行流输出
{}

// 返回指定方向的端口数量
unsigned int StartNodeModel::nPorts(PortType portType) const
{
    switch (portType) {                        // 按端口方向判断
    case PortType::In:
        return 0;                              // 无输入端口
    case PortType::Out:
        return 1;                              // 一个输出端口
    case PortType::None:
        break;                                 // 其他情况
    }
    return 0;                                  // 默认无端口
}

// 返回端口的数据类型（开始节点输出执行流）
NodeDataType StartNodeModel::dataType(PortType portType, PortIndex portIndex) const
{
    Q_UNUSED(portType);                        // 未使用该参数
    Q_UNUSED(portIndex);                       // 未使用该参数
    return FlowPortType::exec();               // 输出执行流类型
}

// 接收输入数据（开始节点无输入，忽略）
void StartNodeModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
{
    // 开始节点没有输入端口。
    Q_UNUSED(nodeData);                        // 未使用该参数
    Q_UNUSED(portIndex);                       // 未使用该参数
}

// 返回输出数据
std::shared_ptr<NodeData> StartNodeModel::outData(PortIndex port)
{
    Q_UNUSED(port);                            // 未使用该参数
    return _outData;                           // 返回默认输出数据
}