#ifndef LONGSNAPDIALOG_H
#define LONGSNAPDIALOG_H

#include <QDialog>

namespace Ui {
class LongSnapDialog;
}

class LongSnapDialog : public QDialog
{
    Q_OBJECT
signals:
    void captured(QPixmap);
public:
    explicit LongSnapDialog(QWidget *parent = nullptr);
    ~LongSnapDialog();

private:
    Ui::LongSnapDialog *ui;

    QPoint* firstPoint;
    QPoint* secondPoint;
    QPoint* clickPoint;
    QRect* region;

    int currentStep;    // 0等待firstPoint，1等待secondPoint，2等待clickPoint

    QPixmap getFullScreen();

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void refreshRectPos(QPoint p1, QPoint p2);
    QPixmap capture();
};

#endif // LONGSNAPDIALOG_H
