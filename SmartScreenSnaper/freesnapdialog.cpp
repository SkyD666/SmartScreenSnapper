#include "freesnapdialog.h"
#include "ui_freesnapdialog.h"
#include <QDebug>
#include <QApplication>
#include <windows.h>

FreeSnapDialog::FreeSnapDialog(QPixmap picture, QPixmap*& result, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreeSnapDialog),
    resultRef(result)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //    SetWindowPos((HWND)this->winId() , HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

    //可在不点击鼠标的情况下捕获移动事件
    setMouseTracking(true);

    this->noOperate = true;
    this->picture = picture;
    this->isDrawing = false;
    this->pictureMap = picture;
    this->pictureMap.fill(GRAYCOLOR);

    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(this->picture));
    setPalette(palette);
}

FreeSnapDialog::~FreeSnapDialog()
{
    resultRef = NULL;
    delete ui;
}

void FreeSnapDialog::paintEvent(QPaintEvent *)
{
    int width = endPoint.x() - lastPoint.x();
    int height = endPoint.y() - lastPoint.y();
    pictureMap = picture;
    QPainter painter(this);
    painter.setPen(QPen(RECTCOLOR, RECTPENWIDTH));

    QPoint topLeftPoint(endPoint.x() > lastPoint.x() ? lastPoint.x() : endPoint.x(),
                        endPoint.y() > lastPoint.y() ? lastPoint.y() : endPoint.y());
    QPoint bottomRightPoint(endPoint.x() < lastPoint.x() ? lastPoint.x() : endPoint.x(),
                            endPoint.y() < lastPoint.y() ? lastPoint.y() : endPoint.y());

    resultRect = QRect(topLeftPoint, bottomRightPoint);

    if (isDrawing) {
        pictureMap2 = pictureMap;
        QPainter p(&pictureMap2);
        p.setPen(QPen(RECTCOLOR, RECTPENWIDTH));
        //上侧
        p.fillRect(0, 0, this->width(), topLeftPoint.y() - RECTPENWIDTH / 2.0, GRAYCOLOR);
        //左侧
        p.fillRect(0, topLeftPoint.y() - RECTPENWIDTH / 2.0, topLeftPoint.x() - RECTPENWIDTH / 2.0, abs(height) + RECTPENWIDTH, GRAYCOLOR);
        //右侧
        p.fillRect(bottomRightPoint.x() + RECTPENWIDTH / 2.0, topLeftPoint.y() - RECTPENWIDTH / 2.0,
                   this->width() - bottomRightPoint.x() - RECTPENWIDTH / 2.0, abs(height) + RECTPENWIDTH, GRAYCOLOR);
        //下侧
        p.fillRect(0, bottomRightPoint.y() + RECTPENWIDTH / 2.0, this->width(), this->height() - bottomRightPoint.y(), GRAYCOLOR);

        //大框
        p.drawRect(lastPoint.x(), lastPoint.y(), width, height);

        //大框周围伸缩小点
        p.setPen(QPen(RECTPOINTCOLOR, 0));
        p.setBrush(QBrush(RECTPOINTCOLOR));
        //左上角的点
        p.drawRect(lastPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                   lastPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                   RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
        //右上角的点
        p.drawRect(endPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                   lastPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                   RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
        //左下角的点
        p.drawRect(lastPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                   endPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                   RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
        //右下角的点
        p.drawRect(endPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                   endPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                   RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);

        painter.drawPixmap(0, 0, pictureMap2);
    } else {
        QPainter p(&pictureMap);
        p.setPen(QPen(RECTCOLOR, RECTPENWIDTH));
        //上侧
        p.fillRect(0, 0, this->width(), topLeftPoint.y() - RECTPENWIDTH / 2.0, GRAYCOLOR);
        //左侧
        p.fillRect(0, topLeftPoint.y() - RECTPENWIDTH / 2.0, topLeftPoint.x() - RECTPENWIDTH / 2.0, abs(height) + RECTPENWIDTH, GRAYCOLOR);
        //右侧
        p.fillRect(bottomRightPoint.x() + RECTPENWIDTH / 2.0, topLeftPoint.y() - RECTPENWIDTH / 2.0,
                   this->width() - bottomRightPoint.x() - RECTPENWIDTH / 2.0, abs(height) + RECTPENWIDTH, GRAYCOLOR);
        //下侧
        p.fillRect(0, bottomRightPoint.y() + RECTPENWIDTH / 2.0, this->width(), this->height() - bottomRightPoint.y(), GRAYCOLOR);

        //大框
        p.drawRect(lastPoint.x(), lastPoint.y(), width, height);

        if (!noOperate) {
            //大框周围伸缩小点
            p.setPen(QPen(RECTPOINTCOLOR, 0));
            p.setBrush(QBrush(RECTPOINTCOLOR));
            //左上角的点
            p.drawRect(lastPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                       lastPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                       RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
            //右上角的点
            p.drawRect(endPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                       lastPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                       RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
            //左下角的点
            p.drawRect(lastPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                       endPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                       RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
            //右下角的点
            p.drawRect(endPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0,
                       endPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0,
                       RECTPOINTPENWIDTH, RECTPOINTPENHEIGHT);
        }

        painter.drawPixmap(0, 0, pictureMap);
    }
}

void FreeSnapDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint topLeftPoint(endPoint.x() > lastPoint.x() ? lastPoint.x() : endPoint.x(),
                            endPoint.y() > lastPoint.y() ? lastPoint.y() : endPoint.y());
        QPoint bottomRightPoint(endPoint.x() < lastPoint.x() ? lastPoint.x() : endPoint.x(),
                                endPoint.y() < lastPoint.y() ? lastPoint.y() : endPoint.y());
        if (inLeftTopPoint(event)) {
            lastPoint = bottomRightPoint;
            endPoint = topLeftPoint;
            this->noOperate = false;
            isDrawing = true;
        } else if (inRightTopPoint(event)) {
            lastPoint.setX(topLeftPoint.x());
            lastPoint.setY(bottomRightPoint.y());
            endPoint.setX(bottomRightPoint.x());
            endPoint.setY(topLeftPoint.y());
            this->noOperate = false;
            isDrawing = true;
        } else if (inLeftBottomPoint(event)) {
            lastPoint.setX(bottomRightPoint.x());
            lastPoint.setY(topLeftPoint.y());
            endPoint.setX(topLeftPoint.x());
            endPoint.setY(bottomRightPoint.y());
            this->noOperate = false;
            isDrawing = true;
        } else if (inRightBottomPoint(event)) {
            lastPoint = topLeftPoint;
            endPoint = bottomRightPoint;
            this->noOperate = false;
            isDrawing = true;
        } else {
            this->noOperate = false;
            QPainter painter(this);
            painter.drawPixmap(0, 0, pictureMap);
            isDrawing = true;
            lastPoint = event->pos();
        }
    }
}

void FreeSnapDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        endPoint = event->pos();
        update();
    }
    if (inLeftTopPoint(event)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (inRightTopPoint(event)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (inLeftBottomPoint(event)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (inRightBottomPoint(event)) {
        setCursor(Qt::SizeFDiagCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void FreeSnapDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDrawing = false;
        endPoint = event->pos();
        update();
    }
}

void FreeSnapDialog::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Return || event->key() ==Qt::Key_Enter) && !noOperate) {
        picture.copy(resultRect);
        *this->resultRef = picture.copy(resultRect);
        close();
        return;
    }
    if(event->key() == Qt::Key_Escape) {
        delete this->resultRef;
        this->resultRef = NULL;
        close();
        return;
    }
}

void FreeSnapDialog::keyReleaseEvent(QKeyEvent *)
{

}

bool FreeSnapDialog::inLeftTopPoint(QMouseEvent *event)
{
    QPoint topLeftPoint(endPoint.x() > lastPoint.x() ? lastPoint.x() : endPoint.x(),
                        endPoint.y() > lastPoint.y() ? lastPoint.y() : endPoint.y());
    return event->x() >= topLeftPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0 &&
            event->x() <= topLeftPoint.x() - RECTPENWIDTH / 2.0 + RECTPOINTPENWIDTH / 2.0 &&
            event->y() >= topLeftPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0 &&
            event->y() <= topLeftPoint.y() - RECTPENWIDTH / 2.0 + RECTPOINTPENHEIGHT / 2.0;
}

bool FreeSnapDialog::inRightTopPoint(QMouseEvent *event)
{
    QPoint topLeftPoint(endPoint.x() > lastPoint.x() ? lastPoint.x() : endPoint.x(),
                        endPoint.y() > lastPoint.y() ? lastPoint.y() : endPoint.y());
    QPoint bottomRightPoint(endPoint.x() < lastPoint.x() ? lastPoint.x() : endPoint.x(),
                            endPoint.y() < lastPoint.y() ? lastPoint.y() : endPoint.y());
    return event->x() >= bottomRightPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0 &&
            event->x() <= bottomRightPoint.x() - RECTPENWIDTH / 2.0 + RECTPOINTPENWIDTH / 2.0 &&
            event->y() >= topLeftPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0 &&
            event->y() <= topLeftPoint.y() - RECTPENWIDTH / 2.0 + RECTPOINTPENHEIGHT / 2.0;
}

bool FreeSnapDialog::inLeftBottomPoint(QMouseEvent *event)
{
    QPoint topLeftPoint(endPoint.x() > lastPoint.x() ? lastPoint.x() : endPoint.x(),
                        endPoint.y() > lastPoint.y() ? lastPoint.y() : endPoint.y());
    QPoint bottomRightPoint(endPoint.x() < lastPoint.x() ? lastPoint.x() : endPoint.x(),
                            endPoint.y() < lastPoint.y() ? lastPoint.y() : endPoint.y());
    return event->x() >= topLeftPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0 &&
            event->x() <= topLeftPoint.x() - RECTPENWIDTH / 2.0 + RECTPOINTPENWIDTH / 2.0 &&
            event->y() >= bottomRightPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0 &&
            event->y() <= bottomRightPoint.y() - RECTPENWIDTH / 2.0 + RECTPOINTPENHEIGHT / 2.0;
}

bool FreeSnapDialog::inRightBottomPoint(QMouseEvent *event)
{
    QPoint bottomRightPoint(endPoint.x() < lastPoint.x() ? lastPoint.x() : endPoint.x(),
                            endPoint.y() < lastPoint.y() ? lastPoint.y() : endPoint.y());
    return event->x() >= bottomRightPoint.x() - RECTPENWIDTH / 2.0 - RECTPOINTPENWIDTH / 2.0 &&
            event->x() <= bottomRightPoint.x() - RECTPENWIDTH / 2.0 + RECTPOINTPENWIDTH / 2.0 &&
            event->y() >= bottomRightPoint.y() - RECTPENWIDTH / 2.0 - RECTPOINTPENHEIGHT / 2.0 &&
            event->y() <= bottomRightPoint.y() - RECTPENWIDTH / 2.0 + RECTPOINTPENHEIGHT / 2.0;
}
