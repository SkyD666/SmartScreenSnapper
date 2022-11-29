#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMimeData>

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

    void setFileName(const QString &newFileName);

private:
    bool mousePressed;

    QPoint lastMousePoint;

    QString fileName;
};

#endif // GRAPHICSVIEW_H
