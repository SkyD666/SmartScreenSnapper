#ifndef PUBLICDATA_H
#define PUBLICDATA_H

#include <QList>
#include <QHash>
#include "screenshothelper.h"
#include "MyGlobalShortcut/MyGlobalShortCut.h"

#define SNAPTYPECOUNT ScreenShotHelper::Count
#define MDIWINBKCOLOR RGB(160, 160, 160)

#define SNAPMETHOD 2

enum {
    SnapMethod1 = 0, SnapMethod2 = 1
};

struct ShotTypeItem {
    ScreenShotHelper::ShotType shotType;
    int waitTime;
    QString hotKey;
    bool isAutoSave;
    bool isManualSave;
    QString autoSavePath;
    QString autoSaveExtName;
};

class PublicData
{

public:
    PublicData();

    static QString qssPath;

    static QString gifSavePath;

    static QString styleName;       // 外观主题名称

    static QString fileNameTemplate;       // 文件名模板

    static int activeWindowIndex;

    static int totalWindowCount;        // 只增不减

    static bool isPlaySound;

    static QHash<Qt::WindowState, QString> mdiWindowInitStates;
    static Qt::WindowState mdiWindowInitState;

    static bool clickCloseToTray;

    static bool ignoreClickCloseToTray;

    static bool hotKeyNoWait;

    static bool includeCursor;

    static bool noBorder;

    //0原始方法，1全屏截图再截取
    static int snapMethod;

    static bool copyToClipBoardAfterSnap;

    //0全屏截图，1活动窗口截图，2截取光标
    static ShotTypeItem snapTypeItems[SNAPTYPECOUNT];

    static QPair<QString, QString> imageExtName[6];

    static QString getSaveExtFilter();

    static QString getConfigFilePath();

    static void readSettings();

    static void writeSettings();

    static void registerAllHotKey(QWidget* parent);

    static void unregisterAllHotKey();

    //0全屏，1活动窗口截图
    static QHash<ScreenShotHelper::ShotType, QList<MyGlobalShortCut*>> hotKey;

    static bool applyQss();
};

#endif // PUBLICDATA_H
