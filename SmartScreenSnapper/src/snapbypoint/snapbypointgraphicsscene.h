#ifndef SNAPBYPOINTGRAPHICSVIEW_H
#define SNAPBYPOINTGRAPHICSVIEW_H

#include <QGraphicsRectItem>
#include <QGraphicsView>

class SnapByPointGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

signals:
    void captured(QRectF);

    void rightButtonClicked();

public:
    SnapByPointGraphicsScene(QWidget *parent = nullptr);

    ~SnapByPointGraphicsScene();


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:

    QGraphicsRectItem* rectItem;

    QList<QPair<HWND, QRect>> windowRectList;
};

#endif // SNAPBYPOINTGRAPHICSVIEW_H
