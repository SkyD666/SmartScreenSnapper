#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "minifmod.h"
#include "windowsinfo.h"
#include "star.h"
#include <windows.h>
#include <QThread>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

void playMusic();
void stopPlayMusic();
void loadMusic();
void freeSong();
void drawStar(QWidget *parent, QWidget * label, QWidget * labelText);
FMUSIC_MODULE *mod;         //xm音乐
QThread *myThread;
StarThread *starObject;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    loadMusic();

    this->setWindowTitle(tr("关于") + " " + QApplication::applicationName());

    QPixmap icon = QPixmap(":/images/icon.png");

    icon.scaled(ui->labelIcon->size(), Qt::KeepAspectRatio);
    ui->labelIcon->setScaledContents(true);
    ui->labelIcon->setPixmap(icon);
    ui->lineEditAppName->setText(QApplication::applicationName());
    ui->lineEditAppVersion->setText(tr("版本: ") + QApplication::applicationVersion());
    ui->lineEditSystemInfo->setText(WindowsInfo::getWindowsVersion());

    ui->checkBoxMusic->setCheckState(Qt::Checked);

    QTimer::singleShot(0, [=](){     //使控件大小都正确初始化
        drawStar(this, ui->labelStar, ui->labelStar);
    });
}


void AboutDialog::on_checkBoxMusic_toggled(bool checked)
{
    if (checked) {
        playMusic();
    } else {
        stopPlayMusic();
    }
}


AboutDialog::~AboutDialog()
{
    disconnect(myThread, SIGNAL(started()), starObject, SLOT(play()));
    disconnect(myThread, SIGNAL(finished()), starObject, SLOT(deleteLater()));

    myThread->requestInterruption();
    myThread->quit();
    myThread->wait();

    stopPlayMusic();
    freeSong();

    delete starObject;

    delete ui;
}

void AboutDialog::on_pushButtonOk_clicked()
{
    close();
}

void drawStar(QWidget *parent, QWidget * label, QWidget * labelText) {
    Star::label = label;
    Star::labelHdc = GetWindowDC( (HWND)label->winId() );

    StarThread::label = labelText;
    StarThread::labelHdc = GetWindowDC( (HWND)labelText->winId() );
    StarThread::text = QApplication::applicationName() + "\n\nVersion: " + QApplication::applicationVersion() +
            "\n\nA screenshot software\n\nMade by Sky_D\n\nWritten in Qt 5.14.2" + "\n\n\n ";   //带一个空格结尾，防止最后一行闪烁

    myThread = new QThread(parent);
    starObject = new StarThread();

    parent->connect(myThread, SIGNAL(started()), starObject, SLOT(play()));
    parent->connect(myThread, SIGNAL(finished()), starObject, SLOT(deleteLater()));
    starObject->moveToThread(myThread);
    myThread->start();
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

//下面是播放xm相关函数，不要修改
typedef struct
{
    int length;
    int pos;
    void *data;
} MEMFILE;


unsigned int memopen(char *name)
{
    MEMFILE *memfile;

    memfile = (MEMFILE *)calloc(sizeof(MEMFILE),1);
    {
        HRSRC              rec;
        HGLOBAL              handle;

        rec = FindResource(NULL, (LPCTSTR)name, TEXT("MUSIC"));
        handle = LoadResource(NULL, rec);

        memfile->data = LockResource(handle);
        memfile->length = SizeofResource(NULL, rec);
        memfile->pos = 0;
    }

    return (uintptr_t)memfile;
}

void memclose(unsigned int handle)
{
    MEMFILE *memfile = (MEMFILE *)handle;
    free(memfile);
}

int memread(void *buffer, int size, unsigned int handle)
{
    MEMFILE *memfile = (MEMFILE *)handle;

    if (memfile->pos + size >= memfile->length)
        size = memfile->length - memfile->pos;

    memcpy(buffer, (char *)memfile->data+memfile->pos, size);
    memfile->pos += size;

    return size;
}

void memseek(unsigned int handle, int pos, signed char mode)
{
    MEMFILE *memfile = (MEMFILE *)handle;

    if (mode == SEEK_SET)
        memfile->pos = pos;
    else if (mode == SEEK_CUR)
        memfile->pos += pos;
    else if (mode == SEEK_END)
        memfile->pos = memfile->length + pos;

    if (memfile->pos > memfile->length)
        memfile->pos = memfile->length;
}

int memtell(unsigned int handle)
{
    MEMFILE *memfile = (MEMFILE *)handle;

    return memfile->pos;
}

void loadMusic()
{
    FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);//将播放功能设置为内存（资源）音乐播放
    mod = FMUSIC_LoadSong((char *)MAKEINTRESOURCE(0), NULL);                    //打开资源
}


void playMusic()
{
    FMUSIC_PlaySong(mod);       //开始播放
}

void stopPlayMusic()
{
    FMUSIC_StopSong(mod);       //停止播放
}

void freeSong()
{
    FMUSIC_FreeSong(mod);       //释放资源
}

void AboutDialog::on_pushButtonSystemInfo_clicked()
{
    if (!QProcess::startDetached("MSINFO32.EXE"))
        QMessageBox::warning(this, tr("警告"), tr("系统信息调用失败！"));
}
