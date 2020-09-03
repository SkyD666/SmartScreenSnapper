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
