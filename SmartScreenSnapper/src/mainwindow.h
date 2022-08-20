#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    int createMDIWindow();

    void initSystemTray();

    void initStatusBar();

    static void exitCancel();

    void setSettings();

    void commonSnapAction(int index, bool isHotKey);

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

    void hotKeyPressed(int);

    QPixmap getWindowPixmap(HWND winId, int type, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

    void savePicture(QString filePath);

    void on_actionCursorSnap_triggered();

    void on_actionCloseAllNotSave_triggered();

    void on_actionOpenSource_triggered();

    void on_actionCopy_triggered();

    void on_actionFreeSnap_triggered();

    void on_actionUpdate_triggered();

    void on_actionPrint_triggered();

    void on_actionGIF_triggered();

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
