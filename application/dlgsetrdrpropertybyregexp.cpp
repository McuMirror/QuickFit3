#include "dlgsetrdrpropertybyregexp.h"
#include "ui_dlgsetrdrpropertybyregexp.h"
#include "programoptions.h"

DlgSetRDRPropertyByRegExp::DlgSetRDRPropertyByRegExp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSetRDRPropertyByRegExp)
{
    ui->setupUi(this);
    ui->chkCaseSensitive->setChecked(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/case_sensitive", ui->chkCaseSensitive->isChecked()).toBool());
    ui->chkMinimal->setChecked(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/minimal", ui->chkMinimal->isChecked()).toBool());
    ui->cmbInputData->setCurrentIndex(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/input", ui->cmbInputData->currentIndex()).toInt());
    ui->cmbMode->setCurrentIndex(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/mode", ui->cmbMode->currentIndex()).toInt());
    ui->cmbOutput->setCurrentIndex(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/output", ui->cmbOutput->currentIndex()).toInt());
    ui->edtOutputValue->setText(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/output_val", ui->edtOutputValue->text()).toString());
    ui->edtRegExp->setText(ProgramOptions::getConfigValue("DlgSetRDRPropertyByRegExp/regexp", ui->edtRegExp->text()).toString());
}

DlgSetRDRPropertyByRegExp::~DlgSetRDRPropertyByRegExp()
{
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/case_sensitive", ui->chkCaseSensitive->isChecked());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/minimal", ui->chkMinimal->isChecked());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/input", ui->cmbInputData->currentIndex());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/mode", ui->cmbMode->currentIndex());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/output", ui->cmbOutput->currentIndex());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/output_val", ui->edtOutputValue->text());
    ProgramOptions::setConfigValue("DlgSetRDRPropertyByRegExp/regexp", ui->edtRegExp->text());
    delete ui;
}

void DlgSetRDRPropertyByRegExp::setProject(QFProject *project)
{
    this->project=project;

    rdrs.clear();
    for (int i=0; i<project->getRawDataCount(); i++) {
        QPointer<QFRawDataRecord> r=project->getRawDataByNum(i);
        rdrs.append(r);
        QListWidgetItem* item=new QListWidgetItem(r->getName(), ui->lstRDR);
        item->setCheckState(Qt::Checked);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, r->getID());
        item->setIcon(r->getSmallIcon());
        ui->lstRDR->addItem(item);
    }

}

QList<QPointer<QFRawDataRecord> > DlgSetRDRPropertyByRegExp::getSelectedRDRs() const
{
    QList<QPointer<QFRawDataRecord> > l;
    for (int i=0; i<ui->lstRDR->count(); i++)  {
        if (ui->lstRDR->item(i)->checkState()==Qt::Checked) {
            l<<rdrs[i];
        }
    }
    return l;
}

QString DlgSetRDRPropertyByRegExp::getInputString(QFRawDataRecord *rdr) const
{
    if (!rdr) return QString();
    if (ui->cmbInputData->currentIndex()==0) {
        return rdr->getName();
    }
    return QString();
}

QString DlgSetRDRPropertyByRegExp::getResult(QFRawDataRecord *rdr, bool* apply) const
{
    QRegExp rx(ui->edtRegExp->text());
    rx.setCaseSensitivity((ui->chkCaseSensitive->isChecked())?Qt::CaseSensitive:Qt::CaseInsensitive);
    rx.setMinimal(ui->chkMinimal->isChecked());
    QString input=getInputString(rdr);
    QString output=ui->edtOutputValue->text();
    if (ui->cmbMode->currentIndex()==0) {
        if (rx.exactMatch(input)) {
            if (apply) *apply=true;
            return output;
        }
    } else if (ui->cmbMode->currentIndex()==1) {
        if (!rx.exactMatch(input)) {
            if (apply) *apply=true;
            return output;
        }
    } else if (ui->cmbMode->currentIndex()==2) {
        int idx=rx.indexIn(input);
        int len=rx.matchedLength();
        if (idx>=0) {
            if (apply) *apply=true;
            return rx.cap(0);
        }
    } else if (ui->cmbMode->currentIndex()==3) {
        int idx=rx.indexIn(input);
        if (idx>=0) {
            if (apply) *apply=true;
            return rx.cap(1);
        }
    } else if (ui->cmbMode->currentIndex()==4) {
        int idx=rx.indexIn(input);
        if (idx>=0) {
            if (apply) *apply=true;
            return rx.cap(2);
        }
    } else if (ui->cmbMode->currentIndex()==5) {
        int idx=rx.indexIn(input);
        if (idx>=0) {
            if (apply) *apply=true;
            return rx.cap(3);
        }
    }
    if (apply) *apply=false;
    return QString();
}

void DlgSetRDRPropertyByRegExp::applyResult(QFRawDataRecord *rdr)
{
    bool apply=false;
    QString res=getResult(rdr, &apply);
    if (apply) {
        switch (ui->cmbOutput->currentIndex()) {
            case 0: // RDR group
                rdr->setGroup(project->addOrFindRDRGroup(res));
                break;
            case 1: // role
                rdr->setRole(res);
                break;
            case 2: // folder
                rdr->setFolder(res);
                break;
            case 3: // name
                rdr->setName(res);
                break;
        }
    }
}

void DlgSetRDRPropertyByRegExp::on_btnSelectNone_clicked()
{
    for (int i=0; i<ui->lstRDR->count(); i++)  {
        ui->lstRDR->item(i)->setCheckState(Qt::Unchecked);
    }
}

void DlgSetRDRPropertyByRegExp::on_btnSelectAll_clicked()
{
    for (int i=0; i<ui->lstRDR->count(); i++)  {
        ui->lstRDR->item(i)->setCheckState(Qt::Checked);
    }
}


void DlgSetRDRPropertyByRegExp::updateTest()
{
    int idx=ui->lstRDR->currentRow();
    QFRawDataRecord* rdr=rdrs.value(idx, NULL);
    if (rdr) {
        ui->labTest->setText(getResult(rdr));//QString("<tt>%1</tt>").arg(getResult(rdr)));
    }
}
