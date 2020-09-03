#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QMetaType>
#include <QThread>
#include <QJsonObject>
#include <QDateTime>
#include <QIODevice>
#include <QFile>
#include <windows.h>

#define READ_BUFFER_SIZE 2048

QThread *uThread;
UpdateThread *updateThread;
QThread *dThread;
DownloadFileThread *downloadFileThread;
QString downloadUrl;
bool isUpdateNetWorkRequest = false;
struct {
    qint64 lastTime = 0;
    long lastB = 0;
} lastDownloadData;

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    fileSize(0)
{
    ui->setupUi(this);

    ui->labelCurrentVersion->setText(tr("当前版本：") + QApplication::applicationVersion());
    ui->labelChangeLog->setVisible(false);
    ui->textEditChangeLog->setVisible(false);
    ui->labelDownloadSpeed->setVisible(false);
    ui->pushButtonDownload->setVisible(false);
    ui->pushButtonDownloadManual->setVisible(false);
    uThread = new QThread(parent);
    updateThread = new UpdateThread();

    ui->labelTips->setText(tr("正在检查更新，请稍候..."));
    //繁忙状态
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    connect(uThread, SIGNAL(started()), updateThread, SLOT(checkUpdate()));
    connect(uThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()));
    connect(updateThread, SIGNAL(checkFinished(Respond)), this, SLOT(UpdateVersionInfo(Respond)));
    updateThread->moveToThread(uThread);
    uThread->start();
}

void UpdateDialog::closeEvent(QCloseEvent* event) {
    if (isUpdateNetWorkRequest) {
        event->ignore();
        QMessageBox::information(this, tr("警告"), tr("请等待网络请求返回..."), QMessageBox::Ok);
    }
}

UpdateDialog::~UpdateDialog()
{
    disconnect(uThread, SIGNAL(started()), updateThread, SLOT(checkUpdate()));
    disconnect(uThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()));

    uThread->requestInterruption();
    uThread->quit();
    uThread->wait();

    delete updateThread;
    delete uThread;

    if (downloadFileThread) {
        disconnect(dThread, SIGNAL(started()), downloadFileThread, SLOT(getFile()));
        disconnect(dThread, SIGNAL(finished()), downloadFileThread, SLOT(deleteLater()));

        dThread->requestInterruption();
        dThread->quit();
        dThread->wait();

        delete downloadFileThread;
        delete dThread;
    }

    delete ui;
}

void UpdateDialog::UpdateVersionInfo(Respond respond) {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(100);
    if (!respond.code) {
        QJsonParseError jsonParseError;
        QByteArray byteArray = respond.json.toUtf8();//(respond.json.toStdString().c_str(), respond.json.length());
        QJsonDocument jsonDocument = QJsonDocument::fromJson(byteArray, &jsonParseError);
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.contains("tag_name") && jsonObject.contains("body") && jsonObject.contains("assets")) {
            QJsonValue value = jsonObject.take("tag_name");
            QJsonValue bodyValue = jsonObject.take("body");
            QJsonArray assetsArray = jsonObject.take("assets").toArray();
            if (value.isString() && bodyValue.isString()) {
                QString versionInfo = value.toString();
                QString bodyInfo = bodyValue.toString();
                versionInfo.replace("V", "");
                if (isNewVersion(versionInfo)) {
                    QJsonObject assetsValue = assetsArray.takeAt(0).toObject();
                    if (assetsValue.contains("browser_download_url") && assetsValue.contains("size") ) {
                        downloadUrl = assetsValue.take("browser_download_url").toString();
                        fileSize = assetsValue.take("size").toInt();
                    } else {
                        QMessageBox::information(this, tr("警告"), tr("获取版本下载地址失败！"), QMessageBox::Ok);
                        return;
                    }
                    ui->labelChangeLog->setVisible(true);
                    ui->textEditChangeLog->setVisible(true);
                    ui->pushButtonDownloadManual->setVisible(true);
                    ui->pushButtonDownload->setVisible(true);
                    ui->labelTips->setText(tr("发现新版本：") + versionInfo);
                    ui->textEditChangeLog->setText(bodyInfo);
                } else {
                    ui->labelTips->setText(tr("当前已是最新版本！"));
                }
            }
        }
    } else {
        ui->labelTips->setText(tr("获取更新信息失败！\nCode: ") + QString::number(respond.code) + "\n" + respond.json);
        QMessageBox::information(this, tr("警告"), tr("获取更新信息失败！"), QMessageBox::Ok);
    }
}

void UpdateThread::checkUpdate() {
    isUpdateNetWorkRequest = true;
    HINTERNET hSession = InternetOpenA("SmartScreenSnap", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    isUpdateNetWorkRequest = false;
    if (!hSession || QThread::currentThread()->isInterruptionRequested()) {
        emit checkFinished(Respond {-1, "InternetOpenA Failed."});
        InternetCloseHandle(hSession);
        return;
    }
    isUpdateNetWorkRequest = true;
    HINTERNET hConnection = InternetConnectA(hSession, "api.github.com",//指定要连接的主机
                                             INTERNET_DEFAULT_HTTPS_PORT,//指定https端口，默认端口为443
                                             NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
    isUpdateNetWorkRequest = false;
    if (!hConnection || QThread::currentThread()->isInterruptionRequested()) {
        emit checkFinished(Respond {-2, "InternetConnectA Failed."});
        InternetCloseHandle(hSession);
        InternetCloseHandle(hConnection);
        return;
    }
    isUpdateNetWorkRequest = true;
    HINTERNET hRequest = HttpOpenRequest(hConnection,
                                         L"GET",//指定动作，GET或者POST
                                         L"/repos/SkyD666/SmartScreenSnapper/releases/latest",//指定要请求的uri
                                         HTTP_VERSION,
                                         NULL, NULL, INTERNET_FLAG_SECURE | /*启用ssl模式*/INTERNET_FLAG_IGNORE_CERT_CN_INVALID/*忽略ssl模式下的证书名称错误*/, 0);
    isUpdateNetWorkRequest = false;
    if (!hRequest || QThread::currentThread()->isInterruptionRequested()) {
        emit checkFinished(Respond {-3, "HttpOpenRequest Failed.\n" + QString::number(GetLastError())});
        InternetCloseHandle(hSession);
        InternetCloseHandle(hConnection);
        InternetCloseHandle(hRequest);
        return;
    }
    //BOOL sendRequestResult = ;
    if (QThread::currentThread()->isInterruptionRequested()) return;
    std::string strRet = "";
    isUpdateNetWorkRequest = true;
    if (HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
        isUpdateNetWorkRequest = false;
        char szBuffer[READ_BUFFER_SIZE + 1] = {0};
        DWORD dwReadSize = READ_BUFFER_SIZE;
        HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
        while(1) {
            if (QThread::currentThread()->isInterruptionRequested()) return;
            isUpdateNetWorkRequest = true;
            BOOL bResult = InternetReadFile(hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
            isUpdateNetWorkRequest = false;
            if (!bResult || (0 == dwReadSize)) break;
            szBuffer[dwReadSize] = '\0';
            strRet.append(szBuffer);
        }
        emit checkFinished(Respond {0, QString::fromStdString(strRet)});
    } else {
        isUpdateNetWorkRequest = false;
        emit checkFinished(Respond {-4, "HttpSendRequest Failed.\n" + QString::number(GetLastError())});
    }
    InternetCloseHandle(hSession);
    InternetCloseHandle(hConnection);
    InternetCloseHandle(hRequest);
    return;
}

bool UpdateDialog::isNewVersion(QString version) {
    QStringList versionInfoList = version.split(".");
    QStringList currentVersionInfoList = QApplication::applicationVersion().split(".");

    if (versionInfoList.at(0) < currentVersionInfoList.at(0)) {
        return false;
    } else if (versionInfoList.at(0) == currentVersionInfoList.at(0) && versionInfoList.at(1) < currentVersionInfoList.at(1)) {
        return false;
    } else if (versionInfoList.at(0) == currentVersionInfoList.at(0) && versionInfoList.at(1) == currentVersionInfoList.at(1)
               && versionInfoList.at(2) < currentVersionInfoList.at(2)) {
        return false;
    } else if (versionInfoList.at(0) == currentVersionInfoList.at(0) && versionInfoList.at(1) == currentVersionInfoList.at(1)
               && versionInfoList.at(2) == currentVersionInfoList.at(2) &&
               (versionInfoList.at(3) <= currentVersionInfoList.at(3))) {
        return false;
    }
    return true;
}
UpdateThread::UpdateThread() {
    qRegisterMetaType<Respond>("Respond");
}

UpdateThread::~UpdateThread() {

}

void UpdateDialog::on_pushButtonDownloadManual_clicked()
{
    qDebug() << downloadUrl;
    ShellExecute(NULL, (LPCWSTR)L"open",
                 (LPCWSTR)downloadUrl.toStdWString().c_str(),
                 (LPCWSTR)L"", (LPCWSTR)L"", SW_SHOWNORMAL);
}


bool DownloadFileThread::getFile() {
    QString applicationFilePath = QApplication::applicationFilePath();
    const char* szFileName = applicationFilePath.replace(".exe", ".ex").replace("/", "\\").toStdString().c_str();
    DWORD dwSize = 0;
    char szTemp[2048] = {0};
    HINTERNET hOpen;
    HINTERNET  hConnect;
    FILE * pFile;

    isUpdateNetWorkRequest = true;
    if (!(hOpen = InternetOpenA("SmartScreenSnap", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0))) {
        isUpdateNetWorkRequest = false;
        emit downloadProgress(-1);
        qDebug() << "InternetOpenA Failed";
        return false;
    }
    isUpdateNetWorkRequest = false;
    if (QThread::currentThread()->isInterruptionRequested()) return false;

    isUpdateNetWorkRequest = true;
    if (!(hConnect = InternetOpenUrlA (hOpen, downloadUrl.toStdString().c_str(), NULL,
                                       0, INTERNET_FLAG_RELOAD, 0))) {
        isUpdateNetWorkRequest = false;
        emit downloadProgress(-2);
        qDebug() << "InternetOpenUrl Failed";
        return false;
    }
    isUpdateNetWorkRequest = false;
    if (QThread::currentThread()->isInterruptionRequested()) return false;
    if (!(pFile = _wfopen(QApplication::applicationFilePath().replace(".exe", ".ex").replace("/", "\\").toStdWString().c_str(), TEXT("wb")))) {
        emit downloadProgress(-3);
        qDebug() << "fopen Failed" << " " << szFileName;
        return false;
    }
    //    QFile qFile(QApplication::applicationFilePath().replace(".exe", ".ex"));
    //    qFile.open(QIODevice::ReadWrite);
    //    if (!qFile.isOpen()) {
    //        emit downloadProgress(-3);
    //        qDebug() << "fopen Failed" << " " << szFileName;
    //        return false;
    //    }
    int totalSize = 0;
    do {
        if (QThread::currentThread()->isInterruptionRequested()) return false;
        //isUpdateNetWorkRequest = true;
        if (!InternetReadFile(hConnect, szTemp, sizeof(szTemp), &dwSize)) {
            fclose (pFile);
            //isUpdateNetWorkRequest = false;
            emit downloadProgress(-4);
            qDebug() << "InternetReadFile Failed";
            return false;
        }
        //isUpdateNetWorkRequest = false;
        emit downloadProgress(totalSize += dwSize);
        if (!dwSize){
            break;  // Condition of dwSize=0 indicate EOF. Stop.
        } else {
            //qFile.write(szTemp);
            fwrite(szTemp, sizeof (char), dwSize , pFile);
        }
    } while (1);
    fflush (pFile);
    fclose (pFile);
    //qFile.close();
    emit downloadProgress(-100);
    return true;
}

DownloadFileThread::DownloadFileThread() {

}

DownloadFileThread::~DownloadFileThread() {

}

void UpdateDialog::on_pushButtonDownload_clicked()
{
    ui->pushButtonDownload->setEnabled(false);
    ui->labelDownloadSpeed->setVisible(true);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setMinimum(0);
    dThread = new QThread(this);
    downloadFileThread = new DownloadFileThread();
    connect(dThread, SIGNAL(started()), downloadFileThread, SLOT(getFile()));
    connect(dThread, SIGNAL(finished()), downloadFileThread, SLOT(deleteLater()));
    connect(downloadFileThread, SIGNAL(downloadProgress(long)), this, SLOT(showDownloadProgress(long)));
    downloadFileThread->moveToThread(dThread);
    dThread->start();
}

void UpdateDialog::showDownloadProgress(long value) {
    if (value < 0) {
        if (value == -100) {        //下载完成
            ui->labelDownloadSpeed->setVisible(false);

            QString applicationFilePath = QApplication::applicationFilePath().replace("/", "\\");

            QString updateCmd = "choice /t 3 /d y /n >nul\ntaskkill /im " + QApplication::applicationName() + ".exe" +
                    " -t -f\n:Repeat\ndel \"%~dp0" + QApplication::applicationName() +
                    ".exe\"\nif exist \"%~dp0" + QApplication::applicationName() + ".exe\" goto Repeat\n" + "cd /d \"%~dp0\"\nrename \"" +
                    QApplication::applicationName() + ".ex\" \"" + QApplication::applicationName() + ".exe\"\n:Repeat2\n" +
                    "if not exist \"%~dp0" + QApplication::applicationName() + ".exe\" goto Repeat2\nstart \"\" \"%~dp0" + QApplication::applicationName() + ".exe\"\ndel %0\n";
            ui->textEditChangeLog->setText(updateCmd);
            QFile qFile(QApplication::applicationFilePath().replace(".exe", ".bat"));
            if (qFile.exists()) {
                qFile.remove();
            }
            qFile.open(QIODevice::ReadWrite);
            qFile.write(updateCmd.toStdString().c_str());
            qFile.close();
            //qDebug() << "\"" + QApplication::applicationFilePath().replace(".exe", ".bat").replace("/", "\\") + "\"";
            ShellExecute(NULL, L"open",
                         (LPCWSTR)(QApplication::applicationFilePath().replace(".exe", ".bat").replace("/", "\\")).toStdWString().c_str(),
                         (LPCWSTR)L"", (LPCWSTR)L"", SW_HIDE);
            exit(0);
            return;
        } else if (value == -1) {
            QMessageBox::information(this, tr("Error"), tr("InternetOpenA Failed"), QMessageBox::Ok);
        } else if (value == -2) {
            QMessageBox::information(this, tr("Error"), tr("InternetOpenUrl Failed"), QMessageBox::Ok);
        } else if (value == -3) {
            QMessageBox::information(this, tr("Error"), tr("fopen Failed"), QMessageBox::Ok);
        } else if (value == -4) {
            QMessageBox::information(this, tr("Error"), tr("InternetReadFile Failed"), QMessageBox::Ok);
        }
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(0);
        return;
    }

    if (!lastDownloadData.lastTime) {
        lastDownloadData.lastB = value;
        lastDownloadData.lastTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    if (QDateTime::currentDateTime().toMSecsSinceEpoch() - lastDownloadData.lastTime > 100) {
        //qDebug() << value - lastDownloadData.lastB << " " << (QDateTime::currentDateTime().toMSecsSinceEpoch() - lastDownloadData.lastTime);
        double speed = 0.9765625 * (value - lastDownloadData.lastB) / (QDateTime::currentDateTime().toMSecsSinceEpoch() - lastDownloadData.lastTime);
        ui->labelDownloadSpeed->setText(QString::number(speed, 'f', 1) + "KB/s");
        lastDownloadData.lastB = value;
        lastDownloadData.lastTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    ui->progressBar->setTextVisible(true);
    ui->progressBar->setMaximum(fileSize);
    ui->progressBar->setValue(value);
}
