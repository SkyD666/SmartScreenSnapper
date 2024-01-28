#ifndef FREEHANDSNAPDIALOG_H
#define FREEHANDSNAPDIALOG_H

#include "src/basefullscreensnapdialog.h"
#include <QDialog>
#include <QGraphicsScene>

#include <src/freehandsnap/freehandsnapgraphicsscene.h>

namespace Ui {
class FreeHandSnapDialog;
}

class FreeHandSnapDialog : public BaseFullScreenSnapDialog {
    Q_OBJECT

signals:
    void captured(QPixmap);

public:
    explicit FreeHandSnapDialog(QWidget* parent = nullptr);
    ~FreeHandSnapDialog();

private:
    Ui::FreeHandSnapDialog* ui;

    FreeHandSnapGraphicsScene* scene;

protected:
    void keyPressEvent(QKeyEvent* event);
};

#endif // FREEHANDSNAPDIALOG_H
