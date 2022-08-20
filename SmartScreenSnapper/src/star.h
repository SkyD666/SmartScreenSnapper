#ifndef STAR_H
#define STAR_H

#include <QLabel>
#include <QThread>
#include <random>

#define MAXSTAR 300          // 星星总数

class  Star
{
private:
    int x;
    int y;
    int color;
    double step;
public:
    Star();
    void move(void);

    static QWidget* label;

    static HDC labelHdc;

    static QPalette labelPalette;
};

class StarThread : public QObject
{
    Q_OBJECT
public:
    StarThread();

    ~StarThread();

    static QWidget* label;

    static HDC labelHdc;

    static QString text;
public slots:
    void play();

private:
    Star star[MAXSTAR];
};

#endif // STAR_H
