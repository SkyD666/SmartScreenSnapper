#include "application.h"
#include <windows.h>

Application::Application(int argc, char *argv[]): QApplication(argc, argv)
{

}

bool Application::nativeEventFilter(const QByteArray &eventType, void *message, qintptr*) {
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG * pMsg = reinterpret_cast<MSG *>(message);
        if (pMsg->message == WM_CLOSE) {
//            PublicData::ignoreClickCloseToTray = true;
        }
    }
    return false;
}
