#include "graphicsview.h"
#include "src/publicdata.h"
#include <QScrollBar>
#include <QApplication>
#include <QDrag>
#include <QDir>
#include <QBuffer>
#include <QTemporaryFile>

GraphicsView::GraphicsView(QWidget *parent):
    QGraphicsView(parent),
    mousePressed(false),
    lastMousePoint(-1, -1),
    fileName("")
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
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
}

void GraphicsView::setFileName(const QString &newFileName)
{
    fileName = newFileName;
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
    if (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::ControlModifier) {
        QPixmap pixmap(scene()->width(), scene()->height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        scene()->render(&painter);

        QPixmap dragPixmap(scene()->width(), scene()->height());
        dragPixmap.fill(Qt::transparent);
        QPainter dragPixmapPainter(&dragPixmap);
        dragPixmapPainter.setOpacity(0.5f);
        dragPixmapPainter.drawPixmap(0, 0, pixmap);

        QTemporaryFile tempFile = QTemporaryFile();
        tempFile.open();
        if (!fileName.isEmpty()) {
            tempFile.rename(QDir::tempPath() + "/" +  fileName + PublicData::imageExtName[0].first);
        }
        pixmap.save(&tempFile, nullptr, PublicData::saveImageQuality);

        QMimeData *mimeData = new QMimeData();
        mimeData->clear();
        mimeData->setImageData(pixmap.toImage());
        mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(tempFile.fileName()));
        QDrag *drag = new QDrag(this);      // The QDrag must be constructed on the heap with a parent QObject
        drag->setPixmap(dragPixmap);
        drag->setMimeData(mimeData);
        drag->setHotSpot(mapToScene(event->pos()).toPoint());
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }
    lastMousePoint.setX(event->position().x());
    lastMousePoint.setY(event->position().y());
}
