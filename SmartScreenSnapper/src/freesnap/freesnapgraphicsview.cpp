#include "freesnapgraphicsview.h"

#include <QMouseEvent>

FreeSnapGraphicsView::FreeSnapGraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    ignoreMouseEvent(false)
{
    setMouseTracking(true);
}

void FreeSnapGraphicsView::setIgnoreMouseEvent(bool ignore)
{
    ignoreMouseEvent = ignore;
}

void FreeSnapGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (ignoreMouseEvent) event->ignore();
}

void FreeSnapGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (ignoreMouseEvent) event->ignore();
}

void FreeSnapGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (ignoreMouseEvent) event->ignore();
}
