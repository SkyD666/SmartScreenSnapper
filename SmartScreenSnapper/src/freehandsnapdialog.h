#ifndef FREEHANDSNAPDIALOG_H
#define FREEHANDSNAPDIALOG_H

#include <QDialog>
#include <QGraphicsScene>

#include <src/freehandsnap/freehandsnapgraphicsscene.h>

namespace Ui {
class FreeHandSnapDialog;
}

class FreeHandSnapDialog : public QDialog
{
    Q_OBJECT

signals:
    void captured(QPixmap);

public:
    explicit FreeHandSnapDialog(QWidget *parent = nullptr);
    ~FreeHandSnapDialog();

private:
    Ui::FreeHandSnapDialog *ui;

    FreeHandSnapGraphicsScene *scene;

    QPixmap fullScreenPixmap;

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // FREEHANDSNAPDIALOG_H
