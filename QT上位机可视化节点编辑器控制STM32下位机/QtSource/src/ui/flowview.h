#ifndef FLOWVIEW_H
#define FLOWVIEW_H

#include <QtNodes/GraphicsView>                // 基类：QtNodes 画布视图

/// 右侧画布视图。
/// 接收从 NodeTreeView 拖入的节点项，并在鼠标落点处创建对应的节点。
class FlowView : public QtNodes::GraphicsView
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    explicit FlowView(QWidget *parent = nullptr);                              // 构造函数

    FlowView(QtNodes::BasicGraphicsScene *scene, QWidget *parent = nullptr);   // 构造函数（绑定场景）

protected:
    void dragEnterEvent(QDragEnterEvent *event) override; // 重写：拖拽进入

    void dragMoveEvent(QDragMoveEvent *event) override;   // 重写：拖拽移动

    void dropEvent(QDropEvent *event) override;           // 重写：放下

private:
    void createNodeAt(QString const &nodeType, QPoint const &viewPos); // 在落点创建节点
};

#endif // FLOWVIEW_H