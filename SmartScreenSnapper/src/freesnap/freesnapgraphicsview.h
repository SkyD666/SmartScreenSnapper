#ifndef FREESNAPGRAPHICSVIEW_H
#define FREESNAPGRAPHICSVIEW_H

#include <QGraphicsView>

class FreeSnapGraphicsView : public QGraphicsView
{
public:
    FreeSnapGraphicsView(QWidget *parent = nullptr);

    void setIgnoreMouseEvent(bool ignore);

private:
    bool ignoreMouseEvent;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // FREESNAPGRAPHICSVIEW_H
