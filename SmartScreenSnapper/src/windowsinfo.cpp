#include "windowsinfo.h"

WindowsInfo::WindowsInfo()
{

}

unsigned long long WindowsInfo::getMemorySize() {
    MEMORYSTATUSEX memoryStatusEx;
    memoryStatusEx.dwLength = sizeof (memoryStatusEx);
    GlobalMemoryStatusEx(&memoryStatusEx);
    return memoryStatusEx.ullTotalPhys;
}

unsigned long long WindowsInfo::getAvailMemorySize() {
    MEMORYSTATUSEX memoryStatusEx;
    memoryStatusEx.dwLength = sizeof (memoryStatusEx);
    GlobalMemoryStatusEx(&memoryStatusEx);
    return memoryStatusEx.ullAvailPhys;
}

QString WindowsInfo::getCPU() {
    QSettings CPU("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",QSettings::NativeFormat);
    return CPU.value("ProcessorNameString").toString();
}

int WindowsInfo::getShadowSize(HWND hwnd) {
    if (IsZoomed (hwnd)) return 0;

    // 传入一个高和宽都是0的窗体大小，就可以获得两倍的shadow大小
    RECT rc = {0, 0, 0, 0};
    AdjustWindowRectEx (&rc, WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, FALSE, WS_EX_WINDOWEDGE);

    return (rc.right - rc.left);
}

void WindowsInfo::getWindowBorderSize(HWND hwnd, long* xBorder, long* yBorder, long* captionBorder) {
    long windowStyle = GetWindowLong(hwnd, GWL_STYLE);
    long windowExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    (*captionBorder) = 0;
    (*xBorder) = 0;
    (*yBorder) = 0;

    if (windowExStyle & WS_EX_TOOLWINDOW) {
        if (windowStyle & WS_CAPTION) {                //有标题栏和边框
            (*captionBorder) = GetSystemMetrics(SM_CYSMCAPTION);
            if (windowStyle & WS_THICKFRAME) {      //尺寸可变
                (*xBorder) = GetSystemMetrics(SM_CXFRAME);
                (*yBorder) = GetSystemMetrics(SM_CYFRAME);
            } else {
                (*xBorder) = GetSystemMetrics(SM_CXBORDER);
                (*yBorder) = GetSystemMetrics(SM_CYBORDER);
            }
            return;
        } else {
            (*captionBorder) = 0;
            (*xBorder) = 0;
            (*yBorder) = 0;
            return;
        }
    } else {
        if (windowStyle & WS_CAPTION) {                //有标题栏和边框
            (*captionBorder) = GetSystemMetrics(SM_CYCAPTION);
            if (windowStyle & WS_THICKFRAME) {      //尺寸可变
                (*xBorder) = GetSystemMetrics(SM_CXFRAME);
                (*yBorder) = GetSystemMetrics(SM_CYFRAME);
            } else {
                (*xBorder) = GetSystemMetrics(SM_CXBORDER);
                (*yBorder) = GetSystemMetrics(SM_CYBORDER);
            }
            return;
        } else {
            (*captionBorder) = 0;
            (*xBorder) = 0;
            (*yBorder) = 0;
            return;
        }
    }
}
