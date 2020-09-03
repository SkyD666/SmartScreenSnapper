#include "settingdialog.h"
#include "publicdata.h"
#include "mainwindow.h"
#include "ui_settingdialog.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QDebug>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    ui->tabWidgetSetting->tabBar()->hide();

    ui->listWidgetSetting->addItem(new QListWidgetItem(QIcon(":/images/icon.png"), tr("常规设置"), ui->listWidgetSetting));
//    ui->listWidgetSetting->addItem(new QListWidgetItem(QIcon(":/images/icon.png"), tr("热键设置"), ui->listWidgetSetting));

    connect(ui->listWidgetSetting, &QListWidget::currentRowChanged, [=](int currentRow){
        ui->tabWidgetSetting->setCurrentIndex(currentRow);
    });

    //此处的信号有重载
    connect(ui->comboBoxSnapType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        if(index <= (int)(sizeof(PublicData::snapType)/sizeof(SnapType))) {
            ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(PublicData::snapType[index].waitTime) + tr("s"));
            ui->horizontalSliderWaitTime->setValue(PublicData::snapType[index].waitTime);
            ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapType[index].hotKey));
            ui->lineEditAutoSavePath->setText(PublicData::snapType[index].autoSavePath);
            ui->checkBoxIsAutoSave->setChecked(PublicData::snapType[index].isAutoSave);
            ui->lineEditAutoSavePath->setEnabled(PublicData::snapType[index].isAutoSave);
            ui->toolButtonAutoSavePath->setEnabled(PublicData::snapType[index].isAutoSave);
        }
    });

    //注意调用顺序，下面这几句要在connect(ui->comboBoxSnapType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){下面
    //保证第一次进入时horizontalSliderWaitTime的connect能调用，有正确的值
    ui->comboBoxSnapType->addItem(QIcon(":/images/ScreenSnap.png"), tr("全屏截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/images/ActiveWindow.png"), tr("活动窗口截图"));
    ui->comboBoxSnapType->addItem(QIcon(":/images/CursorSnap.png"), tr("截取光标"));

    connect(ui->keySequenceEditHotKey, &QKeySequenceEdit::keySequenceChanged, [=](const QKeySequence &keySequence){
        PublicData::snapType[ui->comboBoxSnapType->currentIndex()].hotKey = keySequence.toString();
    });

    ui->keySequenceEditHotKey->setKeySequence(QKeySequence(PublicData::snapType[ui->comboBoxSnapType->currentIndex()].hotKey));

    readSettings(); //注意调用顺序，上面是进行初始设置，下面是在控件状态（选中、值等）发生改变时改变PublicData里的值

    connect(ui->horizontalSliderWaitTime, &QAbstractSlider::valueChanged, [=](int value){
        ui->labelWaitTime->setText(tr("截图前等待时间: ") + QString::number(value) + tr("s"));
        if(ui->comboBoxSnapType->currentIndex() <= (int)(sizeof(PublicData::snapType)/sizeof(SnapType))) {
            PublicData::snapType[ui->comboBoxSnapType->currentIndex()].waitTime = value;
        }
    });

    connect(ui->checkBoxIsAutoSave, &QCheckBox::stateChanged, [=](int state){
        if(ui->comboBoxSnapType->currentIndex() <= (int)(sizeof(PublicData::snapType)/sizeof(SnapType))) {
            PublicData::snapType[ui->comboBoxSnapType->currentIndex()].isAutoSave = state;
        }
        ui->lineEditAutoSavePath->setEnabled(state);
        ui->toolButtonAutoSavePath->setEnabled(state);
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::editingFinished, [=](){
        if(ui->comboBoxSnapType->currentIndex() <= (int)(sizeof(PublicData::snapType)/sizeof(SnapType))) {
            PublicData::snapType[ui->comboBoxSnapType->currentIndex()].autoSavePath = ui->lineEditAutoSavePath->text();
        }
    });

    connect(ui->lineEditAutoSavePath, &QLineEdit::textChanged, [=](const QString &text){
        if(ui->comboBoxSnapType->currentIndex() <= (int)(sizeof(PublicData::snapType)/sizeof(SnapType))) {
            PublicData::snapType[ui->comboBoxSnapType->currentIndex()].autoSavePath = text;
        }
    });

    connect(ui->checkBoxClickCloseToTray, &QCheckBox::stateChanged, [=](int state){
        PublicData::clickCloseToTray = state;
    });

    connect(ui->checkBoxPlaySound, &QCheckBox::stateChanged, [=](int state){
        PublicData::isPlaySound = state;
    });

    connect(ui->checkBoxHotKeyNoWait, &QCheckBox::stateChanged, [=](int state){
        PublicData::hotKeyNoWait = state;
    });

    connect(ui->checkBoxIncludeCursor, &QCheckBox::stateChanged, [=](int state){
        PublicData::includeCursor = state;
    });
}

SettingDialog::~SettingDialog()
{
    PublicData::writeSettings();
    delete ui;
}

void SettingDialog::readSettings()
{
    ui->checkBoxClickCloseToTray->setChecked(PublicData::clickCloseToTray);
    ui->checkBoxPlaySound->setChecked(PublicData::isPlaySound);
    ui->checkBoxHotKeyNoWait->setChecked(PublicData::hotKeyNoWait);
    ui->checkBoxIncludeCursor->setChecked(PublicData::includeCursor);
}

void SettingDialog::on_pushButtonDeleteHotKey_clicked()
{
    ui->keySequenceEditHotKey->clear();
}

void SettingDialog::on_toolButtonAutoSavePath_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择目录"),
                                                        PublicData::snapType[ui->comboBoxSnapType->currentIndex()].autoSavePath, QFileDialog::ShowDirsOnly);
    if (dirPath != "") {
        ui->lineEditAutoSavePath->setText(dirPath);
    }
}
