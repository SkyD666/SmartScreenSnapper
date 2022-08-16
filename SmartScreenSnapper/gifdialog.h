#ifndef GIFDIALOG_H
#define GIFDIALOG_H

#include <QDialog>
#include <QPaintEvent>
#include <QThread>
#include <QTimer>
#include <QCloseEvent>
#include <QSize>
#include "gif-h/gif.h"

namespace Ui {
class GIFDialog;
}

bool GifBegin( GifWriter* writer, const char* filename, uint32_t width, uint32_t height, uint32_t delay, int32_t bitDepth = 8, bool dither = false );
bool GifWriteFrame( GifWriter* writer, const uint8_t* image, uint32_t width, uint32_t height, uint32_t delay, int bitDepth = 8, bool dither = false );
bool GifEnd( GifWriter* writer );

class RecordGif : public QObject
{
    Q_OBJECT
public:
    explicit RecordGif()
    {
        gifWriter = new GifWriter;
        moveToThread(&recordThread);
    }

    ~RecordGif()
    {
        if (recordThread.isRunning()) {
            recordThread.quit();
            recordThread.wait();
        }
    }

    bool isRunning() {
        return recordThread.isRunning();
    }

    void quitThread() {
        recordThread.quit();
    }

    void startThread() {
        recordThread.start();
    }
public slots:
    void gifBegin(const QString &file, int width, int height, int delay)
    {
        GifBegin(gifWriter, file.toLocal8Bit().data(), width, height, delay);
    }
    void gifWriteFrame(const QImage &img, int width, int height, uint32_t delay)
    {
        //单位是1/100秒钟 即100表示一秒
        GifWriteFrame(gifWriter, img.bits(), qMin(width, img.width()), qMin(height, img.height()), delay);
    }
    void gifEnd()
    {
        GifEnd(gifWriter);
        recordThread.quit();
    }

private:
    GifWriter *gifWriter;

    QThread recordThread;
};

class GIFDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GIFDialog(QWidget *parent = nullptr);
    ~GIFDialog();
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_pushButtonStart_clicked();

    void on_pushButtonPath_clicked();

    void on_pushButtonOpenPath_clicked();

private:
    Ui::GIFDialog *ui;

    RecordGif *recordGif;

    QTimer timer;

    int delay;

    int gifWidth;

    int gifHeight;

    std::chrono::milliseconds lastCaptureTime;

    QSize maxWindowSize;

    QRect lastRect;
};

#endif // GIFDIALOG_H
