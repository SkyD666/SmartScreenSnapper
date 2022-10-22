#include "longsnapdialog.h"
#include "ui_longsnapdialog.h"
#include <windows.h>
#include <QCloseEvent>
#include <QtMath>
#include <QDebug>
#include "publicdata.h"
#include "screenshothelper.h"

LongSnapDialog::LongSnapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LongSnapDialog),
    firstPoint(nullptr),
    secondPoint(nullptr),
    clickPoint(nullptr),
    region(nullptr),
    currentStep(0)
{
    ui->setupUi(this);
    // 可在不点击鼠标的情况下捕获移动事件
    setMouseTracking(true);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowState(Qt::WindowActive);
    // 多显示器支持
    move(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN));
    resize(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));

    ui->widgetRect->setAttribute(Qt::WA_TransparentForMouseEvents, true);      // 不接收鼠标事件
    ui->widgetRect->setGeometry(0, 0, 0, 0);
    ui->widgetRect->setVisible(false);
    QPixmap pixmap = getFullScreen();
    QPalette palette(this->palette());
    palette.setBrush(QPalette::Window, QBrush(pixmap));
    setPalette(palette);
}

LongSnapDialog::~LongSnapDialog()
{
    delete ui;
    delete firstPoint;
    delete secondPoint;
    delete clickPoint;
    delete region;
}

QPixmap LongSnapDialog::getFullScreen()
{
    return ScreenShotHelper::getWindowPixmap(
                (HWND)QGuiApplication::primaryScreen()->handle(),
                ScreenShotHelper::ScreenShot,
                PublicData::includeCursor
                );
}

void LongSnapDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
}

void LongSnapDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentStep == 0) {
        firstPoint = new QPoint(event->pos());
        ui->widgetRect->setVisible(true);
        currentStep++;
    } else if (currentStep == 1) {
        secondPoint = new QPoint(event->pos());
        ui->widgetRect->setVisible(false);
        currentStep++;
    } else if (currentStep == 2) {
        clickPoint = new QPoint(event->pos());
        region = new QRect(qMin(firstPoint->x(), secondPoint->x()),
                           qMin(firstPoint->y(), secondPoint->y()),
                           qAbs(firstPoint->x() - secondPoint->x()),
                           qAbs(firstPoint->y() - secondPoint->y()));
        currentStep++;
    }
}

void LongSnapDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (currentStep == 1) {
        refreshRectPos(*firstPoint, event->pos());
    }
}

void LongSnapDialog::refreshRectPos(QPoint p1, QPoint p2)
{
    ui->widgetRect->setGeometry(qMin(p1.x(), p2.x()),
                                qMin(p1.y(), p2.y()),
                                qAbs(p1.x() - p2.x()),
                                qAbs(p1.y() - p2.y()));
}

QPixmap LongSnapDialog::capture()
{
    hide();
    QImage lastRegion;
    QImage currentRegion;
    QImage finalPixmap;
    do {
        currentRegion = ScreenShotHelper::getWindowPixmap(
                    (HWND)QGuiApplication::primaryScreen()->handle(),
                    ScreenShotHelper::ScreenShot,
                    PublicData::includeCursor,
                    region->x(),
                    region->y(),
                    region->width(),
                    region->height()).toImage();
    } while (currentRegion == lastRegion);
}
