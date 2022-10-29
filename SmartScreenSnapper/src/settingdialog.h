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
private:
    Ui::SettingDialog *ui;

    QList<QListWidgetItem*> listItem;

    void initConnect();

    void readAndInitSettings();

    void runWithWindows(bool enable);
};

#endif // SETTINGDIALOG_H
