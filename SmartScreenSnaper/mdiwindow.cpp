#include <QMessageBox>
#include "mdiwindow.h"
#include "mainwindow.h"

MdiWindow::MdiWindow(QWidget *parent, Qt::WindowFlags flags):QMdiSubWindow(parent, flags)
{
    this->saved = true;
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
}

bool MdiWindow::isSaved() {
    return this->saved;
}

void MdiWindow::setSaved(bool saved) {
    this->saved = saved;
}

void MdiWindow::closeEvent(QCloseEvent *event) {
//    if (!this->saved || (!this->saved && MainWindow::noToAllClicked)) {
    if (!this->saved && (!this->saved && !MainWindow::noToAllClicked)) {
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
    } else {
        emit close();
    }
}
