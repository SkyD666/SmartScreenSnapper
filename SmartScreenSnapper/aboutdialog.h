#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_pushButtonOk_clicked();

    void on_checkBoxMusic_toggled(bool checked);

    void on_pushButtonSystemInfo_clicked();

private:
    Ui::AboutDialog *ui;

    void updateQLabelData(QLabel *label);
};

#endif // ABOUTDIALOG_H
