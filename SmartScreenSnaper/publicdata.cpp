#include "publicdata.h"
#include "mainwindow.h"
#include "MyGlobalShortcut/MyGlobalShortCut.h"
#include <QSettings>
#include <QApplication>

int PublicData::activeWindowIndex = -1;
int PublicData::totalWindowCount = 0;
bool PublicData::isPlaySound = false;
bool PublicData::clickCloseToTray = true;
bool PublicData::ignoreClickCloseToTray = false;
bool PublicData::hotKeyNoWait = true;
bool PublicData::includeCursor = false;
bool PublicData::noBorder = false;
bool PublicData::copyToClipBoardAfterSnap = false;
SnapType PublicData::snapType[SNAPTYPECOUNT] = {{0, "", false, QApplication::applicationDirPath()},
                                                {0, "", false, QApplication::applicationDirPath()},
                                                {0, "", false, QApplication::applicationDirPath()},
                                                {0, "", false, QApplication::applicationDirPath()}};
QList<QList<MyGlobalShortCut*>> PublicData::hotKey;

PublicData::PublicData()
{

}

void PublicData::readSettings()
{
    QSettings qSettings(QCoreApplication::applicationDirPath() + "/SmartScreenSnaperConfig.ini", QSettings::IniFormat);

    int size = qSettings.beginReadArray("SnapType");

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        snapType[i].waitTime = qSettings.value("WaitTime", 0).toInt();
        snapType[i].hotKey = qSettings.value("HotKey", "").toString();
        snapType[i].isAutoSave = qSettings.value("IsAutoSave", false).toBool();
        snapType[i].autoSavePath = qSettings.value("AutoSavePath", QApplication::applicationDirPath()).toString();
    }
    qSettings.endArray();

    isPlaySound = qSettings.value("Sound/IsPlaySound", false).toBool();
    clickCloseToTray = qSettings.value("Config/ClickCloseToTray", true).toBool();
    hotKeyNoWait = qSettings.value("Config/HotKeyNoWait", true).toBool();
    includeCursor = qSettings.value("Config/IncludeCursor", false).toBool();
    noBorder = qSettings.value("Config/NoBorder", false).toBool();
    copyToClipBoardAfterSnap = qSettings.value("Config/CopyToClipBoardAfterSnap", false).toBool();
}

void PublicData::writeSettings()
{
    QSettings qSettings(QCoreApplication::applicationDirPath() + "/SmartScreenSnaperConfig.ini", QSettings::IniFormat);

    qSettings.beginWriteArray("SnapType");

    int size = sizeof(snapType) / sizeof(SnapType);

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        qSettings.setValue("WaitTime", snapType[i].waitTime);
        qSettings.setValue("HotKey", snapType[i].hotKey);
        qSettings.setValue("IsAutoSave", snapType[i].isAutoSave);
        qSettings.setValue("AutoSavePath", snapType[i].autoSavePath);
    }
    qSettings.endArray();

    qSettings.setValue("Sound/IsPlaySound", isPlaySound);
    qSettings.setValue("Config/ClickCloseToTray", clickCloseToTray);
    qSettings.setValue("Config/HotKeyNoWait", hotKeyNoWait);
    qSettings.setValue("Config/IncludeCursor", includeCursor);
    qSettings.setValue("Config/NoBorder", noBorder);
    qSettings.setValue("Config/CopyToClipBoardAfterSnap", copyToClipBoardAfterSnap);
}

void PublicData::registerAllHotKey(QObject* app)
{
    for (int i = 0; i < (int)(sizeof(PublicData::snapType)/sizeof(SnapType)); i++){
        QList<MyGlobalShortCut*> hotKey;
        QStringList keys = PublicData::snapType[i].hotKey.split(", ");
        for (int j = 0; j < keys.size(); j++) {
            QString key = keys.at(j);
            MyGlobalShortCut* shortcut = new MyGlobalShortCut(key, app);
            hotKey.push_back(shortcut);
            QObject::connect(shortcut, SIGNAL(activatedHotKey(int)), app, SLOT(hotKeyPressed(int)));
        }
        PublicData::hotKey.push_back(hotKey);
    }
}

void PublicData::unregisterAllHotKey()
{
    for (int i = 0; i < PublicData::hotKey.size(); i++){
        for (int j = 0; j < PublicData::hotKey.at(i).size(); j++){
            PublicData::hotKey.at(i).at(j)->unregisterHotKey();
            delete PublicData::hotKey.at(i).at(j);
        }
    }
    PublicData::hotKey.clear();
}
