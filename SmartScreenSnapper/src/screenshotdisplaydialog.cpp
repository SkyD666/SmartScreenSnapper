#include "screenshotdisplaydialog.h"
#include "ui_screenshotdisplaydialog.h"

#include <QMouseEvent>

ScreenshotDisplayDialog::ScreenshotDisplayDialog(QPixmap pixmap) :
    QDialog(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
    ui(new Ui::ScreenshotDisplayDialog),
    pixmap(pixmap)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    setCursor(Qt::OpenHandCursor);
    resize(pixmap.width(), pixmap.height());
    ui->labPixmap->setPixmap(pixmap);
}

ScreenshotDisplayDialog::~ScreenshotDisplayDialog()
{
    delete ui;
}

void ScreenshotDisplayDialog::mousePressEvent(QMouseEvent *event)
{
    mousePressedPos = event->pos();
    setCursor(Qt::ClosedHandCursor);
    QDialog::mousePressEvent(event);
}

void ScreenshotDisplayDialog::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    QDialog::mouseReleaseEvent(event);
}

void ScreenshotDisplayDialog::mouseMoveEvent(QMouseEvent *event)
{
    move((event->globalPosition() - mousePressedPos).toPoint());
    QDialog::mouseMoveEvent(event);
}
