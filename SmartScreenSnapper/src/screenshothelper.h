#ifndef SCREENSHOTHELPER_H
#define SCREENSHOTHELPER_H

#include <QPixmap>

class ScreenShotHelper
{
public:

    enum ShotType {
        ScreenShot, ActiveWindowShot, CursorShot, FreeShot, FreeHandShot, LongShot, Count
    };

    ScreenShotHelper();

    static QPixmap grabWindow(int snapMethod, HWND winId, int type, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

    static QPixmap grabCursor();

    static QPixmap screenshot(ShotType shotType, bool isHotKey);

    static bool savePicture(QString filePath, QPixmap pixmap);

    static QPixmap getWindowPixmap(HWND winId, ShotType shotType, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

    static QString getPictureName(ShotType shotType);

    static QPixmap getFullScreen();
};

#endif // SCREENSHOTHELPER_H
