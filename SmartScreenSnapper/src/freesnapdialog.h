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

    QColor grayColor = QColor(0, 0, 0, 180);

    QGraphicsItem* grayItem = nullptr;

    float previewZoomRate = 3.2f;

    QGraphicsScene* scene = new QGraphicsScene(this);

    QImage image;

    QColor pixelColor = QColor();

    QPixmap* resultPixmap = nullptr;

    bool& captured;

    int rectLineWidth = 1;
    int pointRadius = 9;

    int deltaHeight = 0;
    int deltaWidth = 0;

    bool hasRect = false;
    bool mousePressed = false;
    QPointF pressedPos;

    QPointF lastPos;

    bool pressedInRectArea = false;
    bool pressedInLeftTopArea = false;
    bool pressedInRightTopArea = false;
    bool pressedInLeftBottomArea = false;
    bool pressedInRightBottomArea = false;
    bool pressedInLeftArea = false;
    bool pressedInRightArea = false;
    bool pressedInTopArea = false;
    bool pressedInBottomArea = false;

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
