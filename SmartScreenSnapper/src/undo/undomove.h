#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include <QGraphicsItem>
#include <QPoint>
#include <QUndoCommand>

class UndoMove : public QUndoCommand
{
public:
    UndoMove(QGraphicsItem *graphicsItem, const QPointF oldPos, QUndoCommand *parent = nullptr);

    void undo();
    void redo();

private:
    QGraphicsItem *graphicsItem;
    QPointF oldPos;
    QPointF newPos;
};

#endif // UNDOMOVE_H
