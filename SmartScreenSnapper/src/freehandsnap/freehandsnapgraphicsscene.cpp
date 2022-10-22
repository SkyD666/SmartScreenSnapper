#include "freehandsnapgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPolygonItem>
#include <QRegion>
#include <QDebug>

FreeHandSnapGraphicsScene::FreeHandSnapGraphicsScene(QWidget *parent) :
    QGraphicsScene(parent),
    mousePressed(false),
    unsuredPath(nullptr),
    hasNewUnsuredPath(false),
    path(),
    pathItem(nullptr),
    grayItem(nullptr),
    grayColor(0, 0, 0, 180)
{
    connect(this, &QGraphicsScene::sceneRectChanged, this, [=](){
        refreshGrayArea();
    });
}

FreeHandSnapGraphicsScene::~FreeHandSnapGraphicsScene()
{
    if (unsuredPath) delete unsuredPath;
    if (path) delete path;
}

QPolygonF FreeHandSnapGraphicsScene::getPathPolygonF()
{
    auto p = QPainterPath(*path);
    QPointF point = pathItem->pos();
    p.translate(-point.x(), -point.y());
    return p.toFillPolygon();
}

QRectF FreeHandSnapGraphicsScene::getPathRect()
{
    return path->boundingRect();
}

void FreeHandSnapGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressed = true;
    hasNewUnsuredPath = false;
    if (!path) {
        path = new QPainterPath(event->scenePos());
    }
}

void FreeHandSnapGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    mousePressed = false;

    if (hasNewUnsuredPath) {
        path->lineTo(unsuredPath->currentPosition());
        hasNewUnsuredPath = false;
        refreshPathItem(*path);
    }
}

void FreeHandSnapGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mousePressed) {
        path->lineTo(event->scenePos());
        refreshPathItem(*path);
    } else if (path) {
        if (unsuredPath) {
            unsuredPath->clear();
            unsuredPath->addPath(*path);
        } else {
            unsuredPath = new QPainterPath(*path);
        }
        unsuredPath->lineTo(event->scenePos());
        hasNewUnsuredPath = true;
        refreshPathItem(*unsuredPath);
    }
}

// 更新区域和阴影
void FreeHandSnapGraphicsScene::refreshPathItem(QPainterPath path)
{
    refreshGrayArea();
    if (pathItem) {
        pathItem->setPath(path);
    } else {
        pathItem = addPath(path, QPen(QBrush(Qt::yellow), 5));
    }
}


// 绘制灰色阴影
void FreeHandSnapGraphicsScene::refreshGrayArea()
{
    if (grayItem) {
        grayItem->setPolygon(QPolygonF(sceneRect()).subtracted(path->toFillPolygon()));
    } else {
        grayItem = addPolygon(QPolygonF(sceneRect()), QPen(Qt::transparent), QBrush(grayColor));
    }
}
