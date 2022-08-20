#include "screenshothelper.h"
#include <QtWinExtras/qwinfunctions.h>
#include <windows.h>
#include "publicdata.h"
#include "windowsinfo.h"

ScreenShotHelper::ScreenShotHelper()
{

}

QPixmap ScreenShotHelper::grabWindow(int snapMethod, HWND winId, int type, bool includeCursor, int x, int y, int w , int h)
{
    RECT r = {0, 0, 0, 0};
    GetWindowRect(winId, &r);

    //多屏支持，全屏截图时始终不包含阴影
    if (type == ScreenSnap) {
        r.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        r.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        r.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + r.left;
        r.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + r.top;
    } else if (PublicData::includeShadow) {
        //包括窗体阴影，需要调整r的大小
        int shadowSize = WindowsInfo::getShadowSize(winId);
        r.left -= shadowSize;
        r.right += shadowSize;
        r.top -= shadowSize;
        r.bottom += shadowSize;
        if (snapMethod == 0) {
            //因为下面BitBlt(bitmap_dc, 0, 0, w, h, display_dc, x, y, SRCCOPY | CAPTUREBLT);
            //没有用到r.left和r.top所以这里直接改x和y来调整阴影左上角坐标
            //但是snapMethod == 0，此时GetWindowDC(winId)获取后只有当前窗口区域数据，无法扩展到阴影部分，因此会有黑框
            //（snapMethod == 0不支持截取阴影）
            x -= shadowSize;
            y -= shadowSize;
        }
    }

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    long xBorder, yBorder, captionBorder;

    WindowsInfo::getWindowBorderSize(winId, &xBorder, &yBorder, &captionBorder);

    HDC display_dc = NULL;
    if (snapMethod == 0) {
        display_dc = GetWindowDC(winId);
    } else if (snapMethod == 1) {
        display_dc = GetWindowDC(0);
    }

    HDC bitmap_dc = CreateCompatibleDC(display_dc);
    HBITMAP bitmap = NULL;
    if (PublicData::noBorder && type == ActiveWindowSnap) {
        bitmap = CreateCompatibleBitmap(display_dc, w - 2 * xBorder,
                                        h -  2 * yBorder - captionBorder);
    } else {
        bitmap = CreateCompatibleBitmap(display_dc, w, h);
    }
    HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

    if (PublicData::noBorder && !PublicData::includeShadow && type == ActiveWindowSnap) {
        h -= yBorder;
        w -= yBorder;
        if (snapMethod == 0) {
            BitBlt(bitmap_dc, -xBorder,
                   -captionBorder - yBorder,
                   w, h, display_dc, x, y, SRCCOPY | CAPTUREBLT);
        } else if (snapMethod == 1) {
            BitBlt(bitmap_dc, -xBorder,
                   -captionBorder - yBorder,
                   w, h, display_dc, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
        }
        x += xBorder;
        y += (yBorder + captionBorder);
    } else {
        if (snapMethod == 0) {
            BitBlt(bitmap_dc, 0, 0, w, h, display_dc, x, y, SRCCOPY | CAPTUREBLT);
        } else if (snapMethod == 1) {
            BitBlt(bitmap_dc, 0, 0, w, h, display_dc, r.left + x, r.top + y, SRCCOPY | CAPTUREBLT);
        }
    }

    if(includeCursor){
        CURSORINFO ci;
        ICONINFO iconInf;
        ci.cbSize = sizeof(CURSORINFO);
        GetCursorInfo(&ci);
        GetIconInfo(ci.hCursor, &iconInf);
        if(type == CursorSnap){
            RECT rect = {0, 0, w, h};
            FillRect(bitmap_dc, &rect, CreateSolidBrush(RGB(255, 255, 255)));
            DrawIcon(bitmap_dc, 0, 0, ci.hCursor);
        } else {
            DrawIcon(bitmap_dc, ci.ptScreenPos.x - x - r.left - iconInf.xHotspot, ci.ptScreenPos.y - y - r.top - iconInf.yHotspot, ci.hCursor);
        }
    }

    // clean up all but bitmap
    ReleaseDC(winId, display_dc);
    SelectObject(bitmap_dc, null_bitmap);
    DeleteDC(bitmap_dc);

    QPixmap pixmap = QtWin::fromHBITMAP(bitmap);

    DeleteObject(bitmap);

    return pixmap;
}
