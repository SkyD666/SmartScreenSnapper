#include "screenshothelper.h"
#include <QTimer>
#include <windows.h>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QScreen>
#include <QDebug>
#include <QRandomGenerator>
#include "publicdata.h"
#include "windowsinfo.h"
#include "util.h"

unsigned long ScreenShotHelper::lastPId = 0L;

ScreenShotHelper::ScreenShotHelper()
{

}

QPixmap ScreenShotHelper::grabWindow(int snapMethod, HWND hwnd, int type, bool includeCursor, int x, int y, int w , int h)
{
    GetWindowThreadProcessId(hwnd, &lastPId);

    RECT r = {0, 0, 0, 0};

    // 多屏支持
    if (type == ScreenShot) {
        r.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        r.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        r.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + r.left;
        r.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + r.top;
    } else {
        GetWindowRect(hwnd, &r);
    }

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    long xBorder, yBorder, captionBorder;
    WindowsInfo::getWindowBorderSize(hwnd, &xBorder, &yBorder, &captionBorder);

    HDC displayDC = nullptr;
    if (snapMethod == 0) {
        displayDC = GetWindowDC(hwnd);
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
    ReleaseDC(hwnd, displayDC);
    SelectObject(bitmapDC, null_bitmap);
    DeleteDC(bitmapDC);

    QImage image = QImage::fromHBITMAP(bitmap);
    image.reinterpretAsFormat(QImage::QImage::Format_ARGB32_Premultiplied);
    QPixmap pixmap = QPixmap::fromImage(image);

    DeleteObject(bitmap);

    return pixmap;
}

/**
 * @brief qt_pixmapFromWinHBITMAP
 * https://github.com/qt/qtbase/blob/067b53864112c084587fa9a507eb4bde3d50a6e1/src/gui/image/qpixmap_win.cpp
 * QImage::fromHBITMAP(hbmCanvas);会丢失透明度（默认不透明），因此需要手动调用qt_pixmapFromWinHBITMAP
 * @param bitmap
 * @param hbitmapFormat
 * enum HBitmapFormat
 * {
 *     HBitmapNoAlpha = 0,
 *     HBitmapPremultipliedAlpha = 1,
 *     HBitmapAlpha = 2
 * };
 * @return QPixmap
 */
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat = 0);

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

    QPixmap p = qt_pixmapFromWinHBITMAP(hbmCanvas, 1);

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
        pixmap = getWindowPixmap((HWND)QGuiApplication::primaryScreen()->handle(),
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
    QString defaultTemplate = "<Capture> - <yyyy>-<MM>-<dd>_<HH><mm><ss>_<Rand>";
    QString fileNameTemplate = PublicData::fileNameTemplate;
    if (fileNameTemplate.isEmpty()) {
        fileNameTemplate = defaultTemplate;
    }
    QHash<QString, QString> kv;
    QDateTime dt = QDateTime::currentDateTime();
    kv["yyyy"] = dt.toString("yyyy");
    kv["MM"] = dt.toString("MM");
    kv["dd"] = dt.toString("dd");
    kv["Capture"] = getSnapTypeName(shotType);
    kv["AP"] = dt.time().toString("AP");
    kv["HH"] = dt.toString("HH");
    kv["mm"] = dt.toString("mm");
    kv["ss"] = dt.toString("ss");
    kv["zzz"] = dt.toString("zzz");
    kv["ActivePId"] = QString::number(GetWindowThreadProcessId(GetForegroundWindow(), &lastPId));
    kv["Rand"] = QString::number(QRandomGenerator::global()->bounded(100000, 999999));

    QString result = Util::transformTemplateStr(fileNameTemplate, kv);
    if (result.isEmpty()) result = Util::transformTemplateStr(defaultTemplate, kv);
    return result;
}

QString ScreenShotHelper::getSnapTypeName(ScreenShotHelper::ShotType shotType)
{
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
    case ShotByPoint:{
        typeName = "窗体控件截图";
        break;
    }
    default:{
        typeName = "截图";
    }
    }
    return typeName;
}

bool ScreenShotHelper::savePicture(QWidget *msgBoxParent, QString filePath, QPixmap pixmap) {
    QDir *dir;
    bool saved = false;
    if (!filePath.isEmpty()) {
        dir = new QDir(filePath.left(filePath.lastIndexOf('/') + 1));
    } else {
        dir = new QDir(filePath);
    }
    if (!dir->exists()) dir->mkdir(dir->path());
    saved = pixmap.save(filePath, nullptr, PublicData::saveImageQuality);
    if (!saved) {
        QMessageBox::critical(msgBoxParent, QObject::tr("警告"), QObject::tr("保存图片失败"), QMessageBox::Ok);
    }
    delete dir;
    return saved;
}

QPixmap ScreenShotHelper::getFullScreen()
{
    return getWindowPixmap(
                (HWND)QGuiApplication::primaryScreen()->handle(),
                ScreenShot,
                PublicData::includeCursor);
}
