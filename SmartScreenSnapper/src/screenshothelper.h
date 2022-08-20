#ifndef SCREENSHOTHELPER_H
#define SCREENSHOTHELPER_H

#include <QPixmap>

class ScreenShotHelper
{
public:
    ScreenShotHelper();

    static QPixmap grabWindow(int snapMethod, HWND winId, int type, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

};

#endif // SCREENSHOTHELPER_H
