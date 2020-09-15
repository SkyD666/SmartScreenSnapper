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

QString WindowsInfo::getWindowsVersion() {
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if (NULL != fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    QString sysBit = "unknown";
    if(bIsWow64)
        sysBit = "64位";
    else
        sysBit = "32位";

    return QSysInfo::prettyProductName() + " " + sysBit;
}

QString WindowsInfo::getCPU() {
    QSettings CPU("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",QSettings::NativeFormat);
    return CPU.value("ProcessorNameString").toString();
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
