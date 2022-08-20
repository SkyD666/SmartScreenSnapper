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
bool PublicData::includeShadow = false;
QString PublicData::gifSavePath = "";
QString PublicData::styleName = "";
QString PublicData::qssPath = "";
QString PublicData::imageExtName[] = {".png", ".jpg", ".bmp"};
SnapType PublicData::snapType[SNAPTYPECOUNT] = {{0, "", false, QApplication::applicationDirPath(), imageExtName[0]},
                                                {0, "", false, QApplication::applicationDirPath(), imageExtName[0]},
                                                {0, "", false, QApplication::applicationDirPath(), imageExtName[0]},
                                                {0, "", false, QApplication::applicationDirPath(), imageExtName[0]}};
QList<QList<MyGlobalShortCut*>> PublicData::hotKey;

PublicData::PublicData()
{

}

QString PublicData::getConfigFilePath()
{
    return QCoreApplication::applicationDirPath() + "/SmartScreenSnapperConfig.ini";
}

void PublicData::readSettings()
{
    QSettings qSettings(getConfigFilePath(), QSettings::IniFormat);

    int size = qSettings.beginReadArray("SnapType");

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        snapType[i].waitTime = qSettings.value("WaitTime", 0).toInt();
        snapType[i].hotKey = qSettings.value("HotKey", "").toString();
        snapType[i].isAutoSave = qSettings.value("IsAutoSave", false).toBool();
        snapType[i].autoSavePath = qSettings.value("AutoSavePath", QApplication::applicationDirPath()).toString();
        snapType[i].autoSaveExtName = qSettings.value("AutoSaveExtName", imageExtName[0]).toString();
    }
    qSettings.endArray();

    isPlaySound = qSettings.value("Sound/IsPlaySound", false).toBool();
    clickCloseToTray = qSettings.value("Config/ClickCloseToTray", true).toBool();
    hotKeyNoWait = qSettings.value("Config/HotKeyNoWait", true).toBool();
    includeCursor = qSettings.value("Config/IncludeCursor", false).toBool();
    noBorder = qSettings.value("Config/NoBorder", false).toBool();
    snapMethod = qSettings.value("Config/SnapMethod", SnapMethod2).toInt();
    copyToClipBoardAfterSnap = qSettings.value("Config/CopyToClipBoardAfterSnap", false).toBool();
    gifSavePath =  qSettings.value("Tool/GIFSavePath", "").toString();
    styleName =  qSettings.value("Config/StyleName", "").toString();
    qssPath =  qSettings.value("Config/QssPath", "").toString();
}

void PublicData::writeSettings()
{
    QSettings qSettings(getConfigFilePath(), QSettings::IniFormat);

    qSettings.beginWriteArray("SnapType");

    int size = sizeof(snapType) / sizeof(SnapType);

    for (int i = 0; i < size; i++) {
        qSettings.setArrayIndex(i);
        qSettings.setValue("WaitTime", snapType[i].waitTime);
        qSettings.setValue("HotKey", snapType[i].hotKey);
        qSettings.setValue("IsAutoSave", snapType[i].isAutoSave);
        qSettings.setValue("AutoSavePath", snapType[i].autoSavePath);
        qSettings.setValue("AutoSaveExtName", snapType[i].autoSaveExtName);
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
    qSettings.setValue("Config/QssPath", qssPath);
}

void PublicData::registerAllHotKey(QWidget* parent)
{
    QStringList registeredKeyList;    //记录次热键是否被本程序注册，如果注册过了就不再注册了
    for (int i = 0; i < (int)(sizeof(PublicData::snapType)/sizeof(SnapType)); i++){
        QList<MyGlobalShortCut*> hotKey;
        QStringList keys = PublicData::snapType[i].hotKey.split(", ");
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
            QObject::connect(shortcut, SIGNAL(activatedHotKey(int)), parent, SLOT(hotKeyPressed(int)));
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
