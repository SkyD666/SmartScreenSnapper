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
#include <QDesktopServices>
#include <QPrintDialog>
#include <QPrinter>
#include <QStyleFactory>
#include <QNetworkReply>
#include "graphicsview.h"
#include "windowsinfo.h"
#include "publicdata.h"
#include "mdiwindow.h"
#include "aboutdialog.h"
#include "settingdialog.h"
#include "freesnapdialog.h"
#include "updatedialog.h"
#include "screenshothelper.h"
#include "gifdialog.h"
#include "updateutil.h"
#include "const.h"

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

bool MainWindow::exitApp = false;
bool MainWindow::noToAllClicked = false;
bool MainWindow::closeAllNotSave = false;

void wait(int msec);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);

    initSystemTray();

    initStatusBar();        //这句里面有控件初始化，在设置值之前进行

    PublicData::readSettings();

    PublicData::registerAllHotKey(this);

    setSettings();

    connect(ui->listDocument, &QListWidget::itemClicked, [=](){
        if (ui->listDocument->count() > 0) {
            ui->mdiArea->subWindowList().at(ui->listDocument->currentRow())->setFocus();
        }
    });

    connect(ui->listDocument, &QListWidget::currentRowChanged, [=](int currentRow){
        if (!MainWindow::exitApp) {
            if (currentRow == -1) {
                ui->actionSave->setEnabled(false);
                ui->actionCloseAllNotSave->setEnabled(false);
                ui->actionPrint->setEnabled(false);
            } else {
                ui->actionSave->setEnabled(true);
                ui->actionCloseAllNotSave->setEnabled(true);
                ui->actionPrint->setEnabled(true);
            }
        }
    });

    networkAccessManager = new QNetworkAccessManager();
    connect(networkAccessManager, &QNetworkAccessManager::finished, this , [=] (QNetworkReply *reply) {
        GitHubRelease* gitHubRelease = UpdateUtil::getData(this, reply->readAll());
        if (UpdateUtil::isNewVersion(gitHubRelease->name)) {
            UpdateDialog(this, gitHubRelease).exec();
        }
        reply->deleteLater();
    });
    UpdateUtil::checkUpdate(networkAccessManager);

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, [=](QMdiSubWindow *window){
        if (ui->listDocument->count() > 0 && window) {
            QListWidgetItem *item = &((MdiWindow*)window)->getListItem();
            ui->listDocument->setCurrentItem(item);

            labCount->setText(
                        tr("共%1张, 选中第%2张")
                        .arg(ui->listDocument->count())
                        .arg(ui->listDocument->row(item) + 1));
            sliderZoom->setValue(((MdiWindow*)window)->getXScale() * 100);
        } else {
            labCount->setText(
                        tr("共%1张, 选中第%2张")
                        .arg(ui->listDocument->count())
                        .arg(0));
        }
    });

    ui->actionSave->setEnabled(false);
    ui->actionCloseAllNotSave->setEnabled(false);
    ui->actionPrint->setEnabled(false);

    //动态添加皮肤菜单-------------------------
    int styleCount = QStyleFactory::keys().count();
    QMenu *styleMenu = new QMenu(this);
    styleMenu->setTitle(tr("外观"));
    connect(styleMenu, &QMenu::triggered, [=](QAction *action){
        QList<QAction*> actions = styleMenu->actions();
        for (int i = 0; i < actions.count(); i++) {
            actions.at(i)->setChecked(false);
        }
        action->setChecked(true);
        PublicData::styleName = action->text();
        qApp->setStyle(action->text());
    });
    for(int i = 0; i < styleCount; i++) {
        QAction *action = new QAction(QStyleFactory::keys().at(i), this);

        action->setCheckable(true);
        styleMenu->addAction(action);

        if (action->text() == PublicData::styleName) {
            action->setChecked(true);
            qApp->setStyle(action->text());
        }
    }
    ui->menuTool->insertMenu(ui->actionSetting, styleMenu);
    ui->menuTool->insertSeparator(ui->actionSetting);
    //------------------------------------------------
}

MainWindow::~MainWindow() {
    PublicData::writeSettings();

    PublicData::unregisterAllHotKey();

    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event) {
    if (PublicData::clickCloseToTray && !PublicData::ignoreClickCloseToTray) {
        setVisible(false);
        event->ignore();
        return;
    }

    exitApp = true;
    int subWindowCount = ui->mdiArea->subWindowList().count();
    while(subWindowCount-->0){
        ui->listDocument->setCurrentRow(subWindowCount);
        if (ui->mdiArea->subWindowList().last()->close()) {
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
}

void MainWindow::on_actionNew_triggered() {
    createMDIWindow();
}

int MainWindow::createMDIWindow() {
    MdiWindow *child = new MdiWindow(this);
    QGraphicsScene* graphicsScene = new QGraphicsScene(child);
    GraphicsView * graphicsView = new GraphicsView(child);
    graphicsView->setScene(graphicsScene);
    graphicsView->setBackgroundBrush(QBrush(MDIWINBKCOLOR));
    child->setWidget(graphicsView);
    child->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(child);
    child->show();

    ui->listDocument->addItem(&(child->getListItem()));
    ui->listDocument->setCurrentItem(&child->getListItem());

    labCount->setText(
                tr("共%1张, 选中第%2张")
                .arg(ui->listDocument->count())
                .arg(ui->listDocument->row(&child->getListItem()) + 1));

    QString name = tr("文档") + " " + QString::number(++PublicData::totalWindowCount);
    child->setListItemName(name);
    child->setName(name);

    connect(graphicsView, &GraphicsView::zoom, [=](int n){
        sliderZoom->setValue(sliderZoom->value() + n);
    });

    connect(child, &MdiWindow::save, this, &MainWindow::on_actionSave_triggered);

    connect(child, &MdiWindow::close, [=](){
        if(ui->mdiArea->subWindowList().size() > 1)
            ui->listDocument->setCurrentRow(ui->listDocument->count() - 2);
    });

    PublicData::activeWindowIndex = ui->mdiArea->subWindowList().size() - 1;
    return PublicData::activeWindowIndex;
}

void MainWindow::on_actionSave_triggered() {
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存"),
                                                    activeWindow->getName(), tr("PNG文件(*.png);;JPG文件(*.jpg);;BMP文件(*.bmp)"));
    if (filePath != "") savePicture(filePath);
}

void MainWindow::savePicture(QString filePath) {
    QDir *dir;
    if (filePath != "") {
        dir = new QDir(filePath.left(filePath.lastIndexOf('/') + 1));
    } else {
        dir = new QDir(filePath);
    }
    if (!dir->exists()) dir->mkdir(dir->path());
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    QGraphicsScene* graphicsScene = ((QGraphicsView*)(activeWindow->widget()))->scene();
    QImage image(QSize(graphicsScene->width(), graphicsScene->height()),QImage::Format_ARGB32);
    QPainter painter(&image);
    graphicsScene->render(&painter);
    if (image.save(filePath)) {
        activeWindow->setSaved(true);
    } else {
        QMessageBox::critical(this, tr("警告"), tr("保存图片失败"), QMessageBox::Ok);
    }
    delete dir;
}

void MainWindow::on_actionExit_triggered() {
    PublicData::ignoreClickCloseToTray = true;
    close();
}

void MainWindow::on_actionAbout_triggered() {
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::initStatusBar() {
    cbPlaySound = new QCheckBox(this);
    labCount = new QLabel(this);
    sliderZoom = new QSlider(Qt::Horizontal, this);
    tbtnZoom = new QToolButton(this);

    cbPlaySound->setText(tr("播放提示音"));
    sliderZoom->setMaximum(1000);
    sliderZoom->setMinimum(1);
    sliderZoom->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    sliderZoom->setSingleStep(5);
    sliderZoom->setValue(100);
    tbtnZoom->setAutoRaise(true);
    tbtnZoom->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    tbtnZoom->setText(QString::number(sliderZoom->value()) + "%");

    connect(cbPlaySound, &QCheckBox::stateChanged, [=](int state){
        PublicData::isPlaySound = (state == Qt::Checked);
    });

    connect(sliderZoom, &QAbstractSlider::valueChanged, [=](int value){
        MdiWindow* activeWindow = dynamic_cast<MdiWindow*>(ui->mdiArea->activeSubWindow());
        if (activeWindow) {
            QGraphicsView* graphicsView = dynamic_cast<QGraphicsView*>(activeWindow->widget());
            double xScale = value / 100.0;
            double yScale = value / 100.0;
            graphicsView->scale( xScale / activeWindow->getXScale(), yScale / activeWindow->getYScale() );
            activeWindow->setXScale(xScale);
            activeWindow->setYScale(yScale);

            tbtnZoom->setText(QString::number(value) + "%");
        }
    });

    connect(tbtnZoom, &QAbstractButton::clicked, [=](){
        QPropertyAnimation *animation = new QPropertyAnimation(sliderZoom, "value");
        animation->setDuration(700);
        animation->setEasingCurve(QEasingCurve::OutElastic);
        animation->setStartValue(sliderZoom->value());
        animation->setEndValue(100);
        animation->start();
    });

    labCount->setText(
                tr("共%1张, 选中第%2张")
                .arg(ui->listDocument->count())
                .arg(ui->listDocument->currentRow() + 1));

    ui->statusbar->addPermanentWidget(cbPlaySound);
    ui->statusbar->addPermanentWidget(labCount);
    ui->statusbar->addPermanentWidget(sliderZoom);
    ui->statusbar->addPermanentWidget(tbtnZoom);
}

void MainWindow::commonSnapAction(int index, bool isHotKey) {
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
        int result = QMessageBox::Yes;
        if (!isHotKey || !PublicData::hotKeyNoWait) {
            result = QMessageBox::information(this, tr("活动窗口截图"),
                                              tr("点击确定%1秒后截图").arg(QString::number(PublicData::snapType[ActiveWindowSnap].waitTime)),
                                              QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
        }
        if (result == QMessageBox::Yes) {
            if (!isHotKey || !PublicData::hotKeyNoWait) wait(PublicData::snapType[ActiveWindowSnap].waitTime * 1000);
            HWND activeWindowHwnd = GetForegroundWindow();
            QPixmap activeWindowPicture = getWindowPixmap(activeWindowHwnd,
                                                          ActiveWindowSnap,
                                                          PublicData::includeCursor,
                                                          0, 0/*,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  activeWindowRect.right - activeWindowRect.left,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  activeWindowRect.bottom - activeWindowRect.top*/);
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

    if (PublicData::snapType[index].isAutoSave) {
        QString folderPath = PublicData::snapType[index].autoSavePath + "/";
        folderPath.replace("://", ":\\");
        savePicture(folderPath + activeWindow->getName() + PublicData::snapType[index].autoSaveExtName);
    }
}

QPixmap MainWindow::getWindowPixmap(HWND winId, int type, bool includeCursor, int x, int y, int w , int h) {
    if (type == CursorSnap) return ScreenShotHelper::grabWindow(0, winId, type, includeCursor, x, y, w, h);
    switch (PublicData::snapMethod) {
    case 1: {
        return ScreenShotHelper::grabWindow(1, winId, type, includeCursor, x, y, w, h);
    }
    case 0:
    default: {
        return ScreenShotHelper::grabWindow(0, winId, type, includeCursor, x, y, w, h);
    }
    }
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

    systemTray.setIcon(QIcon(":/image/icon.png"));
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
    if (!PublicData::applyQss()) QMessageBox::critical(this, tr("警告"), tr("QSS文件打开失败"), QMessageBox::Ok);

    if (cbPlaySound) {
        cbPlaySound->setChecked(PublicData::isPlaySound);
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
    if (cbPlaySound) {
        cbPlaySound->setChecked(PublicData::isPlaySound);
    }
    PublicData::registerAllHotKey(this);
}

void MainWindow::hotKeyPressed(int i)
{
    commonSnapAction(i, true);
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
    QDesktopServices::openUrl(QUrl(Const::githubRepository));
}

void MainWindow::on_actionCopy_triggered()
{
    if (!ui->listDocument->count()) return;
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    if (activeWindow) {
        QGraphicsScene* graphicsScene = ((QGraphicsView*)(activeWindow->widget()))->scene();
        QImage image(QSize(graphicsScene->width(), graphicsScene->height()),QImage::Format_ARGB32);
        QPainter painter(&image);
        graphicsScene->render(&painter);
        QApplication::clipboard()->setImage(image);
    }
}

void MainWindow::on_actionFreeSnap_triggered()
{
    commonSnapAction(FreeSnap, false);
}

void MainWindow::on_actionUpdate_triggered()
{
    UpdateDialog().exec();
}

void MainWindow::on_actionPrint_triggered()
{
    MdiWindow* activeWindow = (MdiWindow*)(ui->mdiArea->subWindowList().at(ui->listDocument->currentRow()));
    if (!activeWindow) return;
    QPrintDialog printDialog(this);
    printDialog.setWindowTitle(tr("打印"));
    if(printDialog.exec() == QPrintDialog::Accepted) {
        QPrinter* printer = printDialog.printer();
        //printer->setOutputFileName(activeWindow->getName());        //文件名
        QGraphicsScene* graphicsScene = ((QGraphicsView*)(activeWindow->widget()))->scene();
        QImage image(QSize(graphicsScene->width(), graphicsScene->height()),QImage::Format_ARGB32);
        QPainter painter(&image);
        graphicsScene->render(&painter);

        QPainter painter2(printer);
        QRect rect = painter2.viewport();                           //painter2矩形区域
        QSize size = image.size();                                      //图片的大小
        size.scale(rect.size(), Qt :: KeepAspectRatio);          //按照图形比例大小重新设置视口矩形区域
        painter2.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter2.setWindow(image.rect());
        painter2.drawImage(0, 0, image);
    }
}

void MainWindow::on_actionGIF_triggered()
{
    GIFDialog *gifDialog = new GIFDialog;       //构造函数里有setAttribute(Qt::WA_DeleteOnClose);无需手动delete
    gifDialog->show();
}
