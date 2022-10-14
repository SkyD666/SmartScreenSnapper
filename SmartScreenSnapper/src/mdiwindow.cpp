#include <QGraphicsScene>
#include <QMessageBox>
#include <QSlider>
#include "mdiwindow.h"
#include "mainwindow.h"
#include "graphicsview.h"

MdiWindow::MdiWindow(QWidget *parent, Qt::WindowFlags flags):QMdiSubWindow(parent, flags)
{
    this->parent = parent;
    this->saved = true;
    this->xScale = 1;
    this->yScale = 1;

    setObjectName("mdiWindow");
    setStyleSheet("#mdiWindow { background: url(:/image/Background1.svg); }");

    GraphicsView * graphicsView = new GraphicsView(this);
    QGraphicsScene* graphicsScene = new QGraphicsScene(graphicsView);
    graphicsView->setObjectName("graphicsView");
    graphicsView->setScene(graphicsScene);
    graphicsView->setStyleSheet("#graphicsView { background: transparent; }");
    setWidget(graphicsView);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(graphicsView, &GraphicsView::zoom, [=](int n){
        emit zoom(n);
    });
}

void MdiWindow::setListItemName(QString name) {
    this->listItem.setText(name);
}

QListWidgetItem & MdiWindow::getListItem() {
    return this->listItem;
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
    if (MainWindow::closeAllNotSave || MainWindow::noToAllClicked) {
        emit close();
        return;
    }

    if (this->saved) {
        emit close();
    } else {
        int messageBoxResult = QMessageBox::Cancel;
        if (MainWindow::exitApp) {
            messageBoxResult = QMessageBox::question(NULL, tr("退出"), tr("是否保存截图？"), QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel, QMessageBox::Yes);
        } else {
            messageBoxResult = QMessageBox::question(NULL, tr("退出"), tr("是否保存截图？"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        }

        switch (messageBoxResult) {
        case QMessageBox::Yes:
            emit save();
            emit close();
            break;
        case QMessageBox::No:
            emit close();
            break;
        case QMessageBox::NoToAll:
            MainWindow::noToAllClicked = true;
            emit close();
            break;
        case QMessageBox::Cancel:
            MainWindow::exitCancel();
            event->ignore();
            break;
        }
    }
}

void MdiWindow::setXScale(double xScale)
{
    this->xScale = xScale;
}

double MdiWindow::getXScale()
{
    return this->xScale;
}

void MdiWindow::setYScale(double yScale)
{
    this->yScale = yScale;
}

double MdiWindow::getYScale()
{
    return this->yScale;
}
