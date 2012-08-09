#include "qfespimb040acquisitionconfigwidget2.h"
#include "ui_qfespimb040acquisitionconfigwidget2.h"
#include "qfespimb040mainwindow.h"
#include "qfpluginservices.h"
#include "qfstagecombobox.h"
#include "qfextensionmanager.h"
#include "../interfaces/qfextensionlinearstage.h"
#include "qfextension.h"
#include <QtGui>
#include <QtCore>


QFESPIMB040AcquisitionConfigWidget2::QFESPIMB040AcquisitionConfigWidget2(QWidget* parent, QFPluginServices* pluginServices, QFESPIMB040OpticsSetup* opticsSetup, QString configDirectory) :
    QWidget(parent),
    ui(new Ui::QFESPIMB040AcquisitionConfigWidget2)
{
    m_pluginServices=pluginServices;
    this->opticsSetup=opticsSetup;
    ui->setupUi(this);
    if (opticsSetup) {
        ui->cmbAcquisitionSettings1->init(configDirectory);
        ui->cmbAcquisitionSettings1->setStopResume(opticsSetup->getStopRelease(0));
        ui->cmbAcquisitionSettings1->connectTo(opticsSetup->cameraComboBox(0));
        ui->cmbPreviewSettings1->init(configDirectory);
        ui->cmbPreviewSettings1->setStopResume(opticsSetup->getStopRelease(0));
        ui->cmbPreviewSettings1->connectTo(opticsSetup->cameraComboBox(0));

        ui->cmbAcquisitionSettings2->init(configDirectory);
        ui->cmbAcquisitionSettings2->setStopResume(opticsSetup->getStopRelease(1));
        ui->cmbAcquisitionSettings2->connectTo(opticsSetup->cameraComboBox(1));
        ui->cmbPreviewSettings2->init(configDirectory);
        ui->cmbPreviewSettings2->setStopResume(opticsSetup->getStopRelease(1));
        ui->cmbPreviewSettings2->connectTo(opticsSetup->cameraComboBox(1));
    }

    updateReplaces();
    bindLineEdit(ui->edtPrefix1);
    bindLineEdit(ui->edtPrefix2);

}

QFESPIMB040AcquisitionConfigWidget2::~QFESPIMB040AcquisitionConfigWidget2()
{
    delete ui;
}


void QFESPIMB040AcquisitionConfigWidget2::loadSettings(QSettings& settings, QString prefix) {
    ui->chkOverview->setChecked(settings.value(prefix+"overview", true).toBool());
    ui->chkBackground->setChecked(settings.value(prefix+"background", true).toBool());
    ui->spinBackgroundFrames1->setValue(settings.value(prefix+"background_frames1", 1000).toInt());
    ui->spinBackgroundFrames2->setValue(settings.value(prefix+"background_frames2", 1000).toInt());
    ui->chkUse1->setChecked(settings.value(prefix+"use1", true).toBool());
    ui->chkUse2->setChecked(settings.value(prefix+"use2", true).toBool());
    ui->edtPrefix1->setText(settings.value(prefix+"prefix1", "stack_cam1_%counter%").toString());
    ui->edtPrefix2->setText(settings.value(prefix+"prefix2", "stack_cam2_%counter%").toString());
    ui->cmbAcquisitionSettings1->setCurrentConfig(settings.value(prefix+"acqsettings1", "default").toString());
    ui->cmbAcquisitionSettings2->setCurrentConfig(settings.value(prefix+"acqsettings2", "default").toString());
    ui->cmbPreviewSettings1->setCurrentConfig(settings.value(prefix+"prevsettings1", "default").toString());
    ui->cmbPreviewSettings2->setCurrentConfig(settings.value(prefix+"prevsettings2", "default").toString());

    ui->chkLightpath->setChecked(settings.value(prefix+"lightpath", false).toBool());
    ui->cmbLightpath->setCurrentIndex(settings.value(prefix+"lightpathidx", -1).toInt());

    //qDebug()<<settings.value(prefix+"prevsettings1", "default").toString();
    on_chkUse1_toggled(ui->chkUse1->isChecked());
    on_chkUse2_toggled(ui->chkUse2->isChecked());
    on_chkBackground_toggled(ui->chkBackground->isChecked());
}


void QFESPIMB040AcquisitionConfigWidget2::storeSettings(QSettings& settings, QString prefix) const {
    settings.setValue(prefix+"use1", ui->chkUse1->isChecked());
    settings.setValue(prefix+"use2", ui->chkUse2->isChecked());
    settings.setValue(prefix+"prefix1", ui->edtPrefix1->text());
    settings.setValue(prefix+"prefix2", ui->edtPrefix2->text());
    settings.setValue(prefix+"overview", ui->chkOverview->isChecked());
    settings.setValue(prefix+"background", ui->chkBackground->isChecked());
    settings.setValue(prefix+"background_frames1", ui->spinBackgroundFrames1->value());
    settings.setValue(prefix+"background_frames2", ui->spinBackgroundFrames2->value());
    settings.setValue(prefix+"acqsettings1", ui->cmbAcquisitionSettings1->currentConfigName());
    settings.setValue(prefix+"acqsettings2", ui->cmbAcquisitionSettings2->currentConfigName());
    settings.setValue(prefix+"prevsettings1", ui->cmbPreviewSettings1->currentConfigName());
    settings.setValue(prefix+"prevsettings2", ui->cmbPreviewSettings2->currentConfigName());

    settings.setValue(prefix+"lightpathidx", ui->cmbLightpath->currentIndex());
    settings.setValue(prefix+"lightpath", ui->chkLightpath->isChecked());
}




QString QFESPIMB040AcquisitionConfigWidget2::prefix1() const {
    QString filename= ui->edtPrefix1->text();
    filename=transformFilename(filename);
    return filename;
}

QString QFESPIMB040AcquisitionConfigWidget2::prefix2() const {
    QString filename= ui->edtPrefix2->text();
    filename=transformFilename(filename);
    return filename;
}

bool QFESPIMB040AcquisitionConfigWidget2::use1() const {
    return ui->chkUse1->isChecked();
}

bool QFESPIMB040AcquisitionConfigWidget2::use2() const {
    return ui->chkUse2->isChecked();
}

bool QFESPIMB040AcquisitionConfigWidget2::overview() const {
    return ui->chkOverview->isChecked();
}

bool QFESPIMB040AcquisitionConfigWidget2::lightpathActivated() const {
    return ui->chkLightpath->isChecked();
}

QString QFESPIMB040AcquisitionConfigWidget2::lightpathFilename() const {
    return ui->cmbLightpath->itemData(ui->cmbLightpath->currentIndex()).toString();
}

QString QFESPIMB040AcquisitionConfigWidget2::lightpath() const {
    return ui->cmbLightpath->currentText();
}

void QFESPIMB040AcquisitionConfigWidget2::on_btnAcquire_clicked() {

    emit doAcquisition();
}

void QFESPIMB040AcquisitionConfigWidget2::on_chkUse1_toggled(bool enabled) {
    ui->btnAcquire->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->chkOverview->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->chkBackground->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->edtPrefix1->setEnabled(ui->chkUse1->isChecked());
    ui->widCam2Settings1->setEnabled(ui->chkUse1->isChecked());

    updateBackgroundWidgets();
}

void QFESPIMB040AcquisitionConfigWidget2::on_chkUse2_toggled(bool enabled) {
    ui->btnAcquire->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->chkOverview->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->chkBackground->setEnabled(ui->chkUse1->isChecked() || ui->chkUse2->isChecked());
    ui->edtPrefix2->setEnabled(ui->chkUse2->isChecked());
    ui->widCam2Settings2->setEnabled(ui->chkUse2->isChecked());

    updateBackgroundWidgets();
}

void QFESPIMB040AcquisitionConfigWidget2::updateBackgroundWidgets() {
    ui->cmbPreviewSettings1->setEnabled(ui->chkBackground->isChecked());
    ui->cmbPreviewSettings2->setEnabled(ui->chkBackground->isChecked());
    ui->spinBackgroundFrames1->setEnabled(ui->chkBackground->isChecked());
    ui->spinBackgroundFrames2->setEnabled(ui->chkBackground->isChecked());
    ui->labPreviewFrames1->setEnabled(ui->chkBackground->isChecked());
    ui->labPreview1->setEnabled(ui->chkBackground->isChecked());
    ui->labPreviewFrames2->setEnabled(ui->chkBackground->isChecked());
    ui->labPreview2->setEnabled(ui->chkBackground->isChecked());
}

void QFESPIMB040AcquisitionConfigWidget2::on_chkBackground_toggled(bool enabled) {
    on_chkUse1_toggled(ui->chkUse1->isChecked());
    on_chkUse2_toggled(ui->chkUse2->isChecked());
}

QString QFESPIMB040AcquisitionConfigWidget2::currentConfigFilename(int camera) const {
    if (camera==0) return ui->cmbAcquisitionSettings1->currentConfigFilename();
    if (camera==1) return ui->cmbAcquisitionSettings2->currentConfigFilename();
    return "";
}

QString QFESPIMB040AcquisitionConfigWidget2::currentConfigName(int camera) const {
    if (camera==0) return ui->cmbAcquisitionSettings1->currentConfigName();
    if (camera==1) return ui->cmbAcquisitionSettings2->currentConfigName();
    return "";
}

QString QFESPIMB040AcquisitionConfigWidget2::currentPreviewConfigFilename(int camera) const {
    if (camera==0) return ui->cmbPreviewSettings1->currentConfigFilename();
    if (camera==1) return ui->cmbPreviewSettings2->currentConfigFilename();
    return "";
}

QString QFESPIMB040AcquisitionConfigWidget2::currentPreviewConfigName(int camera) const {
    if (camera==0) return ui->cmbPreviewSettings1->currentConfigName();
    if (camera==1) return ui->cmbPreviewSettings2->currentConfigName();
    return "";
}

int QFESPIMB040AcquisitionConfigWidget2::currentBackgroundFrames(int camera) const {
    if (camera==0) return ui->spinBackgroundFrames1->value();
    if (camera==1) return ui->spinBackgroundFrames2->value();
    return 1;
}

bool QFESPIMB040AcquisitionConfigWidget2::saveBackground() const {
    return ui->chkBackground->isChecked();
}

void QFESPIMB040AcquisitionConfigWidget2::lightpathesChanged(QFESPIMB040OpticsSetupItems lightpathes) {
    QString idx=ui->cmbLightpath->currentText();
    ui->cmbLightpath->clear();
    for (int i=0; i<lightpathes.size(); i++) {
        QTriple<QIcon, QString, QString> p=lightpathes[i];
        ui->cmbLightpath->addItem(p.first, p.second, p.third);
    }
    ui->cmbLightpath->setCurrentIndex(qMax(0, ui->cmbLightpath->findText(idx)));
}

void QFESPIMB040AcquisitionConfigWidget2::updateReplaces()
{
    setGlobalReplaces(opticsSetup);
}

int QFESPIMB040AcquisitionConfigWidget2::repeats() const
{
    return ui->spinRepeat->value();
}
