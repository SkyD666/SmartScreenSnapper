#ifndef FREEHANDSNAPGRAPHICSVIEW_H
#define FREEHANDSNAPGRAPHICSVIEW_H

#include <QGraphicsView>

class FreeHandSnapGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    FreeHandSnapGraphicsScene(QWidget *parent = nullptr);

    ~FreeHandSnapGraphicsScene();

    QPolygonF getPathPolygonF();

    QRectF getPathRect();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void refreshPathItem(QPainterPath path);
    void refreshGrayArea();
private:
    bool mousePressed;

    QPainterPath* unsuredPath;
    bool hasNewUnsuredPath;
    QPainterPath* path;
    QGraphicsPathItem* pathItem;
    QGraphicsPolygonItem* grayItem;
    QColor grayColor;
};

#endif // FREEHANDSNAPGRAPHICSVIEW_H
