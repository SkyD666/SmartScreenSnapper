#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <windows.h>
#include <QThread>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QFile>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("关于") + " " + QApplication::applicationName());

    QPixmap icon = QPixmap(":/image/icon.png");

    icon.scaled(ui->labelIcon->size(), Qt::KeepAspectRatio);
    ui->labelIcon->setScaledContents(true);
    ui->labelIcon->setPixmap(icon);
    ui->lineEditAppName->setText(QApplication::applicationName());
    ui->lineEditAppVersion->setText(tr("版本: ") + QApplication::applicationVersion());
    ui->lineEditSystemInfo->setText(QSysInfo::prettyProductName());

    QFile license(":/license/LICENSE");
    if (license.open(QFile::ReadOnly)) {
        ui->textEditLicense->setText(QTextStream(&license).readAll());
    }
    connect(ui->pushButtonOk, &QPushButton::clicked, this, [this](){ close(); });
}


AboutDialog::~AboutDialog()
{
    delete ui;
}
