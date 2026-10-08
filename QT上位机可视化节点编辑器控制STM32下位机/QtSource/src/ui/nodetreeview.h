#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QtWidgets/QTreeView>                 // 基类：树视图

#include <memory>                              // std::shared_ptr

class QStandardItemModel;                      // 前置声明：树项数据模型

namespace QtNodes {
class NodeDelegateModelRegistry;               // 前置声明：节点注册表
}

/// 左侧节点面板（树视图）。
/// 顶层项是节点分类，子项是该分类下的节点。分类不做真正的展开/收起，
/// 而是由 NodeItemDelegate 画成一个分组框，把该分类的节点框在一起。
/// 节点项可以拖拽到右侧画布（见 FlowView）。
/// 节点项由 NodeDelegateModelRegistry 自动生成，因此后续新增节点类型时
/// 只需注册新的 NodeDelegateModel，无需改动本类即可出现在面板中。
class NodeTreeView : public QTreeView
{
    Q_OBJECT                                   // Qt 元对象宏，启用信号槽

public:
    explicit NodeTreeView(QWidget *parent = nullptr); // 构造函数

    /// 设置节点注册表并据此刷新分类与节点项。
    void setRegistry(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry);

    /// 依据当前注册表重新生成分类与节点项。
    void reload();

    /// 拖拽时使用的 MIME 类型，FlowView 通过它识别节点类型。
    static char const *nodeMimeType();

protected:
    void startDrag(Qt::DropActions supportedActions) override; // 重写：发起拖拽

private:
    void applyTheme();                          // 应用画布同款主题

    QPixmap makeDragPixmap(QString const &text) const; // 生成拖拽预览图

    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> _registry; // 节点注册表
    QStandardItemModel *_model = nullptr;       // 树项数据模型
};

#endif // NODETREEVIEW_H