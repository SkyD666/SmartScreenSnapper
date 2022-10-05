#ifndef FREESNAPRECT_H
#define FREESNAPRECT_H

#include <QFrame>

class FreeSnapRect : public QFrame
{
    Q_OBJECT

signals:
    void mouseMove(QPoint p);

public:
    FreeSnapRect(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // FREESNAPRECT_H
