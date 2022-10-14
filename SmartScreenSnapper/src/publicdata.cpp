#include "publicdata.h"
#include "mainwindow.h"
#include "MyGlobalShortcut/MyGlobalShortCut.h"
#include <QSettings>
#include <QApplication>

int PublicData::activeWindowIndex = -1;
int PublicData::totalWindowCount = 0;
int PublicData::snapMethod = 0;
bool PublicData::isPlaySound = false;
bool PublicData::clickCloseToTray = true;
bool PublicData::ignoreClickCloseToTray = false;
bool PublicData::hotKeyNoWait = true;
bool PublicData::includeCursor = false;
bool PublicData::noBorder = false;
bool PublicData::copyToClipBoardAfterSnap = false;
QString PublicData::gifSavePath = "";
QString PublicData::styleName = "";
QString PublicData::fileNameTemplate = "";
QString PublicData::qssPath = "";
QPair<QString, QString> PublicData::imageExtName[] = {{".png", QObject::tr("PNG 便携网络图形")},
                                                      {".jpg", QObject::tr("JPG")},
                                                      {".bmp", QObject::tr("BMP 位图")},
                                                      {".tiff", QObject::tr("TIFF 标签图像文件")},
                                                      {".webp", QObject::tr("WebP")},
                                                      {".ico", QObject::tr("ICO 图标")}};
ShotTypeItem PublicData::snapTypeItems[SNAPTYPECOUNT] = {};
QHash<ScreenShotHelper::ShotType, QList<MyGlobalShortCut*>> PublicData::hotKey;

PublicData::PublicData()
{

}

QString PublicData::getSaveExtFilter()
{
    QString s;
    for (auto item : imageExtName) {
        s += item.second + " (*" + item.first + ");;";
    }
    if (s.endsWith(";;")) {
        s.remove(s.length() - 2, 2);
    }
    return s;
}

QString PublicData::getConfigFilePath()
{
    return QCoreApplication::applicationDirPath() + "/SmartScreenSnapperConfig.ini";
}

void PublicData::readSettings()
{
    // 初始化数据，防止qSettings.beginReadArray("SnapType")是0导致数据不完整
    for (int i = 0; i < ScreenShotHelper::Count; i++) {
        snapTypeItems[i].shotType = ScreenShotHelper::ShotType(i);
        snapTypeItems[i].waitTime = 0;
        snapTypeItems[i].hotKey = "";
        snapTypeItems[i].isAutoSave = false;
        snapTypeItems[i].autoSavePath = QApplication::applicationDirPath();
        snapTypeItems[i].autoSaveExtName = imageExtName[0].first;
    }

    QSettings qSettings(getConfigFilePath(), QSettings::IniFormat);

    int size = qSettings.beginReadArray("SnapType");

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        snapTypeItems[i].shotType = ScreenShotHelper::ShotType(qSettings.value("ShotType", i).toInt());
        snapTypeItems[i].waitTime = qSettings.value("WaitTime", 0).toInt();
        snapTypeItems[i].hotKey = qSettings.value("HotKey", "").toString();
        snapTypeItems[i].isAutoSave = qSettings.value("IsAutoSave", false).toBool();
        snapTypeItems[i].autoSavePath = qSettings.value("AutoSavePath").toString();
        if (snapTypeItems[i].autoSavePath.isEmpty()) {          // 字符串类型配置值不能自动识别为空串
            snapTypeItems[i].autoSavePath = QApplication::applicationDirPath();
        }
        snapTypeItems[i].autoSaveExtName = qSettings.value("AutoSaveExtName").toString();
        if (snapTypeItems[i].autoSaveExtName.isEmpty()) {       // 字符串类型配置值不能自动识别为空串
            snapTypeItems[i].autoSaveExtName = imageExtName[0].first;
        }
    }
    qSettings.endArray();

    isPlaySound = qSettings.value("Sound/IsPlaySound", false).toBool();
    clickCloseToTray = qSettings.value("Config/ClickCloseToTray", true).toBool();
    hotKeyNoWait = qSettings.value("Config/HotKeyNoWait", true).toBool();
    includeCursor = qSettings.value("Config/IncludeCursor", false).toBool();
    noBorder = qSettings.value("Config/NoBorder", false).toBool();
    snapMethod = qSettings.value("Config/SnapMethod", SnapMethod2).toInt();
    copyToClipBoardAfterSnap = qSettings.value("Config/CopyToClipBoardAfterSnap", false).toBool();
    gifSavePath = qSettings.value("Tool/GIFSavePath", "").toString();
    styleName = qSettings.value("Config/StyleName", "").toString();
    fileNameTemplate = qSettings.value("Config/FileNameTemplate", "").toString();
    qssPath = qSettings.value("Config/QssPath", "").toString();
}

void PublicData::writeSettings()
{
    QSettings qSettings(getConfigFilePath(), QSettings::IniFormat);

    qSettings.beginWriteArray("SnapType");

    int size = sizeof(snapTypeItems) / sizeof(ShotTypeItem);

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        qSettings.setValue("ShotType", snapTypeItems[i].shotType);
        qSettings.setValue("WaitTime", snapTypeItems[i].waitTime);
        qSettings.setValue("HotKey", snapTypeItems[i].hotKey);
        qSettings.setValue("IsAutoSave", snapTypeItems[i].isAutoSave);
        qSettings.setValue("AutoSavePath", snapTypeItems[i].autoSavePath);
        qSettings.setValue("AutoSaveExtName", snapTypeItems[i].autoSaveExtName);
    }
    qSettings.endArray();

    qSettings.setValue("Sound/IsPlaySound", isPlaySound);
    qSettings.setValue("Config/ClickCloseToTray", clickCloseToTray);
    qSettings.setValue("Config/HotKeyNoWait", hotKeyNoWait);
    qSettings.setValue("Config/IncludeCursor", includeCursor);
    qSettings.setValue("Config/NoBorder", noBorder);
    qSettings.setValue("Config/SnapMethod", snapMethod);
    qSettings.setValue("Config/CopyToClipBoardAfterSnap", copyToClipBoardAfterSnap);
    qSettings.setValue("Tool/GIFSavePath", gifSavePath);
    qSettings.setValue("Config/StyleName", styleName);
    qSettings.setValue("Config/FileNameTemplate", fileNameTemplate);
    qSettings.setValue("Config/QssPath", qssPath);
}

void PublicData::registerAllHotKey(QWidget* parent)
{
    QStringList registeredKeyList;    //记录次热键是否被本程序注册，如果注册过了就不再注册了
    for (int i = 0; i < (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem)); i++){
        QList<MyGlobalShortCut*> hotKey;
        QStringList keys = PublicData::snapTypeItems[i].hotKey.split(", ");
        for (int j = 0; j < keys.size(); j++) {
            QString key = keys.at(j);
            if (key == "") continue;
            MyGlobalShortCut* shortcut = NULL;
            if (registeredKeyList.contains(key)) {
                shortcut = new MyGlobalShortCut(key, parent, false);
            } else {
                registeredKeyList.push_back(key);
                shortcut = new MyGlobalShortCut(key, parent, true);
            }
            hotKey.push_back(shortcut);
            QObject::connect(shortcut,
                             SIGNAL(activatedHotKey(ScreenShotHelper::ShotType)),
                             parent,
                             SLOT(hotKeyPressed(ScreenShotHelper::ShotType)));
        }
        PublicData::hotKey.insert(PublicData::snapTypeItems[i].shotType, hotKey);
    }
}

void PublicData::unregisterAllHotKey()
{
    while (!PublicData::hotKey.isEmpty()) {
        QList<MyGlobalShortCut*> hotKey = PublicData::hotKey.begin().value();
        for (int i = 0; i < hotKey.size(); i++){
            hotKey.at(i)->unregisterHotKey();
            delete hotKey.at(i);
        }
        PublicData::hotKey.remove(PublicData::hotKey.begin().key());
    }
}

bool PublicData::applyQss()
{
    if (PublicData::qssPath == "") {
        qApp->setStyleSheet("");
        return true;
    }
    QFile qssFile(PublicData::qssPath);
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen()) {
        qApp->setStyleSheet(QLatin1String(qssFile.readAll()));
        qssFile.close();
        return true;
    }
    return false;
}
