#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>                         // 基类：主窗口
#include <QtNodes/Definitions>                 // NodeId 等定义

#include <memory>                              // 智能指针

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;                              // 前置声明：界面对象
}
QT_END_NAMESPACE

namespace QtNodes {
class NodeDelegateModelRegistry;               // 前置声明：节点注册表
class DataFlowGraphicsScene;                   // 前置声明：场景
} // namespace QtNodes

class NodeTreeView;                            // 前置声明：左侧节点面板
class FlowView;                                // 前置声明：右侧画布视图
class FlowGraphModel;                          // 前置声明：自定义图模型
class FlowConnectionPainter;                   // 前置声明：自定义连线绘制
class QAction;                                 // 前置声明：菜单动作
class QTimer;                                  // 前置声明：定时器

class MainWindow : public QMainWindow
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    explicit MainWindow(QWidget *parent = nullptr); // 构造函数

    ~MainWindow() override;                     // 析构函数

private:
    void setupNodeEditor();                     // 初始化节点编辑器

    void setupRunMenu();                        // 初始化「运行」菜单

    void setupSerialMenu();                     // 初始化「串口」菜单

    void onConnectSerial();                     // 菜单：连接串口

    void onDisconnectSerial();                  // 菜单：断开串口

    // 节点右键菜单：提供删除操作（scenePos 为场景坐标）
    void onNodeContextMenu(QtNodes::NodeId nodeId, QPointF const &scenePos);

    void onRun();                               // 菜单：进入运行态

    void onStop();                              // 菜单：回到停止态

    void onAnimationTick();                     // 虚线动画：推进偏移并重绘连线

    void refreshConnections();                  // 重绘场景中所有连线

    /// 在此注册所有可用的节点模型。
    /// 新增节点类型时只需实现 NodeDelegateModel 并在此处注册，
    /// 左侧面板会自动生成对应的节点项。
    void registerNodeModels();

    Ui::MainWindow *ui;                         // 界面对象

    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> _registry; // 节点注册表
    // 声明顺序保证 _scene 先于 _graphModel 析构。
    std::unique_ptr<FlowGraphModel> _graphModel;                   // 自定义图模型
    std::unique_ptr<QtNodes::DataFlowGraphicsScene> _scene;        // 场景
    FlowView *_view = nullptr;                  // 右侧画布视图
    NodeTreeView *_nodeTreeView = nullptr;      // 左侧节点面板
    QAction *_runAction = nullptr;              // 菜单项：运行
    QAction *_stopAction = nullptr;             // 菜单项：停止
    QAction *_connectSerialAction = nullptr;    // 菜单项：连接串口
    QAction *_disconnectSerialAction = nullptr; // 菜单项：断开串口
    FlowConnectionPainter *_connectionPainter = nullptr; // 自定义连线绘制（所有权在场景）
    QTimer *_animationTimer = nullptr;          // 运行态下的虚线动画定时器
};
#endif // MAINWINDOW_H