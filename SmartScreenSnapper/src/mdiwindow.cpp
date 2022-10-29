#include "mdiwindow.h"
#include "ui_mdiwindowwidget.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QSlider>
#include <QVariant>
#include "mainwindow.h"
#include "graphicsview.h"
#include "publicdata.h"

MdiWindow::MdiWindow(QWidget *parent, Qt::WindowFlags flags) :
    QMdiSubWindow(parent, flags),
    ui(new Ui::MdiWindowWidget),
    listItem(),
    saved(true),
    imageScale(1)
{
    containerWidget = new QWidget(this);
    ui->setupUi(containerWidget);

    QGraphicsScene* graphicsScene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(graphicsScene);
    setWidget(containerWidget);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->graphicsView, &GraphicsView::zoom, this, [=](int n){
        emit zoom(n);
    });

    listItem.setData(MdiWindowRole, QVariant::fromValue(this));
}

MdiWindow::~MdiWindow()
{
    delete ui;
}

void MdiWindow::setPixmap(QPixmap pixmap)
{
    setSaved(false);
    ui->graphicsView->scene()->addPixmap(pixmap);
}

void MdiWindow::setListItemName(QString name) {
    listItem.setText(name);
}

QListWidgetItem & MdiWindow::getListItem() {
    return listItem;
}

QString MdiWindow::getName() {
    return this->name;
}

void MdiWindow::setName(QString name) {
    this->name = name;
    if (saved) {
        setWindowTitle(name);
        setListItemName(name);
    } else {
        setWindowTitle(name + "*");
        setListItemName(name + "*");
    }
}

bool MdiWindow::isSaved() {
    return this->saved;
}

void MdiWindow::setSaved(bool saved) {
    this->saved = saved;
    if (saved) {
        setWindowTitle(name);
        setListItemName(name);
    } else {
        setWindowTitle(name + "*");
        setListItemName(name + "*");
    }
}

void MdiWindow::closeEvent(QCloseEvent *event) {
    if (!saved && !MainWindow::closeAllNotSave && !MainWindow::noToAllClicked) {
        QMessageBox::StandardButton messageBoxResult = QMessageBox::StandardButton(QMessageBox::Cancel);
        messageBoxResult =
                QMessageBox::question(nullptr,
                                      tr("退出"),
                                      tr("是否保存截图？"),
                                      QMessageBox::StandardButtons(QMessageBox::Yes |
                                                                   QMessageBox::No |
                                                                   (MainWindow::exitApp ?
                                                                        QMessageBox::NoToAll : 0) |
                                                                   QMessageBox::Cancel),
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
    QPixmap pixmap(graphicsScene->width(), graphicsScene->height());
    pixmap.fill(Qt::transparent);
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
