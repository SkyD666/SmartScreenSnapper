#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QByteArray>
#include <QFile>
#include <QDesktopServices>
#include <QNetworkReply>
#include "updateutil.h"

UpdateDialog::UpdateDialog(QWidget *parent, GitHubRelease *githubRelease) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    fileSize(0),
    githubRelease(githubRelease)
{
    ui->setupUi(this);

    ui->labelCurrentVersion->setText(tr("当前版本：") + QApplication::applicationVersion());

    connect(ui->pushButtonDownloadManual, &QPushButton::clicked, this, [githubRelease](){
        QDesktopServices::openUrl(QUrl(githubRelease->htmlUrl));
    });

    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply) {
        QByteArray data = reply->readAll();
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (code == 200) {
            this->githubRelease = UpdateUtil::getData(this, data);
            UpdateVersionInfo();
        } else {
            ui->labelTips->setText(tr("获取更新信息失败！\nCode: ") +
                                   QString::number(code) + "\n" + QString(data));
            QMessageBox::information(this, tr("警告"), tr("获取更新信息失败！"), QMessageBox::Ok);
        }
        reply->deleteLater();
    });
    if (!githubRelease) {
        ui->labelChangeLog->setVisible(false);
        ui->textEditChangeLog->setVisible(false);
        ui->pushButtonDownloadManual->setVisible(false);
        ui->labelTips->setText(tr("正在检查更新，请稍候..."));
        // 繁忙状态
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
        UpdateUtil::checkUpdate(manager);
    } else {
        UpdateVersionInfo();
    }
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::UpdateVersionInfo() {
    if (!githubRelease) return;

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(100);

    if (UpdateUtil::isNewVersion(githubRelease->name)) {
        ui->labelChangeLog->setVisible(true);
        ui->textEditChangeLog->setVisible(true);
        ui->pushButtonDownloadManual->setVisible(true);
        ui->labelTips->setText(tr("发现新版本：") + githubRelease->name);
        ui->textEditChangeLog->setText(githubRelease->body);
    } else {
        ui->labelTips->setText(tr("当前已是最新版本！"));
    }
}
