#ifndef FREESNAPDIALOG_H
#define FREESNAPDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>

#include <src/freesnap/freesnapscene.h>
#include <src/freesnap/freesnapgraphicsview.h>

#define GRAYCOLOR grayColor
#define RECTCOLOR rectColor
#define RECTPOINTCOLOR rectPointColor
#define RECTPENWIDTH 2
#define RECTPOINTPENWIDTH 6
#define RECTPOINTPENHEIGHT 6

namespace Ui {
class FreeSnapDialog;
}

class FreeSnapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FreeSnapDialog(QPixmap picture, QPixmap*& result, QWidget *parent = nullptr);
    ~FreeSnapDialog();

private:
    Ui::FreeSnapDialog *ui;

    QColor grayColor;

    QGraphicsItem* grayItem;

    float previewZoomRate;

    FreeSnapScene* scene;

    QPixmap picture;

    QImage image;

    QColor* pixelColor;

    QPixmap*& resultRef;

    int rectLineWidth;
    int pointRadius;

    int deltaHeight;
    int deltaWidth;

    bool hasRect;
    bool mousePressed;
    QPointF pressedPos;

    QPointF lastPos;

    bool pressedInRectArea;
    bool pressedInLeftTopArea;
    bool pressedInRightTopArea;
    bool pressedInLeftBottomArea;
    bool pressedInRightBottomArea;
    bool pressedInLeftArea;
    bool pressedInRightArea;
    bool pressedInTopArea;
    bool pressedInBottomArea;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    void refreshPointPos();         // 更新各个点位置
    QRect checkRectVaild(QRect r); // 调整Rect的位置确保在屏幕内
    void setRectVisible(bool visible);

    void refreshGrayArea();         // 更新灰色区域
    void refreshPreviewArea(QPoint mousePos);      // 更新预览区域

};

#endif // FREESNAPDIALOG_H
