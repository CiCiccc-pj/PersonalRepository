#ifndef NODEITEMDELEGATE_H
#define NODEITEMDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>       // 基类：项绘制代理

/// 左侧节点面板的绘制代理。
/// 顶层项（分类）与它的子项（节点）共同拼成一个“分组框”：
/// 分类行画框的顶部圆角与分类标题，节点行画框的左右边框和框内的节点项，
/// 分类的最后一行再补上底部圆角。分类不需要真正的展开/收起。
class NodeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT                                   // Qt 元对象宏

public:
    explicit NodeItemDelegate(QObject *parent = nullptr); // 构造函数

    static constexpr int NodeRowHeight = 44;   // 节点行高度（像素）
    static constexpr int CategoryRowHeight = 34; // 分类标题行高度（像素）

    void paint(QPainter *painter,
               QStyleOptionViewItem const &option,
               QModelIndex const &index) const override; // 重写：绘制整行

    QSize sizeHint(QStyleOptionViewItem const &option,
                   QModelIndex const &index) const override; // 重写：行高

private:
    // 绘制分类标题行（分组框顶部）
    void paintCategory(QPainter *painter,
                       QStyleOptionViewItem const &option,
                       QModelIndex const &index,
                       QRect const &row,
                       QRect const &box) const;

    // 绘制节点行（分组框内部的一个节点项）
    void paintNode(QPainter *painter,
                   QStyleOptionViewItem const &option,
                   QModelIndex const &index,
                   QRect const &row,
                   QRect const &box) const;
};

#endif // NODEITEMDELEGATE_H