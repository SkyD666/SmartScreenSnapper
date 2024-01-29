#include "settingdialog.h"
#include "publicdata.h"
#include "ui_settingdialog.h"
#include <QCheckBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>

SettingDialog::SettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    ui->tabWidgetSetting->tabBar()->hide();
    auto tabBar = ui->tabWidgetSetting->tabBar();
    for (int i = 0; i < tabBar->count(); i++) {
        ui->listWidgetSetting->addItem(new QListWidgetItem(tabBar->tabIcon(i), tabBar->tabText(i), ui->listWidgetSetting));
    }

    readAndInitSettings(); // 进行初始设置

    initConnect(); // 在控件状态（选中、值等）发生改变时改变PublicData里的值
}

SettingDialog::~SettingDialog()
{
    PublicData::writeSettings();
    if (!PublicData::applyQss()) {
        QMessageBox::critical(this, tr("警告"), tr("QSS文件打开失败"), QMessageBox::Ok);
    }

    delete ui;
}

void SettingDialog::initConnect()
{
    connect(ui->listWidgetSetting, &QListWidget::currentRowChanged, this, [=](int currentRow) {
        ui->tabWidgetSetting->setCurrentIndex(currentRow);
    });

    connect(ui->comboBoxSnapMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index < SNAPMETHOD) {
            PublicData::snapMethod = index;
        }
    });

    connect(ui->keySequenceEditHotKey, &QKeySequenceEdit::keySequenceChanged, this, [=](const QKeySequence& keySequence) {
        PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].hotKey = keySequence.toString();
    });

    connect(ui->checkBoxNoBorder, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::noBorder = state;
    });

    connect(ui->horizontalSliderWaitTime, &QAbstractSlider::valueChanged, this, [=](int value) {
        ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(value) + tr("s"));
        if (ui->comboBoxSnapType->currentIndex() <= ScreenShotHelper::ShotType::Count) {
            PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].waitTime = value;
        }
    });

    connect(ui->cbManualSaveAfterShot, &QCheckBox::stateChanged, this, [=](int state) {
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].isManualSave = state;
        }
    });

    connect(ui->toolButtonDeleteHotKey, &QAbstractButton::clicked, this, [=]() {
        ui->keySequenceEditHotKey->clear();
    });

    connect(ui->checkBoxRunWithWindows, &QCheckBox::stateChanged, this, [=](int state) {
        runWithWindows(state);
    });

    connect(ui->toolButtonAutoSavePath, &QAbstractButton::clicked, this, [=]() {
        QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择目录"),
            PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].autoSavePath, QFileDialog::ShowDirsOnly);
        if (!dirPath.isEmpty()) {
            ui->lineEditAutoSavePath->setText(dirPath);
        }
    });

    connect(ui->toolButtonQssPath, &QAbstractButton::clicked, this, [=]() {
        QString dirPath = QFileDialog::getOpenFileName(this, tr("选择QSS文件"),
            PublicData::qssPath, tr("QSS文件(*.qss);;CSS文件(*.css);;所有文件(*.*)"));
        if (!dirPath.isEmpty()) {
            ui->lineEditQssPath->setText(dirPath);
        }
    });

    connect(ui->pushButtonOpenConfigFile, &QAbstractButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(PublicData::getConfigFilePath()));
    });

    connect(ui->cbAutoSaveAfterShot, &QCheckBox::stateChanged, this, [=](int state) {
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].isAutoSave = state;
        }
        ui->lineEditAutoSavePath->setEnabled(state);
        ui->toolButtonAutoSavePath->setEnabled(state);
        ui->comboBoxAutoSaveExtName->setEnabled(state);
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::editingFinished, this, [=]() {
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].autoSavePath = ui->lineEditAutoSavePath->text();
        }
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::textChanged, this, [=](const QString& text) {
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].autoSavePath = text;
        }
    });

    connect(ui->lineEditQssPath, &QLineEdit::editingFinished, this, [=]() {
        PublicData::qssPath = ui->lineEditQssPath->text();
    });

    connect(ui->lineEditQssPath, &QLineEdit::textChanged, this, [=](const QString& text) {
        PublicData::qssPath = text;
    });

    connect(ui->lineEditFileNameTemplate, &QLineEdit::editingFinished, this, [=]() {
        PublicData::fileNameTemplate = ui->lineEditFileNameTemplate->text();
    });

    connect(ui->lineEditFileNameTemplate, &QLineEdit::textChanged, this, [=](const QString& text) {
        PublicData::fileNameTemplate = text;
        ui->lineEditFileNamePreview->setText(
            ScreenShotHelper::getPictureName(ScreenShotHelper::ScreenShot));
    });

    // 自动保存格式，有信号重载
    connect(ui->comboBoxAutoSaveExtName, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        int i = ui->comboBoxSnapType->currentIndex();
        if (i <= ScreenShotHelper::ShotType::Count) {
            PublicData::snapTypeItems[i].autoSaveExtName = ui->comboBoxAutoSaveExtName->itemData(index).value<QString>();
        }
    });

    connect(ui->checkBoxClickCloseToTray, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::clickCloseToTray = state;
    });

    connect(ui->checkBoxPlaySound, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::isPlaySound = state;
    });

    connect(ui->comboBoxMdiWindowInitState, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        PublicData::mdiWindowInitState = ui->comboBoxMdiWindowInitState->itemData(index).value<Qt::WindowState>();
    });

    connect(ui->checkBoxHotKeyNoWait, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::hotKeyNoWait = state;
    });

    connect(ui->checkBoxIncludeCursor, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::includeCursor = state;
    });

    connect(ui->checkBoxCopyToClipBoardAfterSnap, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::copyToClipBoardAfterSnap = state;
    });

    connect(ui->checkBoxFreeSnapReleaseMouseCapture, &QCheckBox::stateChanged, this, [=](int state) {
        PublicData::freeSnapReleaseMouseCapture = state;
    });

    connect(ui->spinBoxImageQuality, &QSpinBox::valueChanged, this, [=](int i) {
        PublicData::saveImageQuality = i;
    });
}

void SettingDialog::readAndInitSettings()
{
    for (auto item : PublicData::imageExtName) {
        ui->comboBoxAutoSaveExtName->addItem(item.second, item.first);
    }

    connect(ui->comboBoxSnapType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index <= ScreenShotHelper::ShotType::Count) {
            ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(PublicData::snapTypeItems[index].waitTime) + tr("s"));
            ui->horizontalSliderWaitTime->setValue(PublicData::snapTypeItems[index].waitTime);
            ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapTypeItems[index].hotKey));
            ui->lineEditAutoSavePath->setText(PublicData::snapTypeItems[index].autoSavePath);
            ui->cbManualSaveAfterShot->setChecked(PublicData::snapTypeItems[index].isManualSave);
            ui->cbAutoSaveAfterShot->setChecked(PublicData::snapTypeItems[index].isAutoSave);
            for (auto ext : PublicData::imageExtName) {
                if (ext.first == PublicData::snapTypeItems[index].autoSaveExtName) {
                    ui->comboBoxAutoSaveExtName->setCurrentText(ext.second);
                    break;
                }
            }
            ui->lineEditAutoSavePath->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
            ui->toolButtonAutoSavePath->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
            ui->comboBoxAutoSaveExtName->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
        }
    });

    ui->comboBoxSnapType->addItem(QIcon(":/image/ScreenSnap.png"), tr("全屏截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/ActiveWindow.png"), tr("活动窗口截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/CursorSnap.png"), tr("截取光标"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/FreeSnap.png"), tr("自由截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/FreeHandSnap.svg"), tr("徒手截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/SnapByPoint.svg"), tr("窗体控件截图"));

    ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].hotKey));

    ui->comboBoxSnapMethod->addItem(tr("方式1\n(Aero开启时部分区域会透明；截取例如QQ等部分窗体为黑色)"));
    ui->comboBoxSnapMethod->addItem(tr("方式2"));

    foreach (const auto& key, PublicData::mdiWindowInitStates.keys()) {
        ui->comboBoxMdiWindowInitState->addItem(PublicData::mdiWindowInitStates[key], key);
    }
    ui->comboBoxMdiWindowInitState->setCurrentText(PublicData::mdiWindowInitStates[PublicData::mdiWindowInitState]);

    ui->checkBoxClickCloseToTray->setChecked(PublicData::clickCloseToTray);
    ui->checkBoxPlaySound->setChecked(PublicData::isPlaySound);
    ui->checkBoxHotKeyNoWait->setChecked(PublicData::hotKeyNoWait);
    ui->checkBoxIncludeCursor->setChecked(PublicData::includeCursor);
    ui->spinBoxImageQuality->setValue(PublicData::saveImageQuality);
    ui->checkBoxNoBorder->setChecked(PublicData::noBorder);
    ui->checkBoxCopyToClipBoardAfterSnap->setChecked(PublicData::copyToClipBoardAfterSnap);
    ui->checkBoxFreeSnapReleaseMouseCapture->setChecked(PublicData::freeSnapReleaseMouseCapture);
    ui->comboBoxSnapMethod->setCurrentIndex(PublicData::snapMethod);
    ui->lineEditQssPath->setText(PublicData::qssPath);
    ui->lineEditFileNameTemplate->setText(PublicData::fileNameTemplate);
    ui->lineEditFileNamePreview->setText(ScreenShotHelper::getPictureName(ScreenShotHelper::ScreenShot));
    ui->lineEditFileNameTemplate->setValidator(new QRegularExpressionValidator(QRegularExpression("^[^/*?\"\\\\:|]+$"), this));

    QSettings qSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString value = qSettings.value(QApplication::applicationName()).toString();
    QString appPath = QApplication::applicationFilePath();
    appPath = appPath.replace("/", "\\");
    ui->checkBoxRunWithWindows->setChecked(value == "\"" + appPath + "\"" + " -autorun");
}

void SettingDialog::runWithWindows(bool enable)
{
    QSettings qSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appPath = QApplication::applicationFilePath();
    QString appName = QApplication::applicationName();
    appPath = appPath.replace("/", "\\");
    if (enable) {
        qSettings.setValue(appName, "\"" + appPath + "\"" + " -autorun");
    } else {
        qSettings.remove(appName);
    }
}
