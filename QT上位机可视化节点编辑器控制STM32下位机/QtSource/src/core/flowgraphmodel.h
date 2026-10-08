#ifndef FLOWGRAPHMODEL_H
#define FLOWGRAPHMODEL_H

#include <QtCore/QVariant>                     // setPortData 的参数类型
#include <QtNodes/DataFlowGraphModel>          // 基类：数据流图模型
#include <QtNodes/Definitions>                 // NodeId / PortType 等定义

#include <memory>                              // std::shared_ptr

namespace QtNodes {
class NodeDelegateModelRegistry;               // 前置声明：节点注册表
}

class QThread;                                 // 前置声明：执行流程工作线程

/// 自定义图模型，承担三件事：
/// 1. 连接校验：执行流只连执行流、通用型连任意数据、整数可宽化为浮点；
/// 2. 运行状态机：只有处于运行态，数据与执行流才会真正投递给节点，
///    从而避免“连线一建立就立即执行”的问题；
/// 3. 线程分离：运行态下把数据投递下沉到工作线程执行，避免休眠、串口等待阻塞 UI。
class FlowGraphModel : public QtNodes::DataFlowGraphModel
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    enum class RunState { Stopped, Running };  // 运行状态：停止态 / 运行态
    Q_ENUM(RunState)                           // 注册为元类型

    explicit FlowGraphModel(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry); // 构造函数

    ~FlowGraphModel() override;                // 析构：停止并回收工作线程

    bool connectionPossible(QtNodes::ConnectionId const connectionId) const override; // 覆写：连接校验

    // 覆写：数据投递的总闸门，非运行态时不把数据送到输入端
    bool setPortData(QtNodes::NodeId nodeId,
                     QtNodes::PortType portType,
                     QtNodes::PortIndex portIndex,
                     QVariant const &data,
                     QtNodes::PortRole dataRole) override;

    RunState runState() const { return _runState; } // 读取当前运行状态

    void start() { setRunState(RunState::Running); } // 进入运行态

    void stop() { setRunState(RunState::Stopped); }  // 进入停止态

    void setRunState(RunState state);                // 切换运行状态

Q_SIGNALS:
    void runStateChanged(RunState state);      // 运行状态变化的通知

private:
    void triggerRun();                                          // 进入运行态时触发一次全量传递

    void propagateFrom(QtNodes::NodeId nodeId,                  // 从某个输出口向下游投递一次
                       QtNodes::PortIndex outPortIndex);

    RunState _runState = RunState::Stopped;    // 当前状态，默认停止

    QThread *_workerThread = nullptr;          // 执行流程工作线程（跑事件循环）
    QObject *_worker = nullptr;                // 工作线程内的调度对象（接收排队任务）
};

#endif // FLOWGRAPHMODEL_H