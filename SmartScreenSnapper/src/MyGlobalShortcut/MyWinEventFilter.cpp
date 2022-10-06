#include "MyWinEventFilter.h"
#include "MyGlobalShortCut.h"
#include "../publicdata.h"
//#include <qt_windows.h>
MyWinEventFilter::MyWinEventFilter()
{

}
MyWinEventFilter::~MyWinEventFilter()
{

}

MyWinEventFilter::MyWinEventFilter(MyGlobalShortCut *shortcut)
    : m_shortcut(shortcut)
{

}


bool MyWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
    if(eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if(msg->message == WM_HOTKEY) {
            const quint32 keycode = HIWORD(msg->lParam);
            const quint32 modifiers = LOWORD(msg->lParam);
            bool res = m_shortcut->shortcuts.value(qMakePair(keycode, modifiers));
            if (res) {
                for (int i = 0; i < PublicData::hotKey.size(); i++) {
                    ScreenShotHelper::ShotType shotType = PublicData::hotKey.keys().at(i);
                    QList<MyGlobalShortCut*> hotKey = PublicData::hotKey.value(shotType);
                    for (int j = 0; j < hotKey.size(); j++){
                        if (MyGlobalShortCut::nativeKeycode(hotKey.at(j)->key) == HIWORD(msg->lParam) &&
                                MyGlobalShortCut::nativeModifiers(hotKey.at(j)->mods) == LOWORD(msg->lParam)) {
                            m_shortcut->activateShortcut(shotType);
                        }
                    }
                }
                return true;
            }
        }

    }
    return false;
}
