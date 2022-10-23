#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include "updateutil.h"

namespace Ui {
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr, GitHubRelease *githubRelease = nullptr);
    ~UpdateDialog();

public slots:
    void UpdateVersionInfo();

private:
    Ui::UpdateDialog *ui;

    QNetworkAccessManager *manager;

    int fileSize;

    GitHubRelease *githubRelease;

};

#endif // UPDATEDIALOG_H
