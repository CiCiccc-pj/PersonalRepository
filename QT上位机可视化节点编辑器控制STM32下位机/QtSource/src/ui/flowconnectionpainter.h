#ifndef FLOWCONNECTIONPAINTER_H
#define FLOWCONNECTIONPAINTER_H

#include <QtNodes/internal/DefaultConnectionPainter.hpp> // 基类：库默认的连线绘制

/// 自定义连线绘制。
/// 停止态沿用库默认画法；运行态把连线改为流动的虚线，表示数据正在传递。
class FlowConnectionPainter : public QtNodes::DefaultConnectionPainter
{
public:
    void paint(QPainter *painter,
               QtNodes::ConnectionGraphicsObject const &cgo) const override; // 重写：绘制连线

    void setRunning(bool running) { _running = running; } // 切换运行态外观

    void advancePhase(double step);            // 推进虚线偏移（产生流动动画）

private:
    QPainterPath cubicPath(QtNodes::ConnectionGraphicsObject const &cgo) const; // 连线的贝塞尔路径

    bool _running = false;                     // 是否处于运行态
    double _phase = 0.0;                       // 虚线偏移量（动画进度）
};

#endif // FLOWCONNECTIONPAINTER_H