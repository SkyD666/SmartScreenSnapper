#include "freesnapdialog.h"
#include "screenshothelper.h"
#include "ui_freesnapdialog.h"
#include <QDebug>
#include <QApplication>
#include <QGuiApplication>
#include <windows.h>
#include <QtMath>
#include <QGraphicsView>
#include <QPainterPath>
#include <QGraphicsPolygonItem>
#include <QClipboard>
#include <QString>

FreeSnapDialog::FreeSnapDialog(QPixmap* result, bool &captured, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreeSnapDialog),
    grayColor(0, 0, 0, 180),
    grayItem(nullptr),
    previewZoomRate(3.2f),
    pixelColor(nullptr),
    resultPixmap(result),
    captured(captured),
    rectLineWidth(1),
    pointRadius(9),
    deltaHeight(0),
    deltaWidth(0),
    hasRect(false),
    mousePressed(false),
    pressedInRectArea(false),
    pressedInLeftTopArea(false),
    pressedInRightTopArea(false),
    pressedInLeftBottomArea(false),
    pressedInRightBottomArea(false),
    pressedInLeftArea(false),
    pressedInRightArea(false),
    pressedInTopArea(false),
    pressedInBottomArea(false)
{
    ui->setupUi(this);

    picture = ScreenShotHelper::getFullScreen();
    image = picture.toImage();

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowState(Qt::WindowActive);
    //多显示器支持
    move(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN));
    resize(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));

    ui->widgetInfoContainer->adjustSize();
    ui->widgetInfoContainer->setVisible(false);
    ui->widgetInfoContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);      //信息Widget不接收鼠标事件

    // 可在不点击鼠标的情况下捕获移动事件
    setMouseTracking(true);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(QRectF(0, 0, width(), height()));
    scene->setBackgroundBrush(QBrush(picture));
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setGeometry(0, 0, width(), height());
    ui->graphicsView->setIgnoreMouseEvent(true);

    QString frameRectStyle = QString("border: ") +
            QString::number(rectLineWidth) +
            QString(" solid rgb(255, 255, 0)");
    ui->frameRect->setStyleSheet(frameRectStyle);
    // 初始化8个点的鼠标样式
    ui->frameRect->setCursor(Qt::SizeAllCursor);
    ui->widgetLeftTopPoint->setCursor(Qt::SizeFDiagCursor);
    ui->widgetRightBottomPoint->setCursor(Qt::SizeFDiagCursor);
    ui->widgetLeftBottomPoint->setCursor(Qt::SizeBDiagCursor);
    ui->widgetRightTopPoint->setCursor(Qt::SizeBDiagCursor);
    ui->widgetLeftPoint->setCursor(Qt::SizeHorCursor);
    ui->widgetRightPoint->setCursor(Qt::SizeHorCursor);
    ui->widgetTopPoint->setCursor(Qt::SizeVerCursor);
    ui->widgetBottomPoint->setCursor(Qt::SizeVerCursor);
    QString pointStyle = "background: rgb(255, 255, 0)";
    // 初始化8个点的外观样式
    ui->widgetLeftTopPoint->setStyleSheet(pointStyle);
    ui->widgetRightBottomPoint->setStyleSheet(pointStyle);
    ui->widgetLeftBottomPoint->setStyleSheet(pointStyle);
    ui->widgetRightTopPoint->setStyleSheet(pointStyle);
    ui->widgetLeftPoint->setStyleSheet(pointStyle);
    ui->widgetRightPoint->setStyleSheet(pointStyle);
    ui->widgetTopPoint->setStyleSheet(pointStyle);
    ui->widgetBottomPoint->setStyleSheet(pointStyle);

    // 接收矩形区域内鼠标移动的位置
    connect(ui->frameRect, &FreeSnapRect::mouseMove, this, [=](QPoint p){
        refreshPreviewArea(p);
    });

    ui->frameRect->setGeometry(0, 0, 0, 0);
    setRectVisible(false);
    refreshPointPos();
    refreshGrayArea();
    update();
}

FreeSnapDialog::~FreeSnapDialog()
{
    scene->removeItem(grayItem);
    delete grayItem;
    resultPixmap = nullptr;
    delete ui;
}

void FreeSnapDialog::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    mousePressed = true;
    pressedPos = event->pos();
    lastPos = event->pos();

    // 判断在哪个区域按下的鼠标
    if (ui->widgetLeftTopPoint->geometry().contains(event->pos())) pressedInLeftTopArea = true;
    else if (ui->widgetLeftBottomPoint->geometry().contains(event->pos())) pressedInLeftBottomArea = true;
    else if (ui->widgetRightTopPoint->geometry().contains(event->pos())) pressedInRightTopArea = true;
    else if (ui->widgetRightBottomPoint->geometry().contains(event->pos())) pressedInRightBottomArea = true;
    else if (ui->widgetLeftPoint->geometry().contains(event->pos())) pressedInLeftArea = true;
    else if (ui->widgetRightPoint->geometry().contains(event->pos())) pressedInRightArea = true;
    else if (ui->widgetTopPoint->geometry().contains(event->pos())) pressedInTopArea = true;
    else if (ui->widgetBottomPoint->geometry().contains(event->pos())) pressedInBottomArea = true;
    else if (ui->frameRect->geometry().contains(event->pos())) pressedInRectArea = true;

}

void FreeSnapDialog::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (!hasRect) {
        hasRect = true;
    }

    mousePressed = false;
    pressedInRectArea = false;
    pressedInLeftTopArea = false;
    pressedInLeftBottomArea = false;
    pressedInRightTopArea = false;
    pressedInRightBottomArea = false;
    pressedInLeftArea = false;
    pressedInTopArea = false;
    pressedInRightArea = false;
    pressedInBottomArea = false;

    deltaHeight = deltaWidth = 0;
}

void FreeSnapDialog::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (mousePressed) {
        if (!hasRect) {
            // 首次绘制矩形框
            QPointF pos = event->pos();
            QRect r = QRect(std::min(pos.x(), pressedPos.x()),
                            std::min(pos.y(), pressedPos.y()),
                            std::abs(pos.x() - pressedPos.x()),
                            std::abs(pos.y() - pressedPos.y()));
            setRectVisible(true);
            ui->frameRect->setGeometry(r);
            refreshPointPos();
        } else {
            QRect frameRectGeo = ui->frameRect->geometry();
            // 左上点
            if (pressedInLeftTopArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() - lastPos.x() + event->position().x() + deltaWidth,
                                               frameRectGeo.y() - lastPos.y() + event->position().y() + deltaHeight,
                                               frameRectGeo.width() + lastPos.x() - event->position().x() - deltaWidth,
                                               frameRectGeo.height() + lastPos.y() - event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInLeftTopArea = false;
                    pressedInLeftBottomArea = true;
                    r.setHeight(0);
                }
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInLeftTopArea = false;
                    pressedInRightTopArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 左下点
            } else if (pressedInLeftBottomArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() - lastPos.x() + event->position().x() + deltaWidth,
                                               frameRectGeo.y() + deltaHeight,
                                               frameRectGeo.width() + lastPos.x() - event->position().x() - deltaWidth,
                                               frameRectGeo.height() - lastPos.y() + event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInLeftBottomArea = false;
                    pressedInLeftTopArea = true;
                    r.setHeight(0);
                }
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInLeftBottomArea = false;
                    pressedInRightBottomArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 右上点
            } else if (pressedInRightTopArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() + deltaWidth,
                                               frameRectGeo.y() - lastPos.y() + event->position().y() + deltaHeight,
                                               frameRectGeo.width() - lastPos.x() + event->position().x() - deltaWidth,
                                               frameRectGeo.height() + lastPos.y() - event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInRightTopArea = false;
                    pressedInRightBottomArea = true;
                    r.setHeight(0);
                }
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInRightTopArea = false;
                    pressedInLeftTopArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 右下点
            } else if (pressedInRightBottomArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() + deltaWidth,
                                               frameRectGeo.y() + deltaHeight,
                                               frameRectGeo.width() - lastPos.x() + event->position().x() - deltaWidth,
                                               frameRectGeo.height() - lastPos.y() + event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInRightBottomArea = false;
                    pressedInRightTopArea = true;
                    r.setHeight(0);
                }
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInRightBottomArea = false;
                    pressedInLeftBottomArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 上中点
            } else if (pressedInTopArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x(),
                                               frameRectGeo.y() - lastPos.y() + event->position().y() + deltaHeight,
                                               frameRectGeo.width(),
                                               frameRectGeo.height() + lastPos.y() - event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInTopArea = false;
                    pressedInBottomArea = true;
                    r.setHeight(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 下中点
            } else if (pressedInBottomArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x(),
                                               frameRectGeo.y() + deltaHeight,
                                               frameRectGeo.width(),
                                               frameRectGeo.height() - lastPos.y() + event->position().y() - deltaHeight));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.height() < 0) {
                    deltaHeight = r.height();
                    pressedInBottomArea = false;
                    pressedInTopArea = true;
                    r.setHeight(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 左中点
            } else if (pressedInLeftArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() - lastPos.x() + event->position().x() + deltaWidth,
                                               frameRectGeo.y(),
                                               frameRectGeo.width() + lastPos.x() - event->position().x() - deltaWidth,
                                               frameRectGeo.height()));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInLeftArea = false;
                    pressedInRightArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
                // 右中点
            } else if (pressedInRightArea) {
                QRect r = checkRectVaild(QRect(frameRectGeo.x() + deltaWidth,
                                               frameRectGeo.y(),
                                               frameRectGeo.width() - lastPos.x() + event->position().x() - deltaWidth,
                                               frameRectGeo.height()));
                deltaHeight = deltaWidth = 0;
                // 水平或竖直缩成一条线后自动切换到相对应的另一个点
                if (r.width() < 0) {
                    deltaWidth = r.width();
                    pressedInRightArea = false;
                    pressedInLeftArea = true;
                    r.setWidth(0);
                }
                ui->frameRect->setGeometry(r);
                refreshPointPos();
            } else if (pressedInRectArea) {
                int newX = frameRectGeo.x() - lastPos.x() + event->position().x();
                int newY = frameRectGeo.y() - lastPos.y() + event->position().y();
                ui->frameRect->move(qMin(rect().right() - frameRectGeo.width() + 1, qMax(newX, rect().left())),
                                    qMin(rect().bottom() - frameRectGeo.height() + 1, qMax(newY, rect().top())));
                refreshPointPos();
            }
        }
    }

    // 更新预览放大区域内容
    refreshPreviewArea(event->pos());

    lastPos = event->pos();
}

// 更新8个点的位置
void FreeSnapDialog::refreshPointPos()
{
    QRectF r = ui->frameRect->geometry();
    ui->widgetLeftTopPoint->setGeometry(r.x() - pointRadius / 2,
                                        r.y() - pointRadius / 2,
                                        pointRadius,
                                        pointRadius);
    ui->widgetLeftBottomPoint->setGeometry(r.x() - pointRadius / 2,
                                           r.y() + r.height() - pointRadius / 2,
                                           pointRadius,
                                           pointRadius);
    ui->widgetRightTopPoint->setGeometry(r.x() + r.width() - pointRadius / 2,
                                         r.y() - pointRadius / 2,
                                         pointRadius,
                                         pointRadius);
    ui->widgetRightBottomPoint->setGeometry(r.x() + r.width() - pointRadius / 2,
                                            r.y() + r.height() - pointRadius / 2,
                                            pointRadius,
                                            pointRadius);

    ui->widgetLeftPoint->setGeometry(r.x() - pointRadius / 2,
                                     (2 * r.y() + r.height()) / 2 - pointRadius / 2,
                                     pointRadius,
                                     pointRadius);
    ui->widgetRightPoint->setGeometry(r.x() + r.width() - pointRadius / 2,
                                      (2 * r.y() + r.height()) / 2 - pointRadius / 2,
                                      pointRadius,
                                      pointRadius);
    ui->widgetTopPoint->setGeometry((2 * r.x() + r.width()) / 2 - pointRadius / 2,
                                    r.y() - pointRadius / 2,
                                    pointRadius,
                                    pointRadius);
    ui->widgetBottomPoint->setGeometry((2 * r.x() + r.width()) / 2 - pointRadius / 2,
                                       r.y() + r.height() - pointRadius / 2,
                                       pointRadius,
                                       pointRadius);

    refreshGrayArea();
}

// 调整Rect的位置确保在屏幕内
QRect FreeSnapDialog::checkRectVaild(QRect r)
{
    QRect newRect = QRect(qMax(0, r.x()),
                          qMax(0, r.y()),
                          qMin(width() - r.x(), r.width()),
                          qMin(height() - r.y(), r.height()));
    newRect.setWidth(newRect.width() - (newRect.x() - r.x()));
    newRect.setHeight(newRect.height() - (newRect.y() - r.y()));
    return newRect;
}

// 设置选框和8个点的可见性
void FreeSnapDialog::setRectVisible(bool visible)
{
    ui->frameRect->setVisible(visible);
    ui->widgetLeftTopPoint->setVisible(visible);
    ui->widgetLeftBottomPoint->setVisible(visible);
    ui->widgetRightTopPoint->setVisible(visible);
    ui->widgetRightBottomPoint->setVisible(visible);
    ui->widgetLeftPoint->setVisible(visible);
    ui->widgetRightPoint->setVisible(visible);
    ui->widgetTopPoint->setVisible(visible);
    ui->widgetBottomPoint->setVisible(visible);
}

// 绘制灰色阴影
void FreeSnapDialog::refreshGrayArea()
{
    if (grayItem) {
        scene->removeItem(grayItem);
        delete grayItem;
    }
    QPainterPath grayPath = QPainterPath();
    grayPath.addRegion(QRegion(geometry()).subtracted(ui->frameRect->geometry()));
    grayItem = scene->addPolygon(grayPath.toFillPolygon(),
                                 QPen(QColorConstants::Transparent),
                                 QBrush(grayColor));
}

// 更新预览放大内容
void FreeSnapDialog::refreshPreviewArea(QPoint mousePos)
{
    // 更新预判Widget位置---------------------------
    CURSORINFO ci;
    ICONINFO iconInf;
    ci.cbSize = sizeof(CURSORINFO);
    GetCursorInfo(&ci);
    GetIconInfo(ci.hCursor, &iconInf);
    int infoContainerX = mousePos.x() + GetSystemMetrics(SM_CXCURSOR) - iconInf.xHotspot;
    int infoContainerY = mousePos.y() + GetSystemMetrics(SM_CYCURSOR) - iconInf.yHotspot;
    if (infoContainerX + ui->widgetInfoContainer->width() > width())
        infoContainerX = mousePos.x() - GetSystemMetrics(SM_CXCURSOR) + iconInf.xHotspot - ui->widgetInfoContainer->width();
    if (infoContainerY + ui->widgetInfoContainer->height() > height())
        infoContainerY = mousePos.y() - GetSystemMetrics(SM_CYCURSOR) + iconInf.yHotspot - ui->widgetInfoContainer->height();
    ui->widgetInfoContainer->setVisible(true);
    ui->widgetInfoContainer->move(infoContainerX, infoContainerY);
    ui->labelPosition->setText("(" + QString::number(mousePos.x() + 1) + "," + QString::number(mousePos.y() + 1) + ")");
    int sizeX = 0;
    int sizeY = 0;
    ui->labelSize->setText(QString::number(sizeX) + " x " + QString::number(sizeY));
    int r = 0, g = 0, b = 0;
    if (!pixelColor) pixelColor = new QColor();
    pixelColor->setRgb(image.pixelColor(mousePos.x(), mousePos.y()).rgb());
    pixelColor->getRgb(&r, &g, &b);
    ui->labelRGB->setText("RGB:(" + QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + ")");

    // 更新预览图片---------------------------
    float scaledPreviewWidth = ui->labelPreview->width() / previewZoomRate;
    float scaledPreviewHeight = ui->labelPreview->height() / previewZoomRate;
    // 注意要向上取整ceil两次
    int x = ceil(mousePos.x() - scaledPreviewWidth / 2);
    int y = ceil(mousePos.y() - scaledPreviewHeight / 2);
    QPixmap processedPixmap(scaledPreviewWidth, scaledPreviewHeight);
    QPainter painter(&processedPixmap);
    painter.setViewport(0, 0, scaledPreviewWidth, scaledPreviewHeight);
    // 开始绘制noScaledPreviewPixmap的位置
    int drawX = qMax(-x, 0);
    int drawY = qMax(-y, 0);
    // 绘制黑色填充背景
    painter.fillRect(0, 0, scaledPreviewWidth, scaledPreviewHeight, Qt::black);
    // 绘制noScaledPreviewPixmap
    painter.drawPixmap(drawX, drawY, picture.copy(x, y, scaledPreviewWidth, scaledPreviewHeight));
    painter.setPen(QColor(7, 200, 250));
    painter.drawLine(0, processedPixmap.height() / 2, processedPixmap.width(), processedPixmap.height() / 2);
    painter.drawLine(processedPixmap.width() / 2, 0, processedPixmap.width() / 2, processedPixmap.height());
    ui->labelPreview->setPixmap(processedPixmap.scaled(ui->labelPreview->width(), ui->labelPreview->height(), Qt::KeepAspectRatio));
}

// 键盘事件
void FreeSnapDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        *this->resultPixmap = picture.copy(ui->frameRect->geometry());
        captured = true;
        close();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        captured = false;
        close();
        return;
    } else if (event->key() == Qt::Key_C) {         // C键复制颜色
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(QString::number(pixelColor->rgb(), 16));
        return;
    }
}
