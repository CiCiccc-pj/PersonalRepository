#include "flowconnectionpainter.h"              // 本类头文件

#include <QtNodes/internal/ConnectionGraphicsObject.hpp> // 连线图元（端点与控制点）
#include <QtNodes/internal/ConnectionState.hpp>          // 连线状态（是否还在拖拽）
#include <QtNodes/internal/StyleCollection.hpp>          // 全局样式

using namespace QtNodes;                       // 使用 QtNodes 命名空间

// 连线的三次贝塞尔路径（与库默认画法保持一致的几何）
QPainterPath FlowConnectionPainter::cubicPath(ConnectionGraphicsObject const &cgo) const
{
    QPointF const &out = cgo.endPoint(PortType::Out);  // 上游端点
    QPointF const &in = cgo.endPoint(PortType::In);    // 下游端点
    auto const c1c2 = cgo.pointsC1C2();                // 两个控制点

    QPainterPath path(out);                    // 从上游端点起笔
    path.cubicTo(c1c2.first, c1c2.second, in); // 画三次贝塞尔曲线
    return path;                               // 返回路径
}

// 推进虚线偏移：达到上限后回绕，避免数值无限增大
void FlowConnectionPainter::advancePhase(double step)
{
    _phase += step;                            // 前进一个步长
    if (_phase > 1000.0)                       // 数值过大时
        _phase = 0.0;                          // 回绕归零
}

// 绘制连线：运行态画流动虚线，其余情况交给默认画法
void FlowConnectionPainter::paint(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    if (!_running || cgo.connectionState().requiresPort()) { // 非运行态，或仍在拖拽的草稿连线
        DefaultConnectionPainter::paint(painter, cgo);       // 沿用默认画法
        return;                                              // 结束
    }

    auto const &style = StyleCollection::connectionStyle();  // 全局连线样式
    double const lineWidth = style.lineWidth();              // 线宽

    QPen pen;                                                // 虚线画笔
    pen.setWidth(lineWidth);                                 // 设置线宽
    pen.setColor(cgo.isSelected() ? style.selectedColor()    // 选中时用选中色
                                  : style.normalColor());    // 否则用普通色
    pen.setStyle(Qt::CustomDashLine);                        // 使用自定义虚线
    pen.setDashPattern({5.0, 3.0});                          // 虚线段长与间隔（按线宽倍数）
    pen.setDashOffset(_phase);                               // 偏移量让虚线看起来在流动
    pen.setCapStyle(Qt::FlatCap);                            // 平头端点，虚线更规整

    painter->setPen(pen);                                    // 应用画笔
    painter->setBrush(Qt::NoBrush);                          // 不填充
    painter->drawPath(cubicPath(cgo));                       // 画流动虚线

    double const radius = style.pointDiameter() / 2.0;       // 端点圆点半径
    painter->setPen(style.constructionColor());              // 端点画笔
    painter->setBrush(style.constructionColor());            // 端点填充
    painter->drawEllipse(cgo.out(), radius, radius);         // 上游端点圆点
    painter->drawEllipse(cgo.in(), radius, radius);          // 下游端点圆点
}