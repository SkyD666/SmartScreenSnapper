#include "basefullscreensnapdialog.h"
#include "screenshothelper.h"
#include <windows.h>

BaseFullScreenSnapDialog::BaseFullScreenSnapDialog(QWidget* parent)
    : QDialog(parent)
{
}

void BaseFullScreenSnapDialog::doAfterSetupUi()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowState(Qt::WindowActive);

    auto devicePixelRatio = devicePixelRatioF();
    // 多显示器支持
    move(GetSystemMetrics(SM_XVIRTUALSCREEN) / devicePixelRatio,
        GetSystemMetrics(SM_YVIRTUALSCREEN) / devicePixelRatio);
    resize(GetSystemMetrics(SM_CXVIRTUALSCREEN) / devicePixelRatio,
        GetSystemMetrics(SM_CYVIRTUALSCREEN) / devicePixelRatio);

    // 可在不点击鼠标的情况下捕获移动事件
    setMouseTracking(true);

    fullScreenPixmap = ScreenShotHelper::layersToPixmap(ScreenShotHelper::getFullScreen());
    fullScreenPixmap.setDevicePixelRatio(devicePixelRatio);
}
