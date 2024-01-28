#include "snapfrompointdialog.h"
#include "ui_snapfrompointdialog.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <windows.h>

SnapByPointDialog::SnapByPointDialog(QWidget* parent)
    : BaseFullScreenSnapDialog(parent)
    , ui(new Ui::SnapFromPointDialog)
{
    ui->setupUi(this);

    doAfterSetupUi();

    // 多显示器支持
    ui->graphicsView->setMouseTracking(true);

    scene = new SnapByPointGraphicsScene(this);
    scene->setSceneRect(QRectF(0, 0, width(), height()));
    scene->setBackgroundBrush(QBrush(fullScreenPixmap));
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setGeometry(0, 0, width(), height());

    connect(scene, &SnapByPointGraphicsScene::captured, this, [=](QRectF r) {
        emit captured(QPixmap(fullScreenPixmap.copy(r.toRect())));
        close();
    });
    connect(scene, &SnapByPointGraphicsScene::rightButtonClicked, this, [=]() {
        close();
    });
}

SnapByPointDialog::~SnapByPointDialog()
{
    delete ui;
}
