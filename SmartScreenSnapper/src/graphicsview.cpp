#include "graphicsview.h"
#include <QScrollBar>
#include <QApplication>
#include <QDrag>
#include <QBuffer>

GraphicsView::GraphicsView(QWidget *parent): QGraphicsView(parent), mimeData(nullptr)
{
    this->mousePressed = false;
    lastMousePoint.setX(-1);
    lastMousePoint.setY(-1);
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


void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    mousePressed = true;
    lastMousePoint.setX(event->x());
    lastMousePoint.setY(event->y());

    if (event->buttons() & Qt::RightButton) {       // 准备右键拖出时的图片
        QPixmap pixmap(scene()->width(), scene()->height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        scene()->render(&painter);
        mimeData = new QMimeData();
        mimeData->clear();
        mimeData->setImageData(pixmap.toImage());
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    mousePressed = false;
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (mousePressed) {
            this->horizontalScrollBar()->setSliderPosition(this->horizontalScrollBar()->sliderPosition() -
                                                           event->x() + lastMousePoint.x());
            this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition() -
                                                         event->y() + lastMousePoint.y());
        }
    }
    if (event->buttons() & Qt::RightButton) {
        QDrag* drag = new QDrag(this);      //The QDrag must be constructed on the heap with a parent QObject
        drag->setMimeData(mimeData);
        drag->setHotSpot(event->pos());
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }
    lastMousePoint.setX(event->x());
    lastMousePoint.setY(event->y());
}
