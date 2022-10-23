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
#include <QSpinBox>

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

    void hotKeyPressed(ScreenShotHelper::ShotType shotType);
private slots:

    void savePicture(QString filePath);

    void savePicture(QString filePath, QPixmap pixmap);

    QPixmap getActiveWindowPixmap();

private:
    Ui::MainWindow *ui;

    QCheckBox* cbPlaySound;
    QLabel* labCount;
    QSlider* sliderZoom;
    QSpinBox* sbZoom;

    QSystemTrayIcon systemTray;

    QMenu systemTrayMenu;

    QNetworkAccessManager *networkAccessManager;

protected:

    void closeEvent(QCloseEvent *event);

    void connectActionSlots();
};
#endif // MAINWINDOW_H
