#include "freehandsnapdialog.h"
#include "freehandsnap/freehandsnapgraphicsscene.h"
#include "ui_freehandsnapdialog.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

FreeHandSnapDialog::FreeHandSnapDialog(QWidget* parent)
    : BaseFullScreenSnapDialog(parent)
    , ui(new Ui::FreeHandSnapDialog)
{
    ui->setupUi(this);

    doAfterSetupUi();

    // 可在不点击鼠标的情况下捕获移动事件
    ui->graphicsView->setMouseTracking(true);

    scene = new FreeHandSnapGraphicsScene(this);
    scene->setSceneRect(QRectF(0, 0, width(), height()));
    scene->setBackgroundBrush(QBrush(fullScreenPixmap));
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setGeometry(0, 0, width(), height());
}

FreeHandSnapDialog::~FreeHandSnapDialog()
{
    delete ui;
}

void FreeHandSnapDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QRectF r = scene->getPathRect();
        // 先用QImage设置透明度，直接用QPixmap是黑色背景，设置透明无效
        QImage image(r.width(), r.height(), QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&image);
        image.fill(Qt::transparent);
        auto newFullScreenPixmap = fullScreenPixmap.copy();
        newFullScreenPixmap.setDevicePixelRatio(1);
        painter.setBrush(QBrush(newFullScreenPixmap));
        painter.setPen(QPen(Qt::transparent));
        painter.translate(-r.x(), -r.y());
        painter.drawConvexPolygon(scene->getPathPolygonF());
        emit captured(QPixmap::fromImage(image));
        close();
        return;
    }
}
