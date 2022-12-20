#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GraphicsScene(QObject *parent = nullptr);

signals:
    void itemMoved(QGraphicsItem *movedItem, const QPointF &movedFromPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsItem *movingItem = nullptr;
    QPointF oldPos;
};

#endif // GRAPHICSSCENE_H
