#ifndef SNAPFROMPOINTDIALOG_H
#define SNAPFROMPOINTDIALOG_H

#include <QDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

#include "snapbypoint/snapbypointgraphicsscene.h"

namespace Ui {
class SnapFromPointDialog;
}

class SnapByPointDialog : public QDialog
{
    Q_OBJECT

signals:
    void captured(QPixmap);

public:
    explicit SnapByPointDialog(QWidget *parent = nullptr);
    ~SnapByPointDialog();

    SnapByPointGraphicsScene *scene;

    QPixmap fullScreenPixmap;

private:
    Ui::SnapFromPointDialog *ui;

};

#endif // SNAPFROMPOINTDIALOG_H
