#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWidget>
#include <QClipboard>
#include <QGraphicsView>
#include <QDateTime>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QDesktopServices>
#include <QPrintDialog>
#include <QPrinter>
#include <QStyleFactory>
#include <QNetworkReply>
#include <QSoundEffect>
#include <QMimeData>
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
#include "longsnapdialog.h"
#include "freehandsnapdialog.h"
#include "snapfrompointdialog.h"

bool MainWindow::exitApp = false;
bool MainWindow::noToAllClicked = false;
bool MainWindow::closeAllNotSave = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);

    initSystemTray();
    initStatusBar();        // 这句里面有控件初始化，在设置值之前进行

    PublicData::readSettings();
    PublicData::registerAllHotKey(this, [=](ScreenShotHelper::ShotType shotType){
        hotKeyPressed(shotType);
    });

    setSettings();

    connect(ui->listDocument, &QListWidget::currentItemChanged, this,
            [=](QListWidgetItem *current, QListWidgetItem *){
        bool hasDocument = current;
        if (current) {
            ui->mdiArea->setActiveSubWindow(current->data(MdiWindow::MdiWindowRole).value<MdiWindow*>());
        }
        ui->actionSave->setEnabled(hasDocument);
        ui->actionCloseAllNotSave->setEnabled(hasDocument);
        ui->actionPrint->setEnabled(hasDocument);
        updateDocumentCountLabel();
    });
    connect(ui->listDocument->model(), &QAbstractItemModel::rowsRemoved, this, [=](const QModelIndex &, int, int){
        updateDocumentCountLabel();
    });
    connect(ui->listDocument->model(), &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex &, int, int){
        updateDocumentCountLabel();
    });

    networkAccessManager = new QNetworkAccessManager();
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, [=] (QNetworkReply *reply) {
        GitHubRelease* gitHubRelease = UpdateUtil::getData(this, reply->readAll());

        if (gitHubRelease && UpdateUtil::isNewVersion(gitHubRelease->name)) {
            UpdateDialog(this, gitHubRelease).exec();
        }

        reply->deleteLater();
    });
    UpdateUtil::checkUpdate(networkAccessManager);

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, [=](QMdiSubWindow *window){
        MdiWindow *mdiWindow = qobject_cast<MdiWindow*>(window);
        QListWidgetItem *item = nullptr;
        if (ui->listDocument->count() > 0 && mdiWindow) {
            item = &mdiWindow->getListItem();
            ui->listDocument->setCurrentItem(item);
            sliderZoom->setValue(mdiWindow->getScale() * 100);
        }
    });

    ui->actionSave->setEnabled(false);
    ui->actionCloseAllNotSave->setEnabled(false);
    ui->actionPrint->setEnabled(false);
    //------------------------------------------------
    connectActionSlots();
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
    QMdiSubWindow *currentSubWindow = nullptr;
    while (subWindowCount-- > 0) {
        currentSubWindow = ui->mdiArea->currentSubWindow();
        if (currentSubWindow->close()) {
            ui->mdiArea->removeSubWindow(currentSubWindow);
        }
        if (!exitApp) {
            PublicData::ignoreClickCloseToTray = false;
            event->ignore();
            return;
        }
    }
    if (event->isAccepted()) {
        qApp->exit();
    }
}

void MainWindow::connectActionSlots()
{
    // 全屏截图
    connect(ui->actionScreenSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::ScreenShot, false);
    });
    // 活动窗口截图
    connect(ui->actionActiveWindowSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::ActiveWindowShot, false);
    });
    // 截取光标
    connect(ui->actionCursorSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::CursorShot, false);
    });
    // 自由截图
    connect(ui->actionFreeSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::FreeShot, false);
    });
    // 截取窗体/控件
    connect(ui->actionSnapByPoint, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::ShotByPoint, false);
    });
    // 徒手截图
    connect(ui->actionFreeHandSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::FreeHandShot, false);
    });
    // 长截屏
    connect(ui->actionLongSnap, &QAction::triggered, this, [=](){
        commonSnapAction(ScreenShotHelper::LongShot, false);
    });
    // 录制GIF
    connect(ui->actionGIF, &QAction::triggered, this, [=](){
        (new GIFDialog(this))->show();       // 构造函数里有setAttribute(Qt::WA_DeleteOnClose);无需手动delete
    });
    // 显示
    connect(ui->actionShow, &QAction::triggered, this, [=](){
        show();
    });
    // 保存
    connect(ui->actionSave, &QAction::triggered, this, [=](){
        getActiveWindow()->saveByDialog();
    });
    // 打印
    connect(ui->actionPrint, &QAction::triggered, this, [=](){
        MdiWindow* activeWindow = getActiveWindow();
        if (!activeWindow) return;
        QPrintDialog printDialog(this);
        printDialog.setWindowTitle(tr("打印"));
        if (printDialog.exec() == QPrintDialog::Accepted) {
            QPrinter* printer = printDialog.printer();
            QPixmap pixmap = activeWindow->getPixmap();

            QPainter painter(printer);
            QRect rect = painter.viewport();                       // painter矩形区域
            QSize size = pixmap.size();                            // 图片的大小
            size.scale(rect.size(), Qt::KeepAspectRatio);          // 按照图形比例大小重新设置视口矩形区域
            painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            painter.setWindow(pixmap.rect());
            painter.drawPixmap(0, 0, pixmap);
        }
    });
    // 关闭所有文档且不保存
    connect(ui->actionCloseAllNotSave, &QAction::triggered, this, [=](){
        if (QMessageBox::warning(this, tr("警告"), tr("确认是否要关闭所有文档且不保存"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
            closeAllNotSave = true;
            ui->mdiArea->closeAllSubWindows();
            closeAllNotSave = false;
        }
    });
    // 复制
    connect(ui->actionCopy, &QAction::triggered, this, [=](){
        if (!ui->listDocument->count()) return;
        QPixmap pixmap = getActiveWindow()->getPixmap();
        QImage image = pixmap.toImage();
        QMimeData *data = new QMimeData;
        data->setImageData(image);
        QApplication::clipboard()->setMimeData(data, QClipboard::Clipboard);
        QApplication::clipboard()->setImage(image);
    });
    // 退出
    connect(ui->actionExit, &QAction::triggered, this, [=](){
        PublicData::ignoreClickCloseToTray = true;
        close();
    });
    // 检查更新
    connect(ui->actionUpdate, &QAction::triggered, this, [=](){
        UpdateDialog().exec();
    });
    // 开放源码
    connect(ui->actionOpenSource, &QAction::triggered, this, [=](){
        QDesktopServices::openUrl(QUrl(Const::githubRepository));
    });
    // 关于
    connect(ui->actionAbout, &QAction::triggered, this, [=](){
        AboutDialog aboutDialog;
        aboutDialog.exec();
    });
    // 设置
    connect(ui->actionSetting, &QAction::triggered, this, [=](){
        SettingDialog settingDialog(this);
        PublicData::unregisterAllHotKey();      // 进入设置页面前取消热键
        settingDialog.exec();
        if (cbPlaySound) {
            cbPlaySound->setChecked(PublicData::isPlaySound);
        }
        PublicData::registerAllHotKey(this, [=](ScreenShotHelper::ShotType shotType){
            hotKeyPressed(shotType);
        });
    });
    // 动态添加皮肤菜单
    int styleCount = QStyleFactory::keys().count();
    QMenu *styleMenu = new QMenu(this);
    styleMenu->setTitle(tr("外观"));
    connect(styleMenu, &QMenu::triggered, this, [=](QAction *action){
        QList<QAction*> actions = styleMenu->actions();
        for (int i = 0; i < actions.count(); i++) {
            actions.at(i)->setChecked(false);
        }
        action->setChecked(true);
        PublicData::styleName = action->text();
        qApp->setStyle(action->text());
    });
    for (int i = 0; i < styleCount; i++) {
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
}

MdiWindow * MainWindow::createMDIWindow() {
    MdiWindow *child = new MdiWindow(ui->mdiArea);
    ui->listDocument->addItem(&child->getListItem());
    ui->mdiArea->addSubWindow(child);
    child->setWindowState(PublicData::mdiWindowInitState);
    child->show();
    ui->mdiArea->setActiveSubWindow(child);

    QString name = tr("文档") + " " + QString::number(++PublicData::totalWindowCount);
    child->setListItemName(name);
    child->setName(name);

    connect(child, &MdiWindow::zoom, this, [=](int n){
        sliderZoom->setValue(sliderZoom->value() + n);
    });

    PublicData::activeWindowIndex = ui->mdiArea->subWindowList().indexOf(child);
    return child;
}

MdiWindow* MainWindow::getActiveWindow()
{
    return qobject_cast<MdiWindow*>(ui->mdiArea->currentSubWindow());
}

void MainWindow::updateDocumentCountLabel()
{
    labCount->setText(
                tr("共%1张, 选中第%2张")
                .arg(ui->listDocument->count())
                .arg(ui->listDocument->currentRow() + 1));
}

void MainWindow::initStatusBar() {
    cbPlaySound = new QCheckBox(this);
    labCount = new QLabel(this);
    sliderZoom = new QSlider(Qt::Horizontal, this);
    sbZoom = new QSpinBox(this);

    connect(sliderZoom, &QAbstractSlider::valueChanged, this, [=](int value){
        sbZoom->setValue(value);
        MdiWindow* activeWindow = getActiveWindow();
        if (activeWindow) {
            activeWindow->setScale(value / 100.0);
        }
    });

    connect(sbZoom, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value){
        sliderZoom->setValue(value);
    });

    cbPlaySound->setText(tr("播放提示音"));
    sliderZoom->setMaximum(1000);
    sliderZoom->setMinimum(1);
    sliderZoom->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    sliderZoom->setSingleStep(5);
    sbZoom->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    sbZoom->setSuffix("%");
    sbZoom->setRange(sliderZoom->minimum(), sliderZoom->maximum());
    sliderZoom->setValue(100);

    connect(cbPlaySound, &QCheckBox::stateChanged, this, [=](int state){
        PublicData::isPlaySound = (state == Qt::Checked);
    });

    updateDocumentCountLabel();

    ui->statusbar->addPermanentWidget(cbPlaySound);
    ui->statusbar->addPermanentWidget(labCount);
    ui->statusbar->addPermanentWidget(sliderZoom);
    ui->statusbar->addPermanentWidget(sbZoom);
}

void MainWindow::commonSnapAction(ScreenShotHelper::ShotType shotType, bool isHotKey) {
    if (shotType == ScreenShotHelper::ActiveWindowShot && !isHotKey) {
        int result = QMessageBox::information(this,
                                              tr("活动窗口截图"),
                                              tr("点击确定%1秒后截图").arg(QString::number(PublicData::snapTypeItems[shotType].waitTime)),
                                              QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
        if (result != QMessageBox::Yes) return;
    }

    if (shotType == ScreenShotHelper::FreeShot) {
        QPixmap pixmap;
        bool captured = true;
        FreeSnapDialog freeSnapDialog(&pixmap, captured, this);
        freeSnapDialog.exec();
        if (!captured) return;
        snapSuccessCallback(shotType, pixmap);
    } else if (shotType == ScreenShotHelper::LongShot) {
        LongSnapDialog longSnapDialog(this);
        connect(&longSnapDialog, &LongSnapDialog::captured, this, [=](QPixmap pixmap){
            snapSuccessCallback(shotType, pixmap);
        });
        longSnapDialog.exec();
    } else if (shotType == ScreenShotHelper::FreeHandShot) {
        FreeHandSnapDialog freeHandSnapDialog(this);
        connect(&freeHandSnapDialog, &FreeHandSnapDialog::captured, this, [=](QPixmap pixmap){
            snapSuccessCallback(shotType, pixmap);
        });
        freeHandSnapDialog.exec();
    } else if (shotType == ScreenShotHelper::ShotByPoint) {
        SnapByPointDialog snapFromPointDialog(this);
        connect(&snapFromPointDialog, &SnapByPointDialog::captured, this, [=](QPixmap pixmap){
            snapSuccessCallback(shotType, pixmap);
        });
        snapFromPointDialog.exec();
    } else {
        snapSuccessCallback(shotType, ScreenShotHelper::screenshot(shotType, isHotKey));
    }
}

void MainWindow::snapSuccessCallback(ScreenShotHelper::ShotType shotType, QPixmap pixmap)
{
    MdiWindow* activeWindow = nullptr;
    QString name = ScreenShotHelper::getPictureName(shotType);
    activeWindow = createMDIWindow();
    activeWindow->setName(name);
    activeWindow->setPixmap(pixmap);

    if (PublicData::copyToClipBoardAfterSnap) {
        QApplication::clipboard()->setPixmap(pixmap);
    }
    if (PublicData::isPlaySound) {
        QSoundEffect *startSound = new QSoundEffect(this);
        startSound->setSource(QUrl::fromLocalFile(":/sound/typewriter.wav"));
        connect(startSound, &QSoundEffect::playingChanged, this, [=](){
            if (!startSound->isPlaying()) {
                disconnect(this, nullptr, startSound, nullptr);
                delete startSound;
            }
        });
        startSound->play();

    }

    ShotTypeItem snapTypeItem = PublicData::snapTypeItems[shotType];
    if (snapTypeItem.isAutoSave) {
        QString folderPath = snapTypeItem.autoSavePath + "/";
        folderPath.replace("://", ":\\");
        activeWindow->saveByPath(folderPath + name + snapTypeItem.autoSaveExtName);
    }
    if (snapTypeItem.isManualSave) {
        activeWindow->saveByDialog();
    }
}

void MainWindow::exitCancel()
{
    exitApp = false;
}

void MainWindow::initSystemTray()
{
    systemTrayMenu.addAction(ui->actionShow);
    systemTrayMenu.addSeparator();
    systemTrayMenu.addAction(ui->actionScreenSnap);
    systemTrayMenu.addAction(ui->actionActiveWindowSnap);
    systemTrayMenu.addAction(ui->actionCursorSnap);
    systemTrayMenu.addAction(ui->actionFreeSnap);
    systemTrayMenu.addAction(ui->actionGIF);
    systemTrayMenu.addAction(ui->actionFreeHandSnap);
    systemTrayMenu.addAction(ui->actionSnapByPoint);
    systemTrayMenu.addSeparator();
    systemTrayMenu.addAction(ui->actionExit);

    systemTray.setIcon(QIcon(":/image/icon.png"));
    systemTray.setToolTip(QApplication::applicationName());
    systemTray.setContextMenu(&systemTrayMenu);

    connect(&systemTray, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        switch (reason) {
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

void MainWindow::hotKeyPressed(ScreenShotHelper::ShotType shotType)
{
    commonSnapAction(shotType, true);
}
