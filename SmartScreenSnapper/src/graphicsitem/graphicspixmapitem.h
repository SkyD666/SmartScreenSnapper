#ifndef GRAPHICSPIXMAPITEM_H
#define GRAPHICSPIXMAPITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class GraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    GraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // GRAPHICSPIXMAPITEM_H
