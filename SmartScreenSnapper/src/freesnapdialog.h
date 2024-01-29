#ifndef FREESNAPDIALOG_H
#define FREESNAPDIALOG_H

#include "src/basefullscreensnapdialog.h"
#include <QDialog>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QRect>

#include <src/freesnap/freesnapgraphicsview.h>

namespace Ui {
class FreeSnapDialog;
}

class FreeSnapDialog : public BaseFullScreenSnapDialog {
    Q_OBJECT

public:
    explicit FreeSnapDialog(QPixmap* result, bool& captured, QWidget* parent = nullptr);
    ~FreeSnapDialog();

private:
    Ui::FreeSnapDialog* ui;

    QColor grayColor;

    QGraphicsItem* grayItem;

    float previewZoomRate;

    QGraphicsScene* scene;

    QImage image;

    QColor* pixelColor;

    QPixmap* resultPixmap;

    bool& captured;

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
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void refreshPointPos(); // 更新各个点位置
    QRect checkRectVaild(QRect r); // 调整Rect的位置确保在屏幕内
    void setRectVisible(bool visible);

    void refreshGrayArea(); // 更新灰色区域
    void refreshPreviewArea(QPoint mousePos); // 更新预览区域

    void captureAndClose(); // 截图并关闭窗体
};

#endif // FREESNAPDIALOG_H
