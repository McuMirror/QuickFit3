#include "filters.h"
#include "ui_qf3filtereditor.h"

FilterDescription::FilterDescription() {
    manufacturer="";
    name=QObject::tr("invalid");
    type="";
}


QF3FilterEditor::QF3FilterEditor(const FilterDescription& filter, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QF3FilterEditor)
{
    ui->setupUi(this);
    init(filter);
}

QF3FilterEditor::~QF3FilterEditor()
{
    delete ui;
}

void QF3FilterEditor::init(const FilterDescription& filter) {
    ui->edtManufacturer->setText(filter.manufacturer);
    ui->edtName->setText(filter.name);
    ui->cmbType->setEditText(filter.type);
}

FilterDescription QF3FilterEditor::getData() {
    FilterDescription o;
    o.name=ui->edtName->text();
    o.manufacturer=ui->edtManufacturer->text();
    o.type=ui->cmbType->currentText();
    return o;
}

















QF3FilterComboboxNotifier* QF3FilterCombobox::m_notifier=NULL;


QF3FilterCombobox::QF3FilterCombobox(QWidget* parent):
    QWidget(parent)
{
    if (m_notifier==NULL) m_notifier=new QF3FilterComboboxNotifier(this);
    globalfilters="";
    localfilters="";

    QHBoxLayout* hbl=new QHBoxLayout(this);
    setLayout(hbl);
    hbl->setContentsMargins(0,0,0,0);
    hbl->setSpacing(1);
    cmbFilters=new QComboBox(this);
    connect(cmbFilters, SIGNAL(currentIndexChanged(int)), this, SLOT(currentFilterChanged(int)));
    hbl->addWidget(cmbFilters);

    btnAddFilter=new QToolButton(this);
    btnAddFilter->setToolTip(tr("add a new filter"));
    btnAddFilter->setIcon(QIcon(":/libqf3widgets/filter_add.png"));
    btnAddFilter->setEnabled(true);
    connect(btnAddFilter, SIGNAL(clicked()), this, SLOT(addFilter()));
    hbl->addWidget(btnAddFilter);
    btnDeleteFilter=new QToolButton(this);
    btnDeleteFilter->setToolTip(tr("delete the current filter"));
    btnDeleteFilter->setIcon(QIcon(":/libqf3widgets/filter_delete.png"));
    btnDeleteFilter->setEnabled(true);
    connect(btnDeleteFilter, SIGNAL(clicked()), this, SLOT(deleteFilter()));
    hbl->addWidget(btnDeleteFilter);
    btnEditFilter=new QToolButton(this);
    btnEditFilter->setToolTip(tr("edit the current filter"));
    btnEditFilter->setIcon(QIcon(":/libqf3widgets/filter_rename.png"));
    btnEditFilter->setEnabled(true);
    connect(btnEditFilter, SIGNAL(clicked()), this, SLOT(editFilter()));
    hbl->addWidget(btnEditFilter);
    labFilterDescription=new QLabel(this);
    hbl->addWidget(labFilterDescription, 1);


    connect(m_notifier, SIGNAL(doUpdate()), this, SLOT(loadFilters()));

}

QF3FilterCombobox::~QF3FilterCombobox() {

}

void QF3FilterCombobox::currentFilterChanged(int idx) {
    int iD=cmbFilters->currentIndex();
    if (iD>=0 && iD<filters.size()) labFilterDescription->setText(tr(""));
    else labFilterDescription->setText("");
}


void QF3FilterCombobox::loadFilters() {
    filters.clear();
    QString currentO=cmbFilters->currentText();
    cmbFilters->clear();
    QSettings inifileg(globalfilters, QSettings::IniFormat);
    QSettings inifile(localfilters, QSettings::IniFormat);
    // first read global config
    QStringList groups=inifile.childGroups();
    QStringList groupsg=inifile.childGroups();
    for (int i=0; i<groupsg.size(); i++) {
        if (!groups.contains(groupsg[i])) groups.append(groupsg[i]);
    }
    for (int i=0; i<groups.size(); i++) {
        QString g=groups[i];
        FilterDescription o;
        o.name=inifile.value(g+"/name", inifileg.value(g+"/name", "")).toString();
        o.manufacturer=inifile.value(g+"/manufacturer", inifileg.value(g+"/manufacturer", "")).toString();
        o.type=inifile.value(g+"/type", inifileg.value(g+"/type", "")).toString();
        filters.append(o);
        cmbFilters->addItem(QIcon(":/libqf3widgets/filter.png"), o.name);
    }

    int i=cmbFilters->findText(currentO);
    if (i<0) i=0;
    cmbFilters->setCurrentIndex(i);
}

void QF3FilterCombobox::setFilterINI(QString globalfilters, QString localfilters) {
    this->globalfilters=globalfilters;
    this->localfilters=localfilters;
    loadFilters();
}

void QF3FilterCombobox::storeFilters() {
    QSettings inifileg(globalfilters, QSettings::IniFormat);
    QSettings inifilel(localfilters, QSettings::IniFormat);
    QSettings* inifile=&inifileg;
    if (!localfilters.isEmpty()) {
        inifile=&inifilel;
    }
    if (inifile->isWritable()) {
        inifile->clear();
        for (int i=0; i<filters.size(); i++) {
            FilterDescription o=filters[i];
            QString g="filter"+QString::number(i);
            inifile->setValue(g+"/name", o.name);
            inifile->setValue(g+"/manufacturer", o.manufacturer);
            inifile->setValue(g+"/type", o.type);
        }
        emit m_notifier->emitUpdate();
    }
    loadFilters();
}

void QF3FilterCombobox::deleteFilter() {
    int i=cmbFilters->currentIndex();
    if (i>=0 && i<filters.size()) {
        filters.removeAt(i);
    }
    storeFilters();
    loadFilters();
}


void QF3FilterCombobox::editFilter() {
    int i=cmbFilters->currentIndex();
    if (i>=0 && i<filters.size()) {
        FilterDescription d=filters[i];

        QF3FilterEditor dlg(d, NULL);
        if (dlg.exec()==QDialog::Accepted) {
            filters[i]=dlg.getData();
        }

    }
    storeFilters();
    loadFilters();
}

void QF3FilterCombobox::addFilter() {
    FilterDescription d;
    d.name=tr("new filter");
    QF3FilterEditor dlg(d, NULL);
    if (dlg.exec()==QDialog::Accepted) {
        d=dlg.getData();
        while (filterExists(d.name)) {
            QMessageBox::critical(this, "Filter Editor", "A filter with the same name already exists. Please rename!", QMessageBox::Ok, QMessageBox::Ok);
            if (dlg.exec()!=QDialog::Accepted) return;
            d=dlg.getData();
        }
        //qDebug()<<d.name;
        filters.append(d);
    }
    storeFilters();
    loadFilters();
}

FilterDescription QF3FilterCombobox::getFilterDescription(int i) {
    return filters[i];
}

FilterDescription QF3FilterCombobox::filter() {
    FilterDescription d;
    int i=cmbFilters->currentIndex();
    if (i>=0 && i<filters.size()) return filters[i];
    return d;
}
bool QF3FilterCombobox::filterExists(QString name) {
    for (int i=0; i<filters.size(); i++) {
        if (name==filters[i].name) return true;
    }
    return false;

}

void QF3FilterCombobox::loadSettings(QSettings& settings, QString property) {
    cmbFilters->setCurrentIndex(cmbFilters->findText(settings.value(property, "").toString()));
}

void QF3FilterCombobox::saveSettings(QSettings& settings, QString property) {
    settings.setValue(property, cmbFilters->currentText());
}
