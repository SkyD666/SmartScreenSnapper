#include "screenshothelper.h"
#include <QtWinExtras/qwinfunctions.h>
#include <QTimer>
#include <windows.h>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QDir>
#include <QDateTime>
#include "freesnapdialog.h"
#include "publicdata.h"
#include "windowsinfo.h"

ScreenShotHelper::ScreenShotHelper()
{

}

QPixmap ScreenShotHelper::grabWindow(int snapMethod, HWND winId, int type, bool includeCursor, int x, int y, int w , int h)
{
    RECT r = {0, 0, 0, 0};

    // 多屏支持
    if (type == ScreenShot) {
        r.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        r.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        r.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + r.left;
        r.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + r.top;
    } else {
        GetWindowRect(winId, &r);
    }

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    long xBorder, yBorder, captionBorder;
    WindowsInfo::getWindowBorderSize(winId, &xBorder, &yBorder, &captionBorder);

    HDC displayDC = nullptr;
    if (snapMethod == 0) {
        displayDC = GetWindowDC(winId);
    } else if (snapMethod == 1) {
        displayDC = GetWindowDC(0);
    }

    HDC bitmapDC = CreateCompatibleDC(displayDC);
    HBITMAP bitmap = nullptr;
    if (PublicData::noBorder) {
        bitmap = CreateCompatibleBitmap(displayDC, w - 2 * xBorder,
                                        h -  2 * yBorder - captionBorder);
    } else {
        bitmap = CreateCompatibleBitmap(displayDC, w, h);
    }
    HGDIOBJ null_bitmap = SelectObject(bitmapDC, bitmap);

    if (PublicData::noBorder) {
        h -= yBorder;
        w -= xBorder;
        if (snapMethod == 0) {
            BitBlt(bitmapDC, -xBorder,
                   -captionBorder - yBorder,
                   w, h, displayDC, x, y, SRCCOPY | CAPTUREBLT);
        } else if (snapMethod == 1) {
            BitBlt(bitmapDC, -xBorder,
                   -captionBorder - yBorder,
                   w, h, displayDC, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
        }
        x += xBorder;
        y += (yBorder + captionBorder);
    } else {
        if (snapMethod == 0) {
            BitBlt(bitmapDC, 0, 0, w, h, displayDC, x, y, SRCCOPY | CAPTUREBLT);
        } else if (snapMethod == 1) {
            BitBlt(bitmapDC, 0, 0, w, h, displayDC, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
        }
    }

    if (includeCursor) {
        CURSORINFO ci;
        ICONINFO iconInf;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        GetIconInfo(ci.hCursor, &iconInf);
        DrawIcon(bitmapDC, ci.ptScreenPos.x - x - r.left - iconInf.xHotspot, ci.ptScreenPos.y - y - r.top - iconInf.yHotspot, ci.hCursor);
    }

    // clean up all but bitmap
    ReleaseDC(winId, displayDC);
    SelectObject(bitmapDC, null_bitmap);
    DeleteDC(bitmapDC);

    QPixmap pixmap = QtWin::fromHBITMAP(bitmap, QtWin::HBitmapPremultipliedAlpha);

    DeleteObject(bitmap);

    return pixmap;
}

QPixmap ScreenShotHelper::grabCursor()
{
    HDC hdcScreen = GetDC(0);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    HBITMAP hbmCanvas = CreateCompatibleBitmap(hdcScreen,
                                               GetSystemMetrics(SM_CXCURSOR),
                                               GetSystemMetrics(SM_CYCURSOR));

    HGDIOBJ hbmOld = SelectObject(hdcMem, hbmCanvas);

    CURSORINFO ci;
    ci.cbSize = sizeof(ci);
    GetCursorInfo(&ci);

    DrawIconEx(hdcMem, 0, 0, ci.hCursor,
               0, 0, 0, NULL, DI_NORMAL);

    QPixmap p = QtWin::fromHBITMAP(hbmCanvas, QtWin::HBitmapPremultipliedAlpha);

    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmCanvas);
    DeleteDC(hdcMem);
    ReleaseDC(0, hdcScreen);

    return p;
}

void wait(int msec)
{
    QEventLoop loop;            //定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();                    //事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

QPixmap ScreenShotHelper::screenshot(ShotType shotType, bool isHotKey)
{
    QPixmap pixmap;

    ShotTypeItem snapType = PublicData::snapTypeItems[shotType];
    if (!isHotKey || !PublicData::hotKeyNoWait) {
        wait(snapType.waitTime * 1000);
    }
    switch (shotType) {
    case ScreenShot: {
        pixmap = getWindowPixmap((HWND)QApplication::desktop()->winId(),
                                 shotType,
                                 PublicData::includeCursor);
        break;
    }
    case ActiveWindowShot: {
        pixmap = getWindowPixmap(GetForegroundWindow(),
                                 shotType,
                                 PublicData::includeCursor,
                                 0, 0);
        break;
    }
    case CursorShot: {
        pixmap = grabCursor();
        break;
    }
    default: break;
    }
    return pixmap;
}

QPixmap ScreenShotHelper::getWindowPixmap(HWND winId, ShotType shotType, bool includeCursor, int x, int y, int w , int h) {
    if (shotType == CursorShot) {
        return grabCursor();
    } else {
        return ScreenShotHelper::grabWindow(PublicData::snapMethod, winId, shotType, includeCursor, x, y, w, h);
    }
}

QString ScreenShotHelper::getPictureName(ShotType shotType)
{
    QString time = " - " + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss_zzz");
    QString typeName;
    switch (shotType) {
    case ScreenShot:{
        typeName = "全屏截图";
        break;
    }
    case ActiveWindowShot:{
        typeName = "活动窗口截图";
        break;
    }
    case CursorShot:{
        typeName = "截取光标";
        break;
    }
    case FreeShot:{
        typeName = "自由截图";
        break;
    }
    case FreeHandShot:{
        typeName = "徒手截图";
        break;
    }
    default:{
        typeName = "截图";
    }
    }
    return typeName + time;
}

bool ScreenShotHelper::savePicture(QString filePath, QPixmap pixmap) {
    QDir *dir;
    bool saved = false;
    if (!filePath.isEmpty()) {
        dir = new QDir(filePath.left(filePath.lastIndexOf('/') + 1));
    } else {
        dir = new QDir(filePath);
    }
    if (!dir->exists()) dir->mkdir(dir->path());
    saved = pixmap.save(filePath);
    delete dir;
    return saved;
}

QPixmap ScreenShotHelper::getFullScreen()
{
    return getWindowPixmap(
                (HWND)QApplication::desktop()->winId(),
                ScreenShot,
                PublicData::includeCursor);
}
