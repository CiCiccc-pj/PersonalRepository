#include "flowview.h"                          // 本类头文件

#include "nodetreeview.h"                       // 复用其 MIME 类型定义

#include <QtCore/QMimeData>                    // 拖拽数据
#include <QtGui/QDragEnterEvent>               // 拖拽进入事件
#include <QtGui/QDropEvent>                    // 放下事件
#include <QtNodes/AbstractGraphModel>          // 图模型基类
#include <QtNodes/BasicGraphicsScene>          // 场景，用于取图模型
#include <QtNodes/Definitions>                 // NodeId / NodeRole 等定义

// 构造函数（无场景）
FlowView::FlowView(QWidget *parent)
    : QtNodes::GraphicsView(parent)            // 构造基类
{
    setAcceptDrops(true);                      // 开启接受拖放
}

// 构造函数（绑定场景）
FlowView::FlowView(QtNodes::BasicGraphicsScene *scene, QWidget *parent)
    : QtNodes::GraphicsView(scene, parent)     // 构造基类并绑定场景
{
    setAcceptDrops(true);                      // 开启接受拖放
}

// 拖拽进入：仅接受节点面板的拖拽
void FlowView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(NodeTreeView::nodeMimeType())) // 是本面板的节点
        event->acceptProposedAction();                               // 接受
    else
        QtNodes::GraphicsView::dragEnterEvent(event);                // 交给基类处理
}

// 拖拽移动：保持接受状态
void FlowView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(NodeTreeView::nodeMimeType())) // 是本面板的节点
        event->acceptProposedAction();                               // 接受
    else
        QtNodes::GraphicsView::dragMoveEvent(event);                 // 交给基类处理
}

// 放下：在落点创建节点
void FlowView::dropEvent(QDropEvent *event)
{
    QMimeData const *mimeData = event->mimeData();                    // 取拖拽数据
    if (!mimeData->hasFormat(NodeTreeView::nodeMimeType())) {         // 非本面板节点
        QtNodes::GraphicsView::dropEvent(event);                      // 交给基类处理
        return;                                                       // 结束
    }

    QString const nodeType = QString::fromUtf8(mimeData->data(NodeTreeView::nodeMimeType())); // 读出类型名

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint const viewPos = event->position().toPoint();               // Qt6：落点坐标
#else
    QPoint const viewPos = event->pos();                              // Qt5：落点坐标
#endif

    createNodeAt(nodeType, viewPos);                                  // 在落点创建节点
    event->acceptProposedAction();                                    // 标记已接受
}

// 在指定视图坐标处创建节点
void FlowView::createNodeAt(QString const &nodeType, QPoint const &viewPos)
{
    QtNodes::BasicGraphicsScene *scene = nodeScene();                 // 取当前场景
    if (!scene)                                                       // 场景为空
        return;                                                       // 直接返回

    QtNodes::AbstractGraphModel &graphModel = scene->graphModel();    // 取图模型
    QtNodes::NodeId const nodeId = graphModel.addNode(nodeType);      // 创建节点
    if (nodeId == QtNodes::InvalidNodeId)                             // 类型无效
        return;                                                       // 直接返回

    graphModel.setNodeData(nodeId,                                    // 设置节点数据
                           QtNodes::NodeRole::Position,               // 角色：位置
                           mapToScene(viewPos));                      // 视图坐标转场景坐标
}