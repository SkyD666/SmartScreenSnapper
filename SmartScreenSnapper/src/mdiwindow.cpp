#include "mdiwindow.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "publicdata.h"
#include "screenshotdisplaydialog.h"
#include "src/graphicsscene.h"
#include "ui_mdiwindowwidget.h"
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QSlider>
#include <QVariant>
#include <QWindow>
#include <src/graphicsitem/graphicspixmapitem.h>
#include <src/undo/undomove.h>

MdiWindow::MdiWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMdiSubWindow(parent, flags)
    , ui(new Ui::MdiWindowWidget)
    , listItem()
    , saved(true)
    , imageScale(1)
    , contextMenu(new QMenu(this))
    , undoStack(new QUndoStack(this))
{
    containerWidget = new QWidget(this);
    ui->setupUi(containerWidget);

    connect(this, &QMdiSubWindow::aboutToActivate, this, [=]() {
        undoStack->setActive();
    });

    initActions();

    GraphicsScene* graphicsScene = new GraphicsScene(ui->graphicsView);
    connect(graphicsScene, &GraphicsScene::itemMoved, this, [=](QGraphicsItem* movedItem, const QPointF& movedFromPosition) {
        undoStack->push(new UndoMove(movedItem, movedFromPosition));
    });
    ui->graphicsView->setScene(graphicsScene);
    setWidget(containerWidget);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->graphicsView, &GraphicsView::zoom, this, [=](int n) {
        emit zoom(n);
    });

    listItem.setData(MdiWindowRole, QVariant::fromValue(this));

    connect(screen(), &QScreen::physicalDotsPerInchChanged, this, [this](qreal) {
        for (auto& item : ui->graphicsView->scene()->items()) {
            auto pixmapItem = ((GraphicsPixmapItem*)item);
            auto newPixmap = pixmapItem->pixmap();
            newPixmap.setDevicePixelRatio(screen()->devicePixelRatio());
            pixmapItem->setPixmap(newPixmap);
        }
    });
}

MdiWindow::~MdiWindow()
{
    delete ui;
}

void MdiWindow::setLayers(QList<QPair<QPixmap, QPoint>> layers)
{
    setSaved(false);
    QGraphicsPixmapItem* item = nullptr;
    for (auto& layer : layers) {
        layer.first.setDevicePixelRatio(devicePixelRatio());
        item = new GraphicsPixmapItem(layer.first, nullptr);
        ui->graphicsView->scene()->addItem(item);
        item->setPos(layer.second);
    }
}

void MdiWindow::setListItemName(QString name)
{
    listItem.setText(name);
}

QListWidgetItem& MdiWindow::getListItem()
{
    return listItem;
}

QString MdiWindow::getName()
{
    return name;
}

void MdiWindow::setName(QString name)
{
    this->name = name;
    ui->graphicsView->setFileName(name);
    if (saved) {
        setWindowTitle(name);
        setListItemName(name);
    } else {
        setWindowTitle(name + "*");
        setListItemName(name + "*");
    }
}

bool MdiWindow::isSaved()
{
    return saved;
}

void MdiWindow::setSaved(bool saved)
{
    this->saved = saved;
    if (saved) {
        setWindowTitle(name);
        setListItemName(name);
    } else {
        setWindowTitle(name + "*");
        setListItemName(name + "*");
    }
}

ScreenShotHelper::ShotType MdiWindow::getShotType() const
{
    return shotType;
}

void MdiWindow::setShotType(ScreenShotHelper::ShotType newShotType)
{
    shotType = newShotType;
}

void MdiWindow::initActions()
{
    QAction* screenshotDisplay = new QAction(tr("到顶层展示..."), this);
    screenshotDisplay->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    screenshotDisplay->setShortcutContext(Qt::WindowShortcut);

    addAction(screenshotDisplay);
    contextMenu->addAction(screenshotDisplay);

    connect(screenshotDisplay, &QAction::triggered, this, [=]() {
        ScreenshotDisplayDialog* dialog = new ScreenshotDisplayDialog(getPixmap());
        dialog->show();
    });
}

QUndoStack* MdiWindow::getUndoStack() const
{
    return undoStack;
}

void MdiWindow::closeEvent(QCloseEvent* event)
{
    if (!saved && !MainWindow::closeAllNotSave && !MainWindow::noToAllClicked) {
        QMessageBox::StandardButton messageBoxResult = QMessageBox::StandardButton(QMessageBox::Cancel);
        messageBoxResult = QMessageBox::question(nullptr,
            tr("退出"),
            tr("是否保存截图？"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | (MainWindow::exitApp ? QMessageBox::NoToAll : 0) | QMessageBox::Cancel),
            QMessageBox::StandardButton(QMessageBox::Yes));

        switch (messageBoxResult) {
        case QMessageBox::Yes:
            if (saveByDialog()) {
                break;
            }
            // 若没保存成功，则取消关闭窗口（执行case QMessageBox::Cancel:）
        case QMessageBox::Cancel:
            MainWindow::exitCancel();
            event->ignore();
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::NoToAll:
            MainWindow::noToAllClicked = true;
            break;
        default:;
        }
    }
    if (event->isAccepted()) {
        emit onClose();
    }
}

void MdiWindow::contextMenuEvent(QContextMenuEvent* event)
{
    contextMenu->popup(mapToGlobal(event->pos()));
}

void MdiWindow::setScale(double scale)
{
    ui->graphicsView->scale(scale / imageScale, scale / imageScale);
    imageScale = scale;
}

double MdiWindow::getScale()
{
    return imageScale;
}

QPixmap MdiWindow::getPixmap()
{
    QGraphicsScene* graphicsScene = ui->graphicsView->scene();
    QPixmap pixmap(graphicsScene->width() * devicePixelRatio(),
        graphicsScene->height() * devicePixelRatio());
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(1);
    QPainter painter(&pixmap);
    graphicsScene->render(&painter);
    return pixmap;
}

bool MdiWindow::saveByDialog()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存"),
        getName(),
        PublicData::getSaveExtFilter());
    return saveByPath(filePath);
}

bool MdiWindow::saveByPath(QString filePath)
{
    if (!filePath.isEmpty()) {
        if (ScreenShotHelper::savePicture(this, filePath, getPixmap())) {
            setSaved(true);
            return true;
        } else {
            return false;
        }
    }
    return false;
}
