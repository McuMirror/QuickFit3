#include "qfproject.h"
#include "qfrawdatarecord.h"
#include "qfevaluationitem.h"
#include "qfrdrtable.h"
#include "qfrdrfcsdata.h"
#include "qfprojectrawdatamodel.h"
#include "qfprojecttreemodel.h"
#include "qfevaluationitemfactory.h"
#include "qfrawdatarecordfactory.h"



QFProject::QFProject(QObject* parent, QtLogFile* lf, QProgressBar* p):
    QObject(parent), QFProperties()
{
    dataChange=false;
    logF=lf;
    prg=p;
    name="unnamed";
    file="";
    description="";
    creator="";
    highestID=-1;
    rawData.clear();
    errorOcc=false;
    errorDesc="";
    rdModel=new QFProjectRawDataModel(this);
    rdModel->setParent(this);

    treeModel=new QFProjectTreeModel(this);
    treeModel->init(this);
    //std::cout<<QModelIndex().internalId()<<std::endl;
    //addRawData("unknown", "testRawData0");
    //addRawData("fcs", "testRawData1");
    //addRawData("table", "testRawData2");
    //addRawData("table", "testRawData3");
    //addEvaluation("unknown", "eval1");
    //addEvaluation("unknown", "eval2");
}

QFProject::QFProject(QString& filename, QObject* parent, QtLogFile* lf, QProgressBar* p):
    QObject(parent)
{
    logF=lf;
    prg=p;
    dataChange=false;
    name="unnamed";
    file="";
    description="";
    creator="";
    highestID=-1;
    rawData.clear();
    errorOcc=false;
    errorDesc="";
    rdModel=new QFProjectRawDataModel(this);
    rdModel->setParent(this);
    treeModel=new QFProjectTreeModel(this);
    treeModel->init(this);
    readXML(filename);
}

QFProject::~QFProject()
{
    //std::cout<<"deleting QFProject\n";
    QMapIterator<int, QFRawDataRecord*> i(rawData);
    while (i.hasNext()) {
        i.next();
        //emit recordAboutToBeDeleted(i.value());
        delete i.value();
    }
    rawData.clear();
    //std::cout<<"deleting QFProject ... OK\n";
}

QFRawDataRecord* QFProject::getNextRawData(QFRawDataRecord* current) {
    if (!current || rawData.size()<=0) return NULL;
    int i=rawData.values().indexOf(current);
    if (i+1>0 && i+1<rawData.size()) return rawData.values().at(i+1);
    if (i==rawData.size()-1) return rawData.values().at(0);
    return NULL;
}

QFRawDataRecord* QFProject::getPreviousRawData(QFRawDataRecord* current) {
    if (!current || rawData.size()<=0) return NULL;
    int i=rawData.values().indexOf(current);
    if (i-1>=0 && i-1<rawData.size()) return rawData.values().at(i-1);
    if (i==0) return rawData.values().at(rawData.size()-1);
    return NULL;
}

QFRawDataRecord* QFProject::getNextRawDataOfSameType(QFRawDataRecord* current) {
    if (!current || rawData.size()<=0) return NULL;
    int i=rawData.values().indexOf(current);
    QString t=current->getType();
    int cnt=rawData.size();
    for (int k=i+1; k<i+cnt; k++) {
        int l=k%cnt;
        if (rawData.values().at(l)->getType()==t) return rawData.values().at(l);
    }
    return current;
}

QFRawDataRecord* QFProject::getPreviousRawDataOfSameType(QFRawDataRecord* current) {
    if (!current || rawData.size()<=0) return NULL;
    int i=rawData.values().indexOf(current);
    QString t=current->getType();
    int cnt=rawData.size();
    for (int k=i-1+cnt; k>i; k--) {
        int l=k%cnt;
        if (rawData.values().at(l)->getType()==t) return rawData.values().at(l);
    }
    return current;
}

QFEvaluationItem* QFProject::getNextEvaluation(QFEvaluationItem* current) {
    if (!current || evaluations.size()<=0) return NULL;
    int i=evaluations.values().indexOf(current);
    if (i+1>=0 && i+1<evaluations.size()) return evaluations.values().at(i+1);
    if (i==evaluations.size()-1) return evaluations.values().at(0);
    return NULL;
}

QFEvaluationItem* QFProject::getPreviousEvaluation(QFEvaluationItem* current) {
    if (!current || evaluations.size()<=0) return NULL;
    int i=evaluations.values().indexOf(current);
    if (i-1>=0 && i-1<evaluations.size()) return evaluations.values().at(i-1);
    if (i==0) return evaluations.values().at(evaluations.size()-1);
    return NULL;
}

QFEvaluationItem* QFProject::getNextEvaluationOfSameType(QFEvaluationItem* current) {
    if (!current || evaluations.size()<=0) return NULL;
    int i=evaluations.values().indexOf(current);
    QString t=current->getType();
    int cnt=evaluations.size();
    for (int k=i+1; k<i+cnt; k++) {
        int l=k%cnt;
        if (evaluations.values().at(l)->getType()==t) return evaluations.values().at(l);
    }
    return current;
}

QFEvaluationItem* QFProject::getPreviousEvaluationOfSameType(QFEvaluationItem* current) {
    if (!current || evaluations.size()<=0) return NULL;
    int i=evaluations.values().indexOf(current);
    QString t=current->getType();
    int cnt=evaluations.size();
    for (int k=i-1+cnt; k>i; k--) {
        int l=k%cnt;
        if (evaluations.values().at(l)->getType()==t) return evaluations.values().at(l);
    }
    return current;
}

void QFProject::projectChanged() {
    dataChange=true;
}


int QFProject::getNewID() {
    highestID++;
    return highestID;
}

bool QFProject::registerRawDataRecord(QFRawDataRecord* rec) {
    if (!rec) return false;
    if (rawData.contains(rec->getID())) return false;
    if (rec->getID()>highestID) highestID=rec->getID();
    rawData[rec->getID()]=rec;
    IDs.insert(rec->getID());
    connect(rec, SIGNAL(rawDataChanged()), this, SLOT(projectChanged()));
    connect(rec, SIGNAL(propertiesChanged()), this, SLOT(projectChanged()));
    connect(rec, SIGNAL(rawDataChanged()), this, SLOT(setDataChanged()));
    connect(rec, SIGNAL(propertiesChanged()), this, SLOT(setDataChanged()));
    dataChange=true;
    emit wasChanged(dataChange);
    return true;
}

bool QFProject::registerEvaluation(QFEvaluationItem* rec) {
    if (!rec) return false;
    if (evaluations.contains(rec->getID())) return false;
    if (rec->getID()>highestID) highestID=rec->getID();
    evaluations[rec->getID()]=rec;
    IDs.insert(rec->getID());
    connect(rec, SIGNAL(resultsChanged()), this, SLOT(projectChanged()));
    connect(rec, SIGNAL(propertiesChanged()), this, SLOT(projectChanged()));
    connect(rec, SIGNAL(resultsChanged()), this, SLOT(setDataChanged()));
    connect(rec, SIGNAL(propertiesChanged()), this, SLOT(setDataChanged()));
    dataChange=true;
    emit wasChanged(dataChange);
    return true;
}

void QFProject::deleteRawData(int ID) {
    if (rawDataIDExists(ID)) {
        QFRawDataRecord* rec=rawData[ID];
        emit recordAboutToBeDeleted(rec);
        rec->disconnect();
        rawData.remove(ID);
        IDs.remove(ID);
        delete rec;
        dataChange=true;
        emit wasChanged(dataChange);
    }
}

void QFProject::deleteEvaluation(int ID) {
    if (evaluationIDExists(ID)) {
        QFEvaluationItem* rec=evaluations[ID];
        emit evaluationAboutToBeDeleted(rec);
        rec->disconnect();
        evaluations.remove(ID);
        IDs.remove(ID);
        delete rec;
        dataChange=true;
        emit wasChanged(dataChange);
    }
}

void QFProject::writeXML(const QString& file) {

    bool namechanged=(file!=this->file);
    this->file=file;
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError(tr("Could no open file '%1' for output!\n Error description: %2.").arg(file).arg(f.errorString()));
        return;
    }
    QXmlStreamWriter w(&f);
    w.setAutoFormatting(true);
    w.writeStartDocument();
    w.writeStartElement("quickfitproject");
    w.writeAttribute("quickfit_version", "3.0");
    w.writeAttribute("name", name);
    w.writeAttribute("creator", creator);
    w.writeStartElement("description");
    w.writeCDATA(description);
    w.writeEndElement();
    w.writeStartElement("properties");
    storeProperties(w);
    w.writeEndElement();
    w.writeStartElement("rawdata");
    for (int i=0; i<rawData.keys().size(); i++) {
        int k=rawData.keys().at(i);
        rawData[k]->writeXML(w);
    }
    w.writeEndElement();
    w.writeStartElement("evaluations");
    for (int i=0; i<evaluations.keys().size(); i++) {
        std::cout<<"writing eval "<<i<<std::endl;
        int k=evaluations.keys().at(i);
        evaluations[k]->writeXML(w);
    }
    w.writeEndElement();

    w.writeEndElement();
    w.writeEndDocument();
    if (!errorOcc) {
        dataChange=false;
        emit wasChanged(dataChange);
        if (namechanged) emit propertiesChanged();
    }
}

void QFProject::readXML(const QString& file) {
    bool namechanged=(file!=this->file);
    this->file=file;
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setError(tr("Could no open file '%1' for input!\n Error description: %2.").arg(file).arg(f.errorString()));
        return;
    }
    QDomDocument d("quickfitproject");
    QString errorm="";
    if (d.setContent(&f, &errorm)) {
        QDomElement e=d.documentElement();
        if (e.tagName()=="quickfitproject") {
            QString v=e.attribute("quickfit_version");
            if (v.toDouble()>3.0) {
                setError(tr("Error while parsing project file '%1'!\n Error description: The project file was written by a newer version of QuickFit than this one (writer version: %1).").arg(file).arg(v));
                return;
            } else {
                name=e.attribute("name");
                creator=e.attribute("creator");
                QDomElement de=e.firstChildElement("description");
                if (!de.isNull()) {
                    description=de.text();
                }
                //std::cout<<"    reading XML: project properties\n";
                QDomElement te=e.firstChildElement("properties");
                readProperties(te);

                if (prg) {
                    prg->setMaximum(e.firstChildElement("rawdata").elementsByTagName("rawdataelement").size()+e.firstChildElement("evaluations").elementsByTagName("evaluation").size());
                }
                QDomElement rd=e.firstChildElement("rawdata");
                if (!rd.isNull()) {
                    rd=rd.firstChildElement("rawdataelement");
                    while (!rd.isNull()) {
                        if (prg) {
                            prg->setValue(prg->value()+1);
                            QApplication::processEvents();
                        }
                        QString t=rd.attribute("type", "invalid").toLower();
                        //std::cout<<t.toStdString()<<std::endl;
                        try {
                            QFRawDataRecord* e=getRawDataRecordFactory()->createRecord(t, this);
                            e->init(rd);
                        } catch(std::exception& E) {
                            setError(tr("Error while opening raw data element: %2").arg(E.what()));
                        }
                        rd=rd.nextSiblingElement("rawdataelement");
                    }
                }
                rd=e.firstChildElement("evaluations");
                if (!rd.isNull()) {
                    rd=rd.firstChildElement("evaluationelement");
                    while (!rd.isNull()) {
                        if (prg) {
                            prg->setValue(prg->value()+1);
                            QApplication::processEvents();
                        }
                        QString t=rd.attribute("type", "invalid").toLower();

                        try {
                            QFEvaluationItem* e=getEvaluationItemFactory()->createRecord(t, this);
                            e->init(rd);
                        } catch(std::exception& E) {
                            setError(tr("Error while opening raw data element: %2").arg(E.what()));
                        }
                        std::cout<<t.toStdString()<<std::endl;
                        /*if (t=="plot") {

                        } else if (t=="unknown") {
                            //new QFRawDataRecord(rd, this);
                            QFEvaluationItem* e=new QFEvaluationItem(this, true, true);
                            e->init(rd);
                        }*/
                        rd=rd.nextSiblingElement("evaluationelement");
                    }
                }
                if (prg) {
                    prg->reset();
                }
            }
        } else {
            setError(tr("Error while parsing project file '%1'!\n Error description: This is not a QuickFit 3.0 project file.").arg(file));
            return;
        }
    } else {
        setError(tr("Error while parsing project file '%1'!\n Error description: %2.").arg(file).arg(errorm));
        return;
    }
    if (!errorOcc) {
        dataChange=false;
        emit wasChanged(dataChange);
        if (namechanged) emit propertiesChanged();
    }

}

QFRawDataRecord* QFProject::addRawData(QString type, QString name, QStringList inputFiles, QMap<QString, QVariant> initParams, QStringList initParamsReadonly) {
    QString t=type.toLower();
    QFRawDataRecord* rde=NULL;
/*    if (t=="table") {
        QFRDRTable* e=new QFRDRTable(this);
        for (int i=0; i<initParams.keys().size(); i++) {
            e->setProperty(initParams.keys().at(i), initParams[initParams.keys().at(i)], !initParamsReadonly.contains(initParams.keys().at(i)));
        }
        e->init(name, inputFiles);
        rde=e;
    } else if (t=="fcs") {
        QFRDRFCSData* e=new QFRDRFCSData(this);
        for (int i=0; i<initParams.keys().size(); i++) {
            e->setProperty(initParams.keys().at(i), initParams[initParams.keys().at(i)], !initParamsReadonly.contains(initParams.keys().at(i)));
        }
        e->init(name, inputFiles);
        rde=e;
    } else if (t=="unknown") {
        //new QFRawDataRecord(this, name, inputFiles);
        QFRawDataRecord* e=new QFRawDataRecord(this);
        e->init(name, inputFiles);
        rde=e;
    }*/
    rde=getRawDataRecordFactory()->createRecord(type, this);
    if (rde) {
        for (int i=0; i<initParams.keys().size(); i++) {
            rde->setQFProperty(initParams.keys().at(i), initParams[initParams.keys().at(i)], !initParamsReadonly.contains(initParams.keys().at(i)));
        }
        rde->init(name, inputFiles);
    } else {
        setError(tr("error while trying to initialize an object for datatype '%1' in the QuickFit Project '%2'").arg(type).arg(getName()));
        return NULL;
    }


    dataChange=true;
    emit wasChanged(dataChange);
    return rde;
}

QFEvaluationItem* QFProject::addEvaluation(QString type, QString name) {
    QString t=type.toLower();
    QFEvaluationItem* rde=NULL;
    /*if (t=="plot") {
        QFRDRTable* e=new QFRDRTable(this);
        for (int i=0; i<initParams.keys().size(); i++) {
            e->setProperty(initParams.keys().at(i), initParams[initParams.keys().at(i)]);
        }
        e->init(name, inputFiles);
        rde=e;
    } else if (t=="unknown") {
        //new QFRawDataRecord(this, name, inputFiles);
        QFEvaluationItem* e=new QFEvaluationItem(this);
        e->init(name);
        rde=e;
    }*/
    rde=getEvaluationItemFactory()->createRecord(type, this);
    if (rde) {
        rde->init(name);
    } else {
        setError(tr("error while trying to initialize an object for datatype '%1' in the QuickFit Project '%2'").arg(type).arg(getName()));
        return NULL;
    }
    dataChange=true;
    emit wasChanged(dataChange);
    return rde;
}



QStringList QFProject::getAllPropertyNames() {
    QStringList sl;
    for (int i=0; i<getRawDataCount(); i++) {
        sl.append(getRawDataByNum(i)->getPropertyNames());
    }
    sl.removeDuplicates();
    sl.sort();
    return sl;
}
