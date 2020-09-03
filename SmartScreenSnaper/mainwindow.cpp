#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <winuser.h>
#include <QWidget>
#include <QGraphicsView>
#include <QDesktopWidget>
#include <QDateTime>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QtWinExtras/qwinfunctions.h>
#include "hicontoqpixmap.h"
#include "publicdata.h"
#include "mdiwindow.h"
#include "aboutdialog.h"
#include "settingdialog.h"

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

bool MainWindow::exitApp = false;
bool MainWindow::noToAllClicked = false;

void wait(int msec);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusBarWidgets[0] = NULL;
    statusBarWidgets[1] = NULL;

    initSystemTray();

    initStatusBar();        //这句里面有控件初始化，在设置值之前进行

    PublicData::readSettings();

    PublicData::registerAllHotKey(this);

    setSettings();

    connect(ui->listDocument, &QListWidget::itemClicked,
            [=](){
        if(ui->listDocument->count() > 0) ui->mdiArea->subWindowList().at(ui->listDocument->currentRow())->setFocus();
    });

    connect(ui->listDocument, &QListWidget::currentRowChanged,
            [=](int currentRow){
        if(!MainWindow::exitApp)
            currentRow == -1 ? ui->actionSave->setEnabled(false) : ui->actionSave->setEnabled(true);
    });

    connect(ui->actionDocumentListDock, &QAction::toggled,
            [=](bool checked){ ui->dockDocumentList->setVisible(checked); });

    connect(ui->actionactionToolBar, &QAction::toggled,
            [=](bool checked){ui->toolBar->setVisible(checked);});

    connect(ui->dockDocumentList, &QDockWidget::visibilityChanged,
            [=](bool visible){ if(this->windowState()!=Qt::WindowState::WindowMinimized && this->isVisible()) ui->actionDocumentListDock->setChecked(visible);});

    connect(ui->toolBar, &QToolBar::visibilityChanged,
            [=](bool visible){ ui->actionactionToolBar->setChecked(visible);});

    connect(ui->mdiArea, &QMdiArea::subWindowActivated,
            [=](QMdiSubWindow *window){
        if(ui->listDocument->count() > 1 && window != NULL){
            ui->listDocument->setCurrentItem(&((MdiWindow*)window)->getListItem());

            ((QLabel*)(statusBarWidgets[1]))->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第") +
                                                      QString::number(ui->listDocument->row(&((MdiWindow*)window)->getListItem()) + 1) + tr("张"));
        } else {
            ((QLabel*)(statusBarWidgets[1]))->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第0张"));
        }

    });

    ui->actionSave->setEnabled(false);
}

MainWindow::~MainWindow()
{
    PublicData::writeSettings();

    PublicData::unregisterAllHotKey();

    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (PublicData::clickCloseToTray && !PublicData::ignoreClickCloseToTray) {
        setVisible(false);
        event->ignore();
        return;
    }

    exitApp = true;
    int subWindowCount = ui->mdiArea->subWindowList().count();
    while(subWindowCount-->0){
        ui->listDocument->setCurrentRow(subWindowCount);
        if(ui->mdiArea->subWindowList().last()->close()) {
            ui->mdiArea->removeSubWindow(ui->mdiArea->subWindowList().last());
        }
        if (!exitApp) {
            PublicData::ignoreClickCloseToTray = false;
            event->ignore();
            return;
        }
    }
    //    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actionNew_triggered()
{
    createMDIWindow();
}

int MainWindow::createMDIWindow() {
    //QPixmap pixmap(500, 500);
    //QPainter painter(&pixmap);              //创建一个画笔
    //painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::white);
    MdiWindow *child = new MdiWindow(this);
    QGraphicsScene* graphicsScene = new QGraphicsScene(child);
    //graphicsScene->addPixmap(pixmap);
    QGraphicsView * graphicsView = new QGraphicsView(child);
    graphicsView->setScene(graphicsScene);
    graphicsView->setBackgroundBrush(QBrush(MDIWINBKCOLOR));
    child->setWidget(graphicsView);
    child->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(child);
    //    child->resize(600, 400);
    child->show();

    ui->listDocument->addItem(&(child->getListItem()));
    ui->listDocument->setCurrentItem(&child->getListItem());

    ((QLabel*)(statusBarWidgets[1]))->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第") +
                                              QString::number(ui->listDocument->row(&child->getListItem()) + 1) + tr("张"));

    QString name = tr("文档") + " " + QString::number(++PublicData::totalWindowCount);
    child->setListItemName(name);
    child->setName(name);

    //    connect(graphicsScene, &QGraphicsScene::changed, [=](){
    //        child->setSaved(false);
    //    });

    connect(child, &MdiWindow::save, this, &MainWindow::on_actionSave_triggered);

    connect(child, &MdiWindow::close, [=](){
        if(ui->mdiArea->subWindowList().size() > 1)
            ui->listDocument->setCurrentRow(ui->listDocument->count() - 2);
    });

    PublicData::activeWindowIndex = ui->mdiArea->subWindowList().size() - 1;
    return PublicData::activeWindowIndex;
}

void MainWindow::on_actionSave_triggered()
{
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存"),
                                                    activeWindow->getName(), tr("PNG文件(*.png);;JPG文件(*.jpg);;BMP文件(*.bmp)"));
    savePicture(filePath);
}

void MainWindow::savePicture(QString filePath)
{
    if (filePath != "") {
        QDir dir(filePath.left(filePath.lastIndexOf('/') + 1));
        if(!dir.exists()) dir.mkdir(dir.path());
        MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
        QGraphicsScene* graphicsScene = ((QGraphicsView*)(activeWindow->widget()))->scene();
        QImage image(QSize(graphicsScene->width(), graphicsScene->height()),QImage::Format_ARGB32);
        QPainter painter(&image);
        graphicsScene->render(&painter);
        image.save(filePath);
        activeWindow->setSaved(true);
    }
}

void MainWindow::on_actionExit_triggered()
{
    PublicData::ignoreClickCloseToTray = true;
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::initStatusBar()
{
    statusBarWidgets[0] = new QCheckBox(this);
    statusBarWidgets[1] = new QLabel(this);

    ((QCheckBox*)statusBarWidgets[0])->setText(tr("播放提示音"));

    connect((QCheckBox*)statusBarWidgets[0], &QCheckBox::stateChanged,
            [=](int state){ PublicData::isPlaySound = (state == Qt::Checked); });

    ((QLabel*)statusBarWidgets[1])->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第") +
                                            QString::number(ui->listDocument->currentRow() + 1) + tr("张"));

    ui->statusbar->addPermanentWidget((QCheckBox*)statusBarWidgets[0]);
    ui->statusbar->addPermanentWidget((QLabel*)statusBarWidgets[1]);
}

void MainWindow::commonSnapAction(int index, bool isHotKey)
{
    int windowIndex = 0;
    QString name;
    switch (index) {
    case ScreenSnap: {
        if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[ScreenSnap].waitTime * 1000);
        windowIndex = createMDIWindow();
        name = tr("全屏截图") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
        ((QGraphicsView*)(ui->mdiArea->activeSubWindow()->widget()))->scene()->addPixmap(grabWindow((HWND)QApplication::desktop()->winId(), PublicData::includeCursor));
        break;
    }
    case ActiveWindowSnap: {
        int messageBoxResult = QMessageBox::Yes;
        if (!isHotKey || !PublicData::hotKeyNoWait) {
            messageBoxResult = QMessageBox::information(this, tr("活动窗口截图"), tr("点击确定") + QString::number(PublicData::snapType[ActiveWindowSnap].waitTime) +
                                                        tr("秒后截图"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
        }
        if (messageBoxResult == QMessageBox::Yes) {
            if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[ActiveWindowSnap].waitTime * 1000);
            windowIndex = createMDIWindow();
            name = tr("活动窗口截图") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
//            int windowFrameSizeX = GetSystemMetrics(SM_CXSIZEFRAME);
//            int windowFrameSizeY = GetSystemMetrics(SM_CYSIZEFRAME);
//            int windowCaptionSize = GetSystemMetrics(SM_CYCAPTION);
            HWND activeWindowHwnd = GetForegroundWindow();
            RECT activeWindowRect;
            GetWindowRect(activeWindowHwnd, &activeWindowRect);
            ((QGraphicsView*)(ui->mdiArea->activeSubWindow()->widget()))->scene()->addPixmap(grabWindow(activeWindowHwnd,
                                                                                                        PublicData::includeCursor,
                                                                                                        0, 0,
                                                                                                        activeWindowRect.right - activeWindowRect.left,
                                                                                                        activeWindowRect.bottom - activeWindowRect.top));
        } else {
            return;
        }
        break;
    }
    case CursorSnap: {
        if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[CursorSnap].waitTime * 1000);
        windowIndex = createMDIWindow();
        name = tr("截取光标") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
        ((QGraphicsView*)(ui->mdiArea->activeSubWindow()->widget()))->scene()->addPixmap(
                    grabWindow((HWND)ui->mdiArea->activeSubWindow()->widget()->winId(), true
                               , 0, 0, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR),
                               true));
        break;
    }
    }

    ui->mdiArea->activeSubWindow()->setWindowTitle(name);
    ((MdiWindow *)ui->mdiArea->subWindowList().at(windowIndex))->setName(name);
    ((MdiWindow*)(ui->mdiArea->activeSubWindow()))->setListItemName(name);
    ((MdiWindow*)(ui->mdiArea->activeSubWindow()))->setSaved(false);

    if(PublicData::isPlaySound) PlaySound(TEXT("DAZIJI"), NULL, SND_RESOURCE | SND_ASYNC);

    if (PublicData::snapType[ScreenSnap].isAutoSave && PublicData::snapType[index].autoSavePath != "") {
        MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
        savePicture(PublicData::snapType[index].autoSavePath + "/" + activeWindow->getName() + ".png");
    }
}


QPixmap MainWindow::grabWindow(HWND winId, bool includeCursor, int x, int y, int w , int h, bool isCursorSnap)
{
    RECT r;
    GetClientRect(winId, &r);

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    HDC display_dc = GetWindowDC(winId);
    HDC bitmap_dc = CreateCompatibleDC(display_dc);
    HBITMAP bitmap = CreateCompatibleBitmap(display_dc, w, h);
    HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

    BitBlt(bitmap_dc, 0, 0, w, h, display_dc, x, y, SRCCOPY | CAPTUREBLT);

    if(includeCursor){
        CURSORINFO ci;
        ICONINFO iconInf;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        RECT winRect;
        GetWindowRect(winId, &winRect);
        GetIconInfo(ci.hCursor, &iconInf);
        if(isCursorSnap){
            RECT rect = {0, 0, w, h};
            FillRect(bitmap_dc, &rect, CreateSolidBrush(RGB(255, 255, 255)));
            DrawIcon(bitmap_dc, 0, 0, ci.hCursor);
        } else {

                DrawIcon(bitmap_dc, ci.ptScreenPos.x - x - winRect.left - iconInf.xHotspot, ci.ptScreenPos.y - y - winRect.top - iconInf.yHotspot, ci.hCursor);
        }
    }

    // clean up all but bitmap
    ReleaseDC(winId, display_dc);
    SelectObject(bitmap_dc, null_bitmap);
    DeleteDC(bitmap_dc);

    QPixmap pixmap = QtWin::fromHBITMAP(bitmap);

    DeleteObject(bitmap);

    return pixmap;
}

void MainWindow::on_actionScreenSnap_triggered()
{
    commonSnapAction(ScreenSnap, false);
}

void MainWindow::on_actionActiveWindowSnap_triggered()
{
    commonSnapAction(ActiveWindowSnap, false);
}

void MainWindow::exitCancel()
{
    exitApp = false;
}

void MainWindow::initSystemTray()
{
    systemTrayMenuActions[0].setText(tr("显示界面"));
    connect(&systemTrayMenuActions[0], &QAction::triggered, [=](){
        this->show();
    });
    systemTrayMenuActions[1].setText(tr("退出"));
    connect(&systemTrayMenuActions[1], &QAction::triggered, [=](){
        PublicData::ignoreClickCloseToTray = true;
        close();
    });

    systemTrayMenu.addAction(&systemTrayMenuActions[0]);
    systemTrayMenu.addSeparator();
    systemTrayMenu.addAction(&systemTrayMenuActions[1]);

    systemTray.setIcon(QIcon(":/images/icon.png"));
    systemTray.setToolTip(QApplication::applicationName());
    systemTray.setContextMenu(&systemTrayMenu);

    connect(&systemTray, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason){
        switch (reason)
        {
        case QSystemTrayIcon::Context:
            systemTrayMenu.show();
            break;
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::MiddleClick:
            this->show();
            break;
        case QSystemTrayIcon::Unknown:
            break;
        }
    });

    systemTray.show();
}

void MainWindow::setSettings()
{
    if (statusBarWidgets[0] != NULL) {
        ((QCheckBox*)statusBarWidgets[0])->setChecked(PublicData::isPlaySound);
    }
}

void wait(int msec)
{
    QEventLoop loop;            //定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();                    //事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void MainWindow::on_actionSetting_triggered()
{
    SettingDialog settingDialog(this);
    PublicData::unregisterAllHotKey();      //进入设置页面前取消热键
    settingDialog.exec();
    if (statusBarWidgets[0] != NULL) {
        ((QCheckBox*)statusBarWidgets[0])->setChecked(PublicData::isPlaySound);
    }
    PublicData::registerAllHotKey(this);
}

void MainWindow::hotKeyPressed(int i)
{
    switch (i) {
    case ScreenSnap: {
        commonSnapAction(ScreenSnap, true);
        break;
    }
    case ActiveWindowSnap: {
        commonSnapAction(ActiveWindowSnap, true);
        break;
    }
    case CursorSnap: {
        commonSnapAction(CursorSnap, true);
        break;
    }
    }
}

void* MainWindow::getStatusBarItem(int index)
{
    return statusBarWidgets[index];
}

void MainWindow::on_actionCursorSnap_triggered()
{
    commonSnapAction(CursorSnap, false);
}
