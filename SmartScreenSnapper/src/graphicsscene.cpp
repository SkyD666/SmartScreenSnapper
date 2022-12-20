#include "graphicsscene.h"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

GraphicsScene::GraphicsScene(QObject *parent)
    : QGraphicsScene{parent}
{

}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(),
                     event->buttonDownScenePos(Qt::LeftButton).y());
    const QList<QGraphicsItem *> itemList = items(mousePos);
    movingItem = itemList.isEmpty() ? nullptr : itemList.first();

    if (movingItem != nullptr && event->button() == Qt::LeftButton)
        oldPos = movingItem->pos();

    clearSelection();
    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (movingItem != nullptr && event->button() == Qt::LeftButton) {
        if (oldPos != movingItem->pos())
            emit itemMoved(qgraphicsitem_cast<QGraphicsItem *>(movingItem),
                           oldPos);
        movingItem = nullptr;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}
