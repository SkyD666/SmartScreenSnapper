#ifndef SCREENSHOTDISPLAYDIALOG_H
#define SCREENSHOTDISPLAYDIALOG_H

#include <QDialog>
#include <QGraphicsScene>

namespace Ui {
class ScreenshotDisplayDialog;
}

class ScreenshotDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenshotDisplayDialog(QPixmap pixmap);
    ~ScreenshotDisplayDialog();

private:
    Ui::ScreenshotDisplayDialog *ui;

    QGraphicsScene *scene;

    QPixmap pixmap;

    QPointF mousePressedPos;
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // SCREENSHOTDISPLAYDIALOG_H
