#pragma once
#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <QObject>
#include "MyWinEventFilter.h"
#include <QKeySequence>
#include <QHash>
#include "src/screenshothelper.h"

class MyGlobalShortCut : public QObject
{
    Q_OBJECT
signals:
    void activatedHotKey(ScreenShotHelper::ShotType shotType);
public:
    MyGlobalShortCut();
    MyGlobalShortCut(QWidget* parent);
    MyGlobalShortCut(QString key,QWidget* parent, bool reportError);
    ~MyGlobalShortCut();
    void activateShortcut(ScreenShotHelper::ShotType shotType);
    bool registerHotKey();
    bool unregisterHotKey();
    QHash<QPair<quint32, quint32>, MyGlobalShortCut*> shortcuts;

    Qt::Key key;
    Qt::KeyboardModifiers mods;
    static quint32 nativeKeycode(Qt::Key keycode);
    static quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);
private:
    QWidget* m_parent;
    QApplication *m_app;
    MyWinEventFilter *m_filter;
    QKeySequence m_key;

};
