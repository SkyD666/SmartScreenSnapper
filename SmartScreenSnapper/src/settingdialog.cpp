#include "settingdialog.h"
#include "publicdata.h"
#include "ui_settingdialog.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QRegularExpression>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    ui->tabWidgetSetting->tabBar()->hide();

    auto tabBar = ui->tabWidgetSetting->tabBar();
    for (int i = 0; i < tabBar->count(); i++) {
        ui->listWidgetSetting->addItem(new QListWidgetItem(tabBar->tabIcon(i), tabBar->tabText(i), ui->listWidgetSetting));
    }

    connect(ui->listWidgetSetting, &QListWidget::currentRowChanged, [=](int currentRow){
        ui->tabWidgetSetting->setCurrentIndex(currentRow);
    });

    // 下面的for循环要在下面的第一个connect之前调用
    for (auto item : PublicData::imageExtName) {
        ui->comboBoxAutoSaveExtName->addItem(item.second + " (*" + item.first + ")");
    }

    // 此处的信号有重载
    connect(ui->comboBoxSnapType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        if(index <= (int)(sizeof(PublicData::snapTypeItems)/sizeof(ShotTypeItem))) {
            ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(PublicData::snapTypeItems[index].waitTime) + tr("s"));
            ui->horizontalSliderWaitTime->setValue(PublicData::snapTypeItems[index].waitTime);
            ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapTypeItems[index].hotKey));
            ui->lineEditAutoSavePath->setText(PublicData::snapTypeItems[index].autoSavePath);
            ui->cbManualSaveAfterShot->setChecked(PublicData::snapTypeItems[index].isManualSave);
            ui->cbAutoSaveAfterShot->setChecked(PublicData::snapTypeItems[index].isAutoSave);
            ui->comboBoxAutoSaveExtName->setCurrentText(PublicData::snapTypeItems[index].autoSaveExtName);
            ui->lineEditAutoSavePath->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
            ui->toolButtonAutoSavePath->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
            ui->comboBoxAutoSaveExtName->setEnabled(PublicData::snapTypeItems[index].isAutoSave);
        }
    });

    // 注意调用顺序，下面这几句要在connect(ui->comboBoxSnapType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){下面
    // 保证第一次进入时horizontalSliderWaitTime的connect能调用，有正确的值
    ui->comboBoxSnapType->addItem(QIcon(":/image/ScreenSnap.png"), tr("全屏截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/ActiveWindow.png"), tr("活动窗口截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/CursorSnap.png"), tr("截取光标"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/FreeSnap.png"), tr("自由截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/FreeHandSnap.svg"), tr("徒手截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/image/SnapByPoint.svg"), tr("窗体控件截图"));

    connect(ui->keySequenceEditHotKey, &QKeySequenceEdit::keySequenceChanged, [=](const QKeySequence &keySequence){
        PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].hotKey = keySequence.toString();
    });

    ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].hotKey));

    ui->comboBoxSnapMethod->addItem(tr("方式1\n(Aero开启时部分区域会透明；截取例如QQ等部分窗体为黑色)"));
    ui->comboBoxSnapMethod->addItem(tr("方式2"));

    //此处的信号有重载
    connect(ui->comboBoxSnapMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        if (index < SNAPMETHOD) {
            PublicData::snapMethod = index;
        }
    });

    //要在read之前
    connect(ui->checkBoxNoBorder, &QCheckBox::stateChanged, [=](int state){
        PublicData::noBorder = state;
    });

    readSettings(); //注意调用顺序，上面是进行初始设置，下面是在控件状态（选中、值等）发生改变时改变PublicData里的值

    connect(ui->horizontalSliderWaitTime, &QAbstractSlider::valueChanged, [=](int value){
        ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(value) + tr("s"));
        if(ui->comboBoxSnapType->currentIndex() <= (int)(sizeof(PublicData::snapTypeItems)/sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].waitTime = value;
        }
    });

    connect(ui->cbManualSaveAfterShot, &QCheckBox::stateChanged, [=](int state){
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].isManualSave = state;
        }
    });

    connect(ui->cbAutoSaveAfterShot, &QCheckBox::stateChanged, [=](int state){
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].isAutoSave = state;
        }
        ui->lineEditAutoSavePath->setEnabled(state);
        ui->toolButtonAutoSavePath->setEnabled(state);
        ui->comboBoxAutoSaveExtName->setEnabled(state);
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::editingFinished, [=](){
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].autoSavePath = ui->lineEditAutoSavePath->text();
        }
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::textChanged, [=](const QString &text){
        int index = ui->comboBoxSnapType->currentIndex();
        if (index <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[index].autoSavePath = text;
        }
    });

    connect(ui->lineEditQssPath, &QLineEdit::editingFinished, [=](){
        PublicData::qssPath = ui->lineEditQssPath->text();
    });

    connect(ui->lineEditQssPath, &QLineEdit::textChanged, [=](const QString &text){
        PublicData::qssPath = text;
    });

    connect(ui->lineEditFileNameTemplate, &QLineEdit::editingFinished, [=](){
        PublicData::fileNameTemplate = ui->lineEditFileNameTemplate->text();
    });

    connect(ui->lineEditFileNameTemplate, &QLineEdit::textChanged, [=](const QString &text){
        PublicData::fileNameTemplate = text;
        ui->lineEditFileNamePreview->setText(
                    ScreenShotHelper::getPictureName(ScreenShotHelper::ScreenShot));
    });

    ui->lineEditFileNameTemplate->setValidator(new QRegularExpressionValidator(QRegularExpression("^[^/*?\"\\\\:|]+$"), this));

    // 自动保存格式，有信号重载
    connect(ui->comboBoxAutoSaveExtName, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        int i = ui->comboBoxSnapType->currentIndex();
        if (i <= (int)(sizeof(PublicData::snapTypeItems) / sizeof(ShotTypeItem))) {
            PublicData::snapTypeItems[i].autoSaveExtName = PublicData::imageExtName[index].first;
        }
    });

    connect(ui->checkBoxClickCloseToTray, &QCheckBox::stateChanged, [=](int state){
        PublicData::clickCloseToTray = state;
    });

    connect(ui->checkBoxPlaySound, &QCheckBox::stateChanged, [=](int state){
        PublicData::isPlaySound = state;
    });

    connect(ui->comboBoxMdiWindowInitState, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        PublicData::mdiWindowInitState = ui->comboBoxMdiWindowInitState->itemData(index).value<Qt::WindowState>();
    });

    connect(ui->checkBoxHotKeyNoWait, &QCheckBox::stateChanged, [=](int state){
        PublicData::hotKeyNoWait = state;
    });

    connect(ui->checkBoxIncludeCursor, &QCheckBox::stateChanged, [=](int state){
        PublicData::includeCursor = state;
    });

    connect(ui->checkBoxCopyToClipBoardAfterSnap, &QCheckBox::stateChanged, [=](int state){
        PublicData::copyToClipBoardAfterSnap = state;
    });
}

SettingDialog::~SettingDialog()
{
    PublicData::writeSettings();
    if (!PublicData::applyQss()) QMessageBox::critical(this, tr("警告"), tr("QSS文件打开失败"), QMessageBox::Ok);

    delete ui;
}

void SettingDialog::readSettings()
{
    for (auto key : PublicData::mdiWindowInitStates.keys()) {
        ui->comboBoxMdiWindowInitState->addItem(PublicData::mdiWindowInitStates[key], key);
    }
    ui->comboBoxMdiWindowInitState->setCurrentText(PublicData::mdiWindowInitStates[PublicData::mdiWindowInitState]);

    ui->checkBoxClickCloseToTray->setChecked(PublicData::clickCloseToTray);
    ui->checkBoxPlaySound->setChecked(PublicData::isPlaySound);
    ui->checkBoxHotKeyNoWait->setChecked(PublicData::hotKeyNoWait);
    ui->checkBoxIncludeCursor->setChecked(PublicData::includeCursor);
    ui->checkBoxNoBorder->setChecked(PublicData::noBorder);
    ui->checkBoxCopyToClipBoardAfterSnap->setChecked(PublicData::copyToClipBoardAfterSnap);
    ui->comboBoxSnapMethod->setCurrentIndex(PublicData::snapMethod);
    ui->lineEditQssPath->setText(PublicData::qssPath);
    ui->lineEditFileNameTemplate->setText(PublicData::fileNameTemplate);
    ui->lineEditFileNamePreview->setText(ScreenShotHelper::getPictureName(ScreenShotHelper::ScreenShot));

    QSettings qSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    QString value = qSettings.value(QApplication::applicationName()).toString();
    QString appPath = QApplication::applicationFilePath();
    appPath = appPath.replace("/","\\");
    if (value == "\"" + appPath + "\"" + " -autorun") {
        ui->checkBoxRunWithWindows->setChecked(true);
    } else {
        ui->checkBoxRunWithWindows->setChecked(false);
    }
}

void SettingDialog::on_pushButtonDeleteHotKey_clicked()
{
    ui->keySequenceEditHotKey->clear();
}

void SettingDialog::on_toolButtonAutoSavePath_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择目录"),
                                                        PublicData::snapTypeItems[ui->comboBoxSnapType->currentIndex()].autoSavePath, QFileDialog::ShowDirsOnly);
    if (dirPath != "") {
        ui->lineEditAutoSavePath->setText(dirPath);
    }
}

void SettingDialog::on_checkBoxRunWithWindows_stateChanged(int state)
{
    QSettings qSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    QString appPath = QApplication::applicationFilePath();
    QString appName = QApplication::applicationName();
    appPath = appPath.replace("/","\\");
    if (state) {
        qSettings.setValue(appName, "\"" + appPath + "\"" + " -autorun");
    } else {
        qSettings.remove(appName);
    }
}

void SettingDialog::on_toolButtonQssPath_clicked()
{
    QString dirPath = QFileDialog::getOpenFileName(this, tr("选择QSS文件"),
                                                   PublicData::qssPath, tr("QSS文件(*.qss);;CSS文件(*.css);;所有文件(*.*)"));
    if (dirPath != "") {
        ui->lineEditQssPath->setText(dirPath);
    }
}

void SettingDialog::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(PublicData::getConfigFilePath()));
}
