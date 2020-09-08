#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

signals:

    void zoom(int n);

public:
    GraphicsView(QWidget *parent = nullptr);


};

#endif // GRAPHICSVIEW_H
