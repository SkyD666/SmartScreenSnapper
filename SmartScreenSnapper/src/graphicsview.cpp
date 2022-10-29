#include "graphicsview.h"
#include <QScrollBar>
#include <QApplication>
#include <QDrag>
#include <QBuffer>

GraphicsView::GraphicsView(QWidget *parent):
    QGraphicsView(parent),
    mousePressed(false),
    lastMousePoint(-1, -1),
    mimeData(nullptr)
{

}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;
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
                horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() -
                                                         event->angleDelta().y());
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
    lastMousePoint.setX(event->position().x());
    lastMousePoint.setY(event->position().y());

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

void GraphicsView::mouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (mousePressed) {
            horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() -
                                                     event->position().x() + lastMousePoint.x());
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() -
                                                   event->position().y() + lastMousePoint.y());
        }
    }
    if (event->buttons() & Qt::RightButton) {
        QDrag* drag = new QDrag(this);      //The QDrag must be constructed on the heap with a parent QObject
        drag->setMimeData(mimeData);
        drag->setHotSpot(event->pos());
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    } else {
        delete mimeData;
    }
    lastMousePoint.setX(event->position().x());
    lastMousePoint.setY(event->position().y());
}
