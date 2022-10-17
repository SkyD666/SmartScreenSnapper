#ifndef SCREENSHOTHELPER_H
#define SCREENSHOTHELPER_H

#include <QPixmap>

class ScreenShotHelper
{
public:

    enum ShotType {
        ScreenShot, ActiveWindowShot, CursorShot, FreeShot, FreeHandShot, ShotByPoint, LongShot, Count
    };

    ScreenShotHelper();

    static QString lastProcessName;
    static unsigned long lastPId;

    static QPixmap grabWindow(int snapMethod, HWND hwnd, int type, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

    static QPixmap grabCursor();

    static QPixmap grabByHdc();

    static QPixmap screenshot(ShotType shotType, bool isHotKey);

    static bool savePicture(QWidget *msgBoxParent, QString filePath, QPixmap pixmap);

    static QPixmap getWindowPixmap(HWND winId, ShotType shotType, bool includeCursor = false, int x = 0, int y = 0, int w = -1, int h = -1);

    static QString getPictureName(ShotType shotType);

    static QString getSnapTypeName(ShotType shotType);

    static QPixmap getFullScreen();
};

#endif // SCREENSHOTHELPER_H
