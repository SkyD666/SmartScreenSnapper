#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <winuser.h>
#include <QWidget>
#include <QClipboard>
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
#include <QToolButton>
#include <QtWinExtras/qwinfunctions.h>
#include <QPropertyAnimation>
#include "graphicsview.h"
#include "windowsinfo.h"
#include "publicdata.h"
#include "mdiwindow.h"
#include "aboutdialog.h"
#include "settingdialog.h"
#include "freesnapdialog.h"

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

bool MainWindow::exitApp = false;
bool MainWindow::noToAllClicked = false;
bool MainWindow::closeAllNotSave = false;

void wait(int msec);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusBarWidgets[0] = NULL;
    statusBarWidgets[1] = NULL;
    statusBarWidgets[2] = NULL;
    statusBarWidgets[3] = NULL;
    statusBarWidgets[4] = NULL;

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
        if (!MainWindow::exitApp) {
            if (currentRow == -1) {
                ui->actionSave->setEnabled(false);
                ui->actionCloseAllNotSave->setEnabled(false);
            } else {
                ui->actionSave->setEnabled(true);
                ui->actionCloseAllNotSave->setEnabled(true);
            }
        }
    });

//    connect(ui->actionDocumentListDock, &QAction::toggled,
//            [=](bool checked){ ui->dockDocumentList->setVisible(checked); });

//    connect(ui->actionactionToolBar, &QAction::toggled,
//            [=](bool checked){ui->toolBar->setVisible(checked);});

//    connect(ui->dockDocumentList, &QDockWidget::visibilityChanged,
//            [=](bool visible){ if(this->windowState()!=Qt::WindowState::WindowMinimized && this->isVisible()) ui->actionDocumentListDock->setChecked(visible);});

//    connect(ui->toolBar, &QToolBar::visibilityChanged,
//            [=](bool visible){ ui->actionactionToolBar->setChecked(visible);});

    connect(ui->mdiArea, &QMdiArea::subWindowActivated,
            [=](QMdiSubWindow *window){
        if(ui->listDocument->count() > 0 && window != NULL){
            ui->listDocument->setCurrentItem(&((MdiWindow*)window)->getListItem());

            ((QLabel*)(statusBarWidgets[1]))->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第") +
                                                      QString::number(ui->listDocument->row(&((MdiWindow*)window)->getListItem()) + 1) + tr("张"));
            ((QSlider*)(statusBarWidgets[3]))->setValue(((MdiWindow*)window)->getXScale() * 100);
        } else {
            ((QLabel*)(statusBarWidgets[1]))->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第0张"));
        }

    });

    ui->actionSave->setEnabled(false);
    ui->actionCloseAllNotSave->setEnabled(false);
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
    if(event->isAccepted()) {
        qApp->exit();
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
    GraphicsView * graphicsView = new GraphicsView(child);
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


    connect(graphicsView, &GraphicsView::zoom, [=](int n){
        QSlider* qSlider = (QSlider*)statusBarWidgets[3];
        //        QPropertyAnimation *animation = new QPropertyAnimation(qSlider, "value");
        //        animation->setDuration(100);
        //        animation->setEasingCurve(QEasingCurve::InOutQuart);
        //        animation->setStartValue(qSlider->value());
        //        animation->setEndValue(qSlider->value() + n);
        //        animation->start();
        qSlider->setValue(qSlider->value() + n);
    });

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
    //    statusBarWidgets[2] =
    statusBarWidgets[3] = new QSlider(Qt::Horizontal, this);
    statusBarWidgets[4] = new QToolButton(this);

    ((QCheckBox*)statusBarWidgets[0])->setText(tr("播放提示音"));
    ((QSlider*)statusBarWidgets[3])->setMaximum(1000);
    ((QSlider*)statusBarWidgets[3])->setMinimum(1);
    ((QSlider*)statusBarWidgets[3])->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    ((QSlider*)statusBarWidgets[3])->setSingleStep(5);
    ((QSlider*)statusBarWidgets[3])->setValue(100);
    ((QToolButton*)statusBarWidgets[4])->setAutoRaise(true);
    ((QToolButton*)statusBarWidgets[4])->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    // str转为int  输出占4位  十进制输出  前位补0
    QString value = QString("%1").arg(((QSlider*)statusBarWidgets[3])->value(), 3, 10, QLatin1Char('0'));
    ((QToolButton*)statusBarWidgets[4])->setText(value + "%");

    connect((QCheckBox*)statusBarWidgets[0], &QCheckBox::stateChanged,
            [=](int state){ PublicData::isPlaySound = (state == Qt::Checked); });

    connect((QSlider*)statusBarWidgets[3], &QAbstractSlider::valueChanged,
            [=](int value){
        MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->activeSubWindow());
        if ( activeWindow != NULL ) {
            QGraphicsView* graphicsView = (QGraphicsView*)(activeWindow->widget());
            double xScale = value / 100.0;
            double yScale = value / 100.0;
            //            qDebug() << xScale << " " << yScale;
            graphicsView->scale( xScale / activeWindow->getXScale(), yScale / activeWindow->getYScale() );
            activeWindow->setXScale(xScale);
            activeWindow->setYScale(yScale);

            QString showValue = QString("%1").arg(value, 3, 10, QLatin1Char('0'));
            ((QToolButton*)statusBarWidgets[4])->setText(showValue + "%");
        }
    });

    connect((QToolButton*)statusBarWidgets[4], &QAbstractButton::clicked,
            [=](){
        QSlider* qSlider = (QSlider*)statusBarWidgets[3];
        QPropertyAnimation *animation = new QPropertyAnimation(qSlider, "value");
        animation->setDuration(700);
        animation->setEasingCurve(QEasingCurve::OutElastic);
        animation->setStartValue(qSlider->value());
        animation->setEndValue(100);
        animation->start();
    });

    ((QLabel*)statusBarWidgets[1])->setText(tr("共") + QString::number(ui->listDocument->count()) + tr("张, 选中第") +
                                            QString::number(ui->listDocument->currentRow() + 1) + tr("张"));

    ui->statusbar->addPermanentWidget((QCheckBox*)statusBarWidgets[0]);
    ui->statusbar->addPermanentWidget((QLabel*)statusBarWidgets[1]);
    ui->statusbar->addPermanentWidget((QSlider*)statusBarWidgets[3]);
    ui->statusbar->addPermanentWidget((QToolButton*)statusBarWidgets[4]);
}

void MainWindow::commonSnapAction(int index, bool isHotKey)
{
    int windowIndex = 0;
    MdiWindow* activeWindow = NULL;
    QString name;
    switch (index) {
    case ScreenSnap: {
        if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[ScreenSnap].waitTime * 1000);
        windowIndex = createMDIWindow();
        activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(windowIndex));
        name = tr("全屏截图") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
        ((QGraphicsView*)(activeWindow->widget()))->scene()->addPixmap(getWindowPixmap((HWND)QApplication::desktop()->winId(), ScreenSnap, PublicData::includeCursor));
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
            HWND activeWindowHwnd = GetForegroundWindow();
            RECT activeWindowRect;
            GetWindowRect(activeWindowHwnd, &activeWindowRect);
            name = tr("活动窗口截图") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
            //            int windowFrameSizeX = GetSystemMetrics(SM_CXSIZEFRAME);
            //            int windowFrameSizeY = GetSystemMetrics(SM_CYSIZEFRAME);
            //            int windowCaptionSize = GetSystemMetrics(SM_CYCAPTION);
            QPixmap activeWindowPicture = getWindowPixmap(activeWindowHwnd,
                                                          ActiveWindowSnap,
                                                          PublicData::includeCursor,
                                                          0, 0,
                                                          activeWindowRect.right - activeWindowRect.left,
                                                          activeWindowRect.bottom - activeWindowRect.top);
            windowIndex = createMDIWindow();
            activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(windowIndex));
            ((QGraphicsView*)(activeWindow->widget()))->scene()->addPixmap(activeWindowPicture);

        } else {
            return;
        }
        break;
    }
    case CursorSnap: {
        if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[CursorSnap].waitTime * 1000);
        windowIndex = createMDIWindow();
        activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(windowIndex));
        name = tr("截取光标") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
        ((QGraphicsView*)(activeWindow->widget()))->scene()->addPixmap(
                    getWindowPixmap((HWND)activeWindow->widget()->winId(), CursorSnap, true
                                    , 0, 0, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR)));
        break;
    }
    case FreeSnap: {
        if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[FreeSnap].waitTime * 1000);
        QPixmap* picture = new QPixmap;
        FreeSnapDialog freeSnapDialog(getWindowPixmap((HWND)QApplication::desktop()->winId(), ScreenSnap, PublicData::includeCursor),
                                      picture, this);
        freeSnapDialog.exec();
        if (picture == NULL)
            return;
        windowIndex = createMDIWindow();
        activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(windowIndex));
        name = tr("自由截图") + " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
        ((QGraphicsView*)(activeWindow->widget()))->scene()->addPixmap(*picture);
        delete picture;
        break;
    }
    }

    activeWindow->setWindowTitle(name);
    activeWindow->setName(name);
    activeWindow->setListItemName(name);
    activeWindow->setSaved(false);

    if(PublicData::copyToClipBoardAfterSnap) ui->actionCopy->trigger();
    if(PublicData::isPlaySound) PlaySound(TEXT("DAZIJI"), NULL, SND_RESOURCE | SND_ASYNC);

    if (PublicData::snapType[index].isAutoSave && PublicData::snapType[index].autoSavePath != "") {
        savePicture(PublicData::snapType[index].autoSavePath + "/" + activeWindow->getName() + ".png");
    }
}

QPixmap MainWindow::getWindowPixmap(HWND winId, int type, bool includeCursor, int x, int y, int w , int h)
{
    if (type == CursorSnap) return grabWindow(winId, type, includeCursor, x, y, w, h);
    switch (PublicData::snapMethod) {
    case 0: {
        return grabWindow(winId, type, includeCursor, x, y, w, h);
    }
    case 1: {
        return grabWindow2(winId, type, includeCursor, x, y, w, h);
    }
    default: {
        return grabWindow(winId, type, includeCursor, x, y, w, h);
    }
    }
}

QPixmap MainWindow::grabWindow2(HWND winId, int type, bool includeCursor, int x, int y, int w , int h)
{
    RECT r;
    GetWindowRect(winId, &r);

    //多屏支持
    if (type == ScreenSnap) {
        r.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        r.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        r.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + r.left;
        r.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + r.top;
    }

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    long xBorder, yBorder, captionBorder;

    WindowsInfo::getWindowBorderSize(winId, &xBorder, &yBorder, &captionBorder);

    HDC display_dc = GetWindowDC(0);

    HDC bitmap_dc = CreateCompatibleDC(display_dc);
    HBITMAP bitmap = NULL;
    if (PublicData::noBorder && type == ActiveWindowSnap) {
        bitmap = CreateCompatibleBitmap(display_dc, w - 2 * xBorder,
                                        h -  2 * yBorder - captionBorder);
    } else {
        bitmap = CreateCompatibleBitmap(display_dc, w, h);
    }
    HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

    if (PublicData::noBorder && type == ActiveWindowSnap) {
        h -= yBorder;
        w -= yBorder;
        BitBlt(bitmap_dc, -xBorder,
               -captionBorder - yBorder,
               w, h, display_dc, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
        x += xBorder;
        y += (yBorder + captionBorder);
    } else {
        BitBlt(bitmap_dc, 0, 0, w, h, display_dc, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
    }

    if(includeCursor){
        CURSORINFO ci;
        ICONINFO iconInf;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        GetIconInfo(ci.hCursor, &iconInf);
        DrawIcon(bitmap_dc, ci.ptScreenPos.x - x - r.left - iconInf.xHotspot, ci.ptScreenPos.y - y - r.top - iconInf.yHotspot, ci.hCursor);
    }

    // clean up all but bitmap
    ReleaseDC(winId, display_dc);
    SelectObject(bitmap_dc, null_bitmap);
    DeleteDC(bitmap_dc);

    QPixmap pixmap = QtWin::fromHBITMAP(bitmap);

    DeleteObject(bitmap);

    return pixmap;
}

QPixmap MainWindow::grabWindow(HWND winId, int type, bool includeCursor, int x, int y, int w , int h)
{
    RECT r/*, r2*/;
    GetClientRect(winId, &r);
    //    GetWindowRect(winId, &r2);

    //多屏支持
    if (type == ScreenSnap) {
        r.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        r.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        r.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + r.left;
        r.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + r.top;
    }

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    long xBorder, yBorder, captionBorder;

    WindowsInfo::getWindowBorderSize(winId, &xBorder, &yBorder, &captionBorder);

    HDC display_dc = GetWindowDC(winId);
    //    if (PublicData::noBorder) {
    //        display_dc = GetDC(winId);
    //    } else {
    //        display_dc = GetWindowDC(winId);
    //    }

    HDC bitmap_dc = CreateCompatibleDC(display_dc);
    HBITMAP bitmap = NULL;
    if (PublicData::noBorder && type == ActiveWindowSnap) {
        bitmap = CreateCompatibleBitmap(display_dc, w - 2 * xBorder,
                                        h -  2 * yBorder - captionBorder);
    } else {
        bitmap = CreateCompatibleBitmap(display_dc, w, h);
    }
    HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

    if (PublicData::noBorder && type == ActiveWindowSnap) {
        h -= yBorder;
        w -= yBorder;
        BitBlt(bitmap_dc, -xBorder,
               -captionBorder - yBorder,
               w, h, display_dc, x, y, SRCCOPY | CAPTUREBLT);
        x += xBorder;
        y += (yBorder + captionBorder);
    } else {
        BitBlt(bitmap_dc, 0, 0, w, h, display_dc, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
    }

    if(includeCursor){
        CURSORINFO ci;
        ICONINFO iconInf;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        if(type == CursorSnap){
            RECT rect = {0, 0, w, h};
            FillRect(bitmap_dc, &rect, CreateSolidBrush(RGB(255, 255, 255)));
            //            SetBkMode(bitmap_dc, TRANSPARENT);
            //            qDebug() << TransparentBlt(bitmap_dc, 0, 0, w - 1, h - 1, bitmap_dc, 0, 0, w, h, RGB(255, 255, 255));
            DrawIcon(bitmap_dc, 0, 0, ci.hCursor);
        } else {
            RECT winRect;
            GetWindowRect(winId, &winRect);
            GetIconInfo(ci.hCursor, &iconInf);
//            DrawIcon(bitmap_dc, ci.ptScreenPos.x - x - r.left - iconInf.xHotspot, ci.ptScreenPos.y - y - r.top - iconInf.yHotspot, ci.hCursor);
            DrawIcon(bitmap_dc, ci.ptScreenPos.x - x - r.left - winRect.left - iconInf.xHotspot, ci.ptScreenPos.y - y - r.top - winRect.top - iconInf.yHotspot, ci.hCursor);
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
        this->show();
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
            this->setWindowState(Qt::WindowActive);
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
    case FreeSnap: {
        commonSnapAction(FreeSnap, true);
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

void MainWindow::on_actionCloseAllNotSave_triggered()
{
    if (QMessageBox::warning(this, tr("警告"), tr("确认是否要关闭所有文档且不保存"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        closeAllNotSave = true;
        ui->mdiArea->closeAllSubWindows();
        closeAllNotSave = false;
    }
}

void MainWindow::on_actionOpenSource_triggered()
{
    ShellExecute((HWND)this->winId(), (LPCWSTR)L"open",
                 (LPCWSTR)L"https://github.com/SkyD666/SmartScreenSnaper",
                 (LPCWSTR)L"", (LPCWSTR)L"", SW_SHOWNORMAL);
}

void MainWindow::on_actionCopy_triggered()
{
    if (!ui->listDocument->count()) return;
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    if (activeWindow != NULL) {
        QGraphicsScene* graphicsScene = ((QGraphicsView*)(activeWindow->widget()))->scene();
        QImage image(QSize(graphicsScene->width(), graphicsScene->height()),QImage::Format_ARGB32);
        QPainter painter(&image);
        graphicsScene->render(&painter);
        QApplication::clipboard()->setImage(image);
    }\
}

void MainWindow::on_actionFreeSnap_triggered()
{
    commonSnapAction(FreeSnap, false);
}
