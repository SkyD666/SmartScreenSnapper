#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mdiwindow.h"
#include "screenshothelper.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QToolButton>
#include <windows.h>
#include <Mmsystem.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    MdiWindow *createMDIWindow(int &windowIndex);

    void initSystemTray();

    void initStatusBar();

    static void exitCancel();

    void setSettings();

    void commonSnapAction(ScreenShotHelper::ShotType shotType, bool isHotKey);

    // 获取到截图后回调函数，用来更新mdiwindow
    void snapSuccessCallback(ScreenShotHelper::ShotType shotType, QPixmap pixmap);

    static bool exitApp;

    static bool noToAllClicked;

    static bool closeAllNotSave;
private slots:
    void on_actionNew_triggered();

    void on_actionScreenSnap_triggered();

    void on_actionSave_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionActiveWindowSnap_triggered();

    void on_actionSetting_triggered();

    void hotKeyPressed(ScreenShotHelper::ShotType shotType);

    void savePicture(QString filePath);

    void savePicture(QString filePath, QPixmap pixmap);

    QPixmap getActiveWindowPixmap();

    void on_actionCursorSnap_triggered();

    void on_actionCloseAllNotSave_triggered();

    void on_actionOpenSource_triggered();

    void on_actionCopy_triggered();

    void on_actionFreeSnap_triggered();

    void on_actionUpdate_triggered();

    void on_actionPrint_triggered();

    void on_actionGIF_triggered();

    void on_actionLongSnap_triggered();

    void on_actionFreeHandSnap_triggered();

private:
    Ui::MainWindow *ui;

    QCheckBox* cbPlaySound;
    QLabel* labCount;
    QSlider* sliderZoom;
    QToolButton* tbtnZoom;

    QSystemTrayIcon systemTray;

    QMenu systemTrayMenu;

    QAction systemTrayMenuActions[2];

    QNetworkAccessManager *networkAccessManager;

protected:

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
