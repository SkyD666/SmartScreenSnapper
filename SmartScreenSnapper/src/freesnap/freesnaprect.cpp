#include "freesnaprect.h"
#include <QMouseEvent>

FreeSnapRect::FreeSnapRect(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
    setMouseTracking(true);
}

void FreeSnapRect::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    emit mouseMove(event->pos());
}
