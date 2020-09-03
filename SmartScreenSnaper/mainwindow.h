#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
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

    void* getStatusBarItem(int index);

    void commonSnapAction(int index, bool isHotKey);

    static bool exitApp;

    static bool noToAllClicked;
private slots:
    void on_actionNew_triggered();

    void on_actionScreenSnap_triggered();

    void on_actionSave_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionActiveWindowSnap_triggered();

    void on_actionSetting_triggered();

    void hotKeyPressed(int);

    QPixmap grabWindow(HWND winId, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1, bool isCursorSnap = false);

    void savePicture(QString filePath);

    void on_actionCursorSnap_triggered();

private:
    Ui::MainWindow *ui;

    void* statusBarWidgets[2];

    QSystemTrayIcon systemTray;

    QMenu systemTrayMenu;

    QAction systemTrayMenuActions[2];

protected:

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
