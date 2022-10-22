#include "gifdialog.h"
#include "ui_gifdialog.h"
#include <QPainter>
#include <QScreen>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QRandomGenerator>
#include <QProcess>
#include <QDebug>
#include <windows.h>
#include <ctime>
#include "publicdata.h"

GIFDialog::GIFDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GIFDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    maxWindowSize = maximumSize();

    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    HWND h = (HWND)winId();
    SetWindowPos(h , HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);        //置顶窗体

    //    lastRect = rect();
    //    lastRect.setWidth(lastRect.width() - ui->scrollAreaControlBar->width());
    ui->labelRecordScreen->setStyleSheet("background-color: #00FF00");

    //透明背景
    SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) | WS_EX_LAYERED);

    SetLayeredWindowAttributes(h, RGB(0, 255, 0), 100, LWA_COLORKEY);             //指定透明的颜色


    recordGif = new RecordGif;
}

GIFDialog::~GIFDialog()
{
    delete recordGif;

    delete ui;
}

void GIFDialog::closeEvent(QCloseEvent*)
{
    if (recordGif->isRunning()) {
        ui->pushButtonStart->setText(tr("录制"));
        ui->spinBoxFPS->setEnabled(true);
        recordGif->quitThread();
        recordGif->gifEnd();
    }
}

void GIFDialog::on_pushButtonStart_clicked()
{
    if (recordGif->isRunning()) {
        timer.stop();
        ui->pushButtonStart->setText(tr("录制"));
        ui->spinBoxFPS->setEnabled(true);
        recordGif->quitThread();
        recordGif->gifEnd();
        //录制结束，解锁大小
        setMinimumSize(0, 0);
        setMaximumSize(maxWindowSize.width(), maxWindowSize.height());
    } else {
        if (PublicData::gifSavePath == "") {
            QMessageBox::critical(this, tr("警告"), tr("请设置保存文件路径."), QMessageBox::Ok);
            return;
        }
        ui->pushButtonStart->setText(tr("停止"));
        delay = ui->spinBoxFPS->value();    //fps
        ui->spinBoxFPS->setEnabled(false);
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=](){
            QImage frame = QGuiApplication::primaryScreen()->
                    grabWindow(0, geometry().x(), geometry().y(), gifWidth, gifHeight).toImage()
                    .convertToFormat(QImage::Format_RGBA8888);
            std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
                    std::chrono::system_clock::now().time_since_epoch()
                );
            long deltaMs = (1000 - (ms.count() - lastCaptureTime.count())) / delay;
            recordGif->gifWriteFrame(frame, gifWidth, gifHeight, static_cast<quint32>(deltaMs / 10.0 + 0.5));
            timer->setInterval(deltaMs);
            //qDebug() <<  static_cast<quint32>(deltaMs / 10.0 + 0.5);
            lastCaptureTime = ms;
        });

        timer->start(1000 / delay);
        lastCaptureTime = std::chrono::duration_cast< std::chrono::milliseconds >(
                    std::chrono::system_clock::now().time_since_epoch()
                );
        gifWidth = rect().width() - ui->scrollAreaControlBar->width();
        gifHeight = rect().height();
        //录制开始，锁定大小
        setMinimumSize(width(), height());
        setMaximumSize(width(), height());
        QString path = PublicData::gifSavePath;
        recordGif->startThread();
        recordGif->gifBegin(path.replace(".gif", "_" + QString::number(QRandomGenerator::global()->bounded(100000, 999999)) + ".gif"), gifWidth, gifHeight, 1);
    }
}

void GIFDialog::on_pushButtonPath_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("选择文件"),
                                                PublicData::gifSavePath,
                                                tr("gif图片(*.gif)"));
    if (path != "") {
        PublicData::gifSavePath = path;
    }
}

void GIFDialog::on_pushButtonOpenPath_clicked()
{
    if (PublicData::gifSavePath == "") {
        QMessageBox::critical(this, tr("警告"), tr("请设置保存文件路径."), QMessageBox::Ok);
        return;
    }
    QString path = PublicData::gifSavePath.left(PublicData::gifSavePath.lastIndexOf("/") + 1);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
