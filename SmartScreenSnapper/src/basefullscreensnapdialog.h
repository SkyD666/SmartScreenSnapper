#ifndef BASEFULLSCREENSNAPDIALOG_H
#define BASEFULLSCREENSNAPDIALOG_H

#include <QDialog>

class BaseFullScreenSnapDialog : public QDialog {
    Q_OBJECT
public:
    explicit BaseFullScreenSnapDialog(QWidget* parent = nullptr);

protected:
    void doAfterSetupUi();

protected:
    QPixmap fullScreenPixmap;
};

#endif // BASEFULLSCREENSNAPDIALOG_H
