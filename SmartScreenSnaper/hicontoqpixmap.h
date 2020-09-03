#ifndef HICONTOQPIXMAP_H
#define HICONTOQPIXMAP_H

#include <QPixmap>

class HICONToQPixmap
{
public:
    HICONToQPixmap();

    void *qMalloc(size_t size);

    void qFree(void *ptr);

    QImage qt_fromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);

    QPixmap fromWinHICON(HICON icon);
};

#endif // HICONTOQPIXMAP_H
