#include "flowgraphmodel.h"                    // 本类头文件

#include "flowporttypes.h"                     // 端口类型定义
#include "serialportmanager.h"                 // 串口管理器（要移入工作线程）

#include <QtCore/QThread>                      // 工作线程
#include <QtNodes/ConnectionIdUtils>           // getNodeId / getPortIndex
#include <QtNodes/NodeData>                    // NodeDataType
#include <QtNodes/NodeDelegateModel>           // nPorts 等节点模型接口
#include <QtNodes/NodeDelegateModelRegistry>   // 节点注册表

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 构造函数：把注册表交给基类，并启动执行流程工作线程
FlowGraphModel::FlowGraphModel(std::shared_ptr<NodeDelegateModelRegistry> registry)
    : DataFlowGraphModel(std::move(registry))  // 构造基类
{
    _workerThread = new QThread(this);         // 创建工作线程（随本对象释放）
    _worker = new QObject();                   // 调度对象（无父对象，手动释放）
    _worker->moveToThread(_workerThread);      // 把调度对象放进工作线程
    _workerThread->start();                    // 启动工作线程事件循环

    // 串口只在工作线程里使用，避免与 UI 线程争抢 QSerialPort
    SerialPortManager::instance().moveToThread(_workerThread); // 迁移串口管理器
}

// 析构：把串口管理器交还主线程后，停止并回收工作线程
FlowGraphModel::~FlowGraphModel()
{
    if (!_workerThread || !_worker)            // 线程未建立
        return;                                // 无需处理

    QThread *const mainThread = QThread::currentThread(); // 记录主线程（析构所在线程）
    QMetaObject::invokeMethod(
        _worker,
        [mainThread]() {                       // 在工作线程里执行
            SerialPortManager::instance().moveToThread(mainThread); // 交还主线程，避免跨线程析构
        },
        Qt::BlockingQueuedConnection);         // 阻塞等待迁移完成

    _workerThread->quit();                     // 请求工作线程退出事件循环
    _workerThread->wait();                     // 等待线程真正结束
    delete _worker;                            // 释放调度对象
    _worker = nullptr;                         // 置空，避免悬空
}

// 连接校验：先判端口空位，再套用兼容矩阵
bool FlowGraphModel::connectionPossible(ConnectionId const connectionId) const
{
    // 读取某一侧端口的数据类型 id
    auto dataTypeId = [&](PortType portType) {
        return portData(getNodeId(portType, connectionId),          // 该侧节点
                        portType,                                   // 该侧方向
                        getPortIndex(portType, connectionId),        // 该侧端口序号
                        PortRole::DataType)                         // 角色：数据类型
            .value<NodeDataType>()                                  // 取出 NodeDataType
            .id;                                                    // 只要 id
    };

    // 判断端口是否还有空位（沿用库的默认策略：输入 One、输出 Many）
    auto portVacant = [&](PortType portType) {
        NodeId const nodeId = getNodeId(portType, connectionId);    // 该侧节点
        PortIndex const portIndex = getPortIndex(portType, connectionId); // 该侧端口序号
        auto const connected = connections(nodeId, portType, portIndex);  // 已有连接
        auto const policy = portData(nodeId, portType, portIndex,   // 取连接策略
                                     PortRole::ConnectionPolicyRole)
                                .value<ConnectionPolicy>();         // 转为枚举
        return connected.empty() || policy == ConnectionPolicy::Many; // 空位判断
    };

    if (!portVacant(PortType::Out) || !portVacant(PortType::In))    // 任一侧已满
        return false;                                              // 不允许连接

    QString const outId = dataTypeId(PortType::Out);                // 上游输出类型 id
    QString const inId = dataTypeId(PortType::In);                  // 下游输入类型 id

    if (outId == FlowPortType::execId() || inId == FlowPortType::execId()) // 涉及执行流
        return outId == inId;                                      // 执行流只能连执行流

    if (outId == FlowPortType::anyId() || inId == FlowPortType::anyId()) // 涉及通用型
        return true;                                               // 通用型可连任意数据

    if (outId == FlowPortType::intId() && inId == FlowPortType::floatId()) // 整数→浮点
        return true;                                               // 允许单向宽化

    return outId == inId;                                          // 其余要求类型一致
}

// 数据投递总闸门：非运行态时，投给输入端的数据/执行流一律丢弃
bool FlowGraphModel::setPortData(NodeId nodeId,
                                 PortType portType,
                                 PortIndex portIndex,
                                 QVariant const &data,
                                 PortRole dataRole)
{
    if (_runState != RunState::Running             // 当前不在运行态
        && dataRole == PortRole::Data              // 且这次是数据投递
        && portType == PortType::In)               // 且目标是输入端
        return false;                              // 直接丢弃，不触发节点执行

    // 运行态下把数据投递下沉到工作线程：节点里的休眠、串口等待就不再阻塞 UI
    if (_runState == RunState::Running             // 处于运行态
        && dataRole == PortRole::Data              // 且是数据投递
        && portType == PortType::In                // 且目标是输入端
        && _worker) {                              // 工作线程已就绪
        QMetaObject::invokeMethod(
            _worker,
            [this, nodeId, portIndex, data]() {    // 在工作线程里执行
                auto *model = delegateModel<NodeDelegateModel>(nodeId); // 取节点模型
                if (model)                          // 节点仍然存在
                    model->setInData(data.value<std::shared_ptr<NodeData>>(), portIndex); // 投递数据
            },
            Qt::QueuedConnection);                 // 排队执行，不阻塞调用方
        return true;                               // 视为已受理
    }

    return DataFlowGraphModel::setPortData(nodeId, portType, portIndex, data, dataRole); // 其余交给基类
}

// 切换运行状态
void FlowGraphModel::setRunState(RunState state)
{
    if (_runState == state)                        // 状态没有变化
        return;                                    // 无需处理

    _runState = state;                             // 更新状态
    Q_EMIT runStateChanged(_runState);             // 通知外部

    if (_runState == RunState::Running)            // 刚进入运行态
        triggerRun();                              // 触发一次数据传递
}

// 进入运行态时，从所有“无输入口”的源节点出发，触发一次全量传递
void FlowGraphModel::triggerRun()
{
    for (NodeId const nodeId : allNodeIds()) {                          // 遍历所有节点
        auto *model = delegateModel<NodeDelegateModel>(nodeId);         // 取节点模型
        if (!model)                                                     // 取不到则跳过
            continue;

        unsigned int const inCount = model->nPorts(PortType::In);       // 输入口数量
        if (inCount != 0)                                               // 只从源节点开始（如开始节点）
            continue;

        unsigned int const outCount = model->nPorts(PortType::Out);     // 输出口数量
        for (PortIndex outIndex = 0; outIndex < outCount; ++outIndex)   // 逐个输出口
            propagateFrom(nodeId, outIndex);                            // 向下游投递一次
    }
}

// 把某个输出口的数据投递给所有相连的下游输入口
void FlowGraphModel::propagateFrom(NodeId nodeId, PortIndex outPortIndex)
{
    QVariant const outData = portData(nodeId, PortType::Out, outPortIndex, PortRole::Data); // 上游输出数据

    for (ConnectionId const &connId : connections(nodeId, PortType::Out, outPortIndex))    // 遍历下游连接
        setPortData(connId.inNodeId, PortType::In, connId.inPortIndex, outData, PortRole::Data); // 投递
}