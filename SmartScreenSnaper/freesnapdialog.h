#ifndef FREESNAPDIALOG_H
#define FREESNAPDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>

#define GRAYCOLOR QColor(0, 0, 0, 180)
#define RECTCOLOR QColor(7, 200, 250)
#define RECTPOINTCOLOR QColor(200, 240, 255)
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

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyReleaseEvent(QKeyEvent *ev);
    void keyPressEvent(QKeyEvent *ev);

    bool inLeftTopPoint(QMouseEvent *event);
    bool inRightTopPoint(QMouseEvent *event);
    bool inLeftBottomPoint(QMouseEvent *event);
    bool inRightBottomPoint(QMouseEvent *event);

private:
    Ui::FreeSnapDialog *ui;

    QPixmap picture;

    QPixmap*& resultRef;

    QPixmap pictureMap;

    QPixmap pictureMap2;

    QPoint lastPoint;

    QPoint endPoint;

    bool isDrawing;

    bool noOperate;

    QRect resultRect;
};

#endif // FREESNAPDIALOG_H
