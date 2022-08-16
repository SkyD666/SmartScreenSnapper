#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

private slots:
    void on_pushButtonDeleteHotKey_clicked();

    void on_toolButtonAutoSavePath_clicked();

    void on_checkBoxRunWithWindows_stateChanged(int arg1);

    void on_toolButtonQssPath_clicked();

    void on_pushButton_clicked();

private:
    Ui::SettingDialog *ui;

    QList<QListWidgetItem*> listItem;

    void readSettings();
};

#endif // SETTINGDIALOG_H
