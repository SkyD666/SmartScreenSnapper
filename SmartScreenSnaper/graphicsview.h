#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

signals:

    void zoom(int n);

public:
    GraphicsView(QWidget *parent = nullptr);

    void wheelEvent(QWheelEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool mousePressed;

    QPoint lastMousePoint;
};

#endif // GRAPHICSVIEW_H
