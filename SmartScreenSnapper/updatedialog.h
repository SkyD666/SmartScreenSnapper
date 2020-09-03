#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <windows.h>
#include <wininet.h>

struct Respond{
    int code;
    QString json;
};

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    static bool isNewVersion(QString version);

    void closeEvent(QCloseEvent* event);

public slots:
    void UpdateVersionInfo(Respond respond);

    void showDownloadProgress(long value);
private slots:
    void on_pushButtonDownloadManual_clicked();

    void on_pushButtonDownload_clicked();

private:
    Ui::UpdateDialog *ui;

    int fileSize;

};

class UpdateThread : public QObject
{
    Q_OBJECT

signals:
    void checkFinished(Respond);

public:
    UpdateThread();

    ~UpdateThread();
public slots:
    void checkUpdate();

private:
};

class DownloadFileThread : public QObject
{
    Q_OBJECT

signals:
    void downloadProgress(long);

public:
    DownloadFileThread();

    ~DownloadFileThread();
public slots:
    bool getFile();
private:
};

#endif // UPDATEDIALOG_H
