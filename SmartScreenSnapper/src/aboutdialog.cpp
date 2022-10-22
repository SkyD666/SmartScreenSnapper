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
}


AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_pushButtonOk_clicked()
{
    close();
}

//此函数目前未使用
void AboutDialog::updateQLabelData(QLabel *label)
{
    QString newText = label->text();

    QFontMetrics font(this->font());
    int fontSize = font.horizontalAdvance(newText);
    //int resizeWidth(width);
    if(fontSize > label->width())
    {
        newText = font.elidedText(label->text(), Qt::ElideRight, label->width());//返回一个带有省略号的字符串
    }

    label->setText(newText);
}

void AboutDialog::on_pushButtonSystemInfo_clicked()
{
    if (!QProcess::startDetached("MSINFO32.EXE"))
        QMessageBox::warning(this, tr("警告"), tr("系统信息调用失败！"));
}
