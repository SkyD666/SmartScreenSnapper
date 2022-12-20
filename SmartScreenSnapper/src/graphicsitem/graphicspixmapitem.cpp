#include "graphicspixmapitem.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <src/mdiwindow.h>

GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant GraphicsPixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsScene *s = scene();
    if (s) {
        QList<QGraphicsView *> views = s->views();
        for (auto view : views) {
            auto *p = qobject_cast<MdiWindow*>(view->parent()->parent());
            if (p) {
                p->setSaved(false);
            }
        }
    }
    return value;
}
