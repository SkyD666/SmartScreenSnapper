#include "undomove.h"
#include <QGraphicsScene>

UndoMove::UndoMove(QGraphicsItem *graphicsItem, const QPointF oldPos, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("移动"), parent),
      graphicsItem(graphicsItem),
      oldPos(oldPos),
      newPos(graphicsItem->pos())
{

}

void UndoMove::undo()
{
    graphicsItem->setPos(oldPos);
    graphicsItem->scene()->update();
}

void UndoMove::redo()
{
    graphicsItem->setPos(newPos);
    graphicsItem->scene()->update();
}
