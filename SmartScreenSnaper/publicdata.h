#ifndef PUBLICDATA_H
#define PUBLICDATA_H

#include <QList>
#include "MyGlobalShortcut/MyGlobalShortCut.h"

#define SNAPTYPECOUNT 4
#define MDIWINBKCOLOR RGB(160, 160, 160)

#define SNAPMETHOD 2

enum {
    ScreenSnap = 0, ActiveWindowSnap = 1, CursorSnap = 2, FreeSnap = 3
};

enum {
    SnapMethod1 = 0, SnapMethod2 = 1
};

struct SnapType{
    int waitTime;
    QString hotKey;
    bool isAutoSave;
    QString autoSavePath;
    QString autoSaveExtName;
};

class PublicData
{

public:
    PublicData();

    static int activeWindowIndex;

    static int totalWindowCount;        //只增不减

    static bool isPlaySound;

    static bool clickCloseToTray;

    static bool ignoreClickCloseToTray;

    static bool hotKeyNoWait;

    static bool includeCursor;

    static bool noBorder;

    //0原始方法，1全屏截图再截取
    static int snapMethod;

    static bool copyToClipBoardAfterSnap;

    //0全屏截图，1活动窗口截图，2截取光标
    static SnapType snapType[SNAPTYPECOUNT];

    static QString imageExtName[3];

    static void readSettings();

    static void writeSettings();

    static void registerAllHotKey(QWidget* parent);

    static void unregisterAllHotKey();

    //0全屏，1活动窗口截图
    static QList<QList<MyGlobalShortCut*>> hotKey;

};

#endif // PUBLICDATA_H
