#include <QPainter>
#include <ctime>
#include <QThread>
#include <windows.h>
#include <QDebug>
#include <wingdi.h>
#include "star.h"

QWidget* Star::label;
HDC Star::labelHdc;
QPalette Star::labelPalette;

QWidget* StarThread::label;
HDC StarThread::labelHdc;

QString StarThread::text;

LPCWSTR stringToLPCWSTR(std::string orig);

Star::Star()
{
    qsrand(qrand());
    qsrand(qrand());

    labelPalette.setColor(QPalette::Background, QColor("black"));
    label->setAutoFillBackground(true);
    label->setPalette(labelPalette);

    x = 0;
    y = qrand() % label->height();//产生0-480的的坐标
    step = qrand() % 1000 + 500;
    color = RGB(255, 255, 255);
}

StarThread::~StarThread()
{
}

void Star::move()
{
    qsrand(qrand());
    qsrand(qrand());

    SetPixel( labelHdc, x, y, RGB(0, 0, 0) );
    SetPixel( labelHdc, x + 1, y, RGB(0, 0, 0) );
    x += step;

    if (x > label->width() || y > label->height() || x < -31 || y < 0 || step <=0 ) {
        x = -30;
        y = qrand() % label->height();  //产生0-480的的坐标
        qsrand(qrand() % 999999);
        step = qrand() % 30;
        color = RGB(qrand() % 255, qrand() % 255, qrand() % 255);
    }
    SetPixel( labelHdc, x, y, color );
    SetPixel( labelHdc, x + 1, y, color );
}

StarThread::StarThread()
{

}

void StarThread::play()
{
    qsrand(time(NULL));

    TEXTMETRIC tm;
    GetTextMetrics(StarThread::labelHdc, &tm);
    SetBkColor(StarThread::labelHdc, RGB(0, 0, 0));
    SetTextColor(StarThread::labelHdc, RGB(255, 255, 0));
    HFONT hFont;
    LOGFONT lFont;
    GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT),&lFont);
    lFont.lfHeight = GetDeviceCaps(StarThread::labelHdc, LOGPIXELSY) * tm.tmHeight / 72;
    lFont.lfWidth = GetDeviceCaps(StarThread::labelHdc, LOGPIXELSX) * tm.tmAveCharWidth / 72;
    lFont.lfEscapement = 0;
    lFont.lfCharSet = tm.tmCharSet;
    wcscpy_s(lFont.lfFaceName, L"黑体");
    hFont = CreateFontIndirect(&lFont);
    SelectObject(StarThread::labelHdc, hFont);

    SIZE size;
    GetTextExtentPoint(StarThread::labelHdc, StarThread::text.toStdWString().c_str(),  StarThread::text.length(), &size);
    QString txt = StarThread::text;
    long txtHetght = (size.cy) * (StarThread::text.count() - txt.remove("\n").count());
    RECT textRect = {0, StarThread::label->height(), StarThread::label->width(), StarThread::label->height() + txtHetght};

    qDebug() << StarThread::label->height();

    //int i = 0;
    while (!QThread::currentThread()->isInterruptionRequested())
    {
        if(textRect.bottom < 0) OffsetRect(&textRect, 0, StarThread::label->height() + txtHetght);

        for (int i = 0; i < MAXSTAR; i++) {
            star[i].move();
        }

        textRect.right = StarThread::label->width();

        SetTextColor(StarThread::labelHdc, RGB(0, 0, 0));
        DrawText(StarThread::labelHdc, StarThread::text.toStdWString().c_str(), -1, &textRect, DT_CENTER);
        OffsetRect(&textRect, 0, -1/*(i == 0 ? i = 1 : i = 0)*/);
        SetTextColor(StarThread::labelHdc, RGB(255, 255, 0));
        DrawText(StarThread::labelHdc, StarThread::text.toStdWString().c_str(), -1, &textRect, DT_CENTER);

        Sleep(30);
    }
}
