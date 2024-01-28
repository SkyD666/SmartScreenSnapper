#ifndef SNAPFROMPOINTDIALOG_H
#define SNAPFROMPOINTDIALOG_H

#include <QDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

#include "snapbypoint/snapbypointgraphicsscene.h"
#include "src/basefullscreensnapdialog.h"

namespace Ui {
class SnapFromPointDialog;
}

class SnapByPointDialog : public BaseFullScreenSnapDialog {
    Q_OBJECT

signals:
    void captured(QPixmap);

public:
    explicit SnapByPointDialog(QWidget* parent = nullptr);
    ~SnapByPointDialog();

    SnapByPointGraphicsScene* scene;

private:
    Ui::SnapFromPointDialog* ui;
};

#endif // SNAPFROMPOINTDIALOG_H
