#ifndef PUBLICDATA_H
#define PUBLICDATA_H

#include <QList>
#include <QHash>
#include "screenshothelper.h"
#include "MyGlobalShortcut/MyGlobalShortCut.h"

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

    static int saveImageQuality;

    //0原始方法，1全屏截图再截取
    static int snapMethod;

    static bool copyToClipBoardAfterSnap;

    static ShotTypeItem snapTypeItems[ScreenShotHelper::ShotType::Count];

    static QPair<QString, QString> imageExtName[7];

    static QString getSaveExtFilter();

    static QString getConfigFilePath();

    static void readSettings();

    static void writeSettings();

    static void registerAllHotKey(QWidget* parent, std::function<void (ScreenShotHelper::ShotType shotType)> receiver);

    static void unregisterAllHotKey();

    static QHash<ScreenShotHelper::ShotType, QList<MyGlobalShortCut*>> hotKey;

    static bool applyQss();

    static bool editMode;
};

#endif // PUBLICDATA_H
