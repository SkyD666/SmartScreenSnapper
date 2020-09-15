#ifndef WINDOWSINFO_H
#define WINDOWSINFO_H

#include <windows.h>
#include <QString>
#include <QSettings>

class WindowsInfo
{
public:
    WindowsInfo();

    static unsigned long long getMemorySize();

    static unsigned long long getAvailMemorySize();

    static QString getWindowsVersion();

    static QString getCPU();

    static QString getGPU();

    static void getWindowBorderSize(HWND hwnd, long* xBorder, long* yBorder, long* captionBorder);
};

#endif // WINDOWSINFO_H
