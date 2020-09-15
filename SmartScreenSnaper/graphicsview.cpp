#include "graphicsview.h"
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent): QGraphicsView(parent)
{

}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees=event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        if (event->modifiers() == Qt::ControlModifier) {
            if (numDegrees.y() > 0) {
                emit zoom(10);
            } else if (numDegrees.y() < 0) {
                emit zoom(-10);
            }
            event->accept();
            return;
        }  else if (event->modifiers() == Qt::ShiftModifier) {
            if (numDegrees.y() != 0) {
                this->horizontalScrollBar()->setSliderPosition(this->horizontalScrollBar()->sliderPosition() -
                                                               event->delta());
            }
            event->accept();
            return;
        }
    }
    QGraphicsView::wheelEvent(event);
}
