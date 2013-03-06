#include "qfetcspcimporterjobthread.h"
#include "qfetcspcimporterdialog.h"
#include "qftcspcreader.h"
#include "statistics_tools.h"
#include <stdint.h>
#include "qfimportermanager.h"
#include "binarydatatools.h"

QMutex* QFETCSPCImporterJobThread::mutexFilename=NULL;

// this number has to be dividable by 8192
#define FCS_CHANNELBUFFER_ITEMS (8192*64)

QFETCSPCImporterJobThread::QFETCSPCImporterJobThread(QFPluginServices *services, QObject *parent) :
    QThread(parent)
{
    if (!mutexFilename) mutexFilename=new QMutex();
    outLocale=QLocale::c();
    outLocale.setNumberOptions(QLocale::OmitGroupSeparator);
    m_status=0;
    was_canceled=false;
    duration=0;
    this->pluginServices=services;
}

QFETCSPCImporterJobThread::~QFETCSPCImporterJobThread() {
}

double QFETCSPCImporterJobThread::durationMS() const {
    return duration;
}

double QFETCSPCImporterJobThread::durationS() const {
    return duration;
}

QList<QPair<QStringList, QString> > QFETCSPCImporterJobThread::getAddFiles() const {
    return addFiles;
}

TCSPCImporterJob QFETCSPCImporterJobThread::getJob() const {
    return job;
}

QStringList QFETCSPCImporterJobThread::getImporterFilterList(QFPluginServices* pluginServices)  {
    QStringList l;
    int i=0;
    QFTCSPCReader* r=NULL;
    while ((r=getImporter(i, pluginServices))!=NULL) {
        l.append(r->filter());
        delete r;
        i++;
    }
    return l;
}

QStringList QFETCSPCImporterJobThread::getImporterFormatNameList(QFPluginServices *pluginServices)  {
     QStringList l;
     int i=0;
     QFTCSPCReader* r=NULL;
     while ((r=getImporter(i, pluginServices))!=NULL) {
         l.append(r->formatName());
         delete r;
         i++;
     }
     return l;
}

QFTCSPCReader *QFETCSPCImporterJobThread::getImporter(int idx, QFPluginServices *pluginservices)  {
    QFTCSPCReader* r=NULL;

    QStringList imp=pluginservices->getImporterManager()->getImporters<QFTCSPCReader*>();

    if (idx>=0 && idx<imp.size()) {
        r=dynamic_cast<QFTCSPCReader*>(pluginservices->getImporterManager()->createImporter(imp[idx]));
    }

    return r;

}

int QFETCSPCImporterJobThread::getImporterCount(QFPluginServices *pluginservices) {
    QStringList imp=pluginservices->getImporterManager()->getImporters<QFTCSPCReader*>();
    return imp.size();

}

int QFETCSPCImporterJobThread::status() const {
    return m_status;
}

void QFETCSPCImporterJobThread::cancel() {
    was_canceled=true;
    if (!isRunning()) {
        m_status=-1;
        emit messageChanged(tr("canceled by user"));
        emit statusChanged(m_status);
    }
}

void QFETCSPCImporterJobThread::init(TCSPCImporterJob job) {
    this->job=job;
    m_status=0;
    was_canceled=false;
    addFiles.clear();
}

QString QFETCSPCImporterJobThread::replacePostfixSpecials(const QString& input, int counter) const {
    QString cnt="";
    if (counter>0) cnt=QString::number(counter);
    QString result=input;
    result=result.replace("%counter%", cnt, Qt::CaseInsensitive);
    result=result.replace("%s%", QString::number(job.fcs_S), Qt::CaseInsensitive);
    result=result.replace("%p%", QString::number(job.fcs_P), Qt::CaseInsensitive);
    result=result.replace("%m%", QString::number(job.fcs_m), Qt::CaseInsensitive);
    result=result.replace("%start%", QString::number(job.range_min), Qt::CaseInsensitive);
    result=result.replace("%end%", QString::number(job.range_max), Qt::CaseInsensitive);
    result=result.replace("%fcs_segments%", QString::number(job.fcs_segments), Qt::CaseInsensitive);
    result=result.replace("%fcs_taumin%", QString::number(job.fcs_taumin), Qt::CaseInsensitive);
    result=result.replace("%cr_binning%", QString::number(job.countrate_binning), Qt::CaseInsensitive);

    QString corr="unknown";
    /*if (job.correlator==CORRELATOR_DIRECT) corr="direct";
    if (job.correlator==CORRELATOR_DIRECTAVG) corr="directavg";*/
    if (job.fcs_correlator==CORRELATOR_MTAUALLMON) corr="mtauallmon";
    if (job.fcs_correlator==CORRELATOR_MTAUONEMON) corr="mtauonemon";

    result=result.replace("%correlator%", corr, Qt::CaseInsensitive);
    result=result.replace("%correlatorid%", QString::number(job.fcs_correlator), Qt::CaseInsensitive);
    return result;
}

void QFETCSPCImporterJobThread::run() {
    QTime ptime;

    ptime.start();

    starttime=0;
    range_duration=0;
    file_duration=0;
    channels=0;

    emit messageChanged(tr("loading data ..."));
    if ((job.fileFormat<0)||(job.fileFormat>=getImporterCount(pluginServices))) {
        m_status=-1;
        emit statusChanged(m_status);
        emit messageChanged(tr("file format not supported or given ... format given was: %1").arg(job.fileFormat));
        emit rangeChanged(0,100);
        emit progressChanged(0);
    } else if (!QFile::exists(job.filename)) {
        m_status=-1;
        emit statusChanged(m_status);
        emit messageChanged(tr("file does not exist ..."));
        emit rangeChanged(0,100);
        emit progressChanged(0);
    } else {
        m_status=1;
        emit statusChanged(m_status);
        emit rangeChanged(0,1040);


        reader=NULL;
        bool OK=false;
        if (job.fileFormat>=0 && job.fileFormat<getImporterCount(pluginServices)) {
            reader=getImporter(job.fileFormat, pluginServices);
        }
        if (reader) {
            emit messageChanged(tr("opening %1 file ...").arg(reader->formatName()));
            OK=reader->open(job.filename);
            if (!OK) {
                m_status=-1; emit statusChanged(m_status);
                if (reader) messageChanged(tr("error opening file '%1': %2").arg(job.filename).arg(reader->lastError()));
                else emit messageChanged(tr("error opening file '%1'").arg(job.filename));
            } else {
                emit progressIncrement(10);
                ////////////////////////////////////////////////////////////////////////////////////////////
                // CREATE FILENAMES FOR RESULTS AND MAKE SURE THE DIRECTORY FOR THE FILES EXISTS (mkpath() )
                ////////////////////////////////////////////////////////////////////////////////////////////
                QDir d=QFileInfo(job.filename).dir();

                if (mutexFilename) mutexFilename->lock();
                int counter=-1;
                do {
                    outputFilenameBase=d.absoluteFilePath(replacePostfixSpecials(job.prefix)+QFileInfo(job.filename).completeBaseName()+replacePostfixSpecials(job.postfix, counter));
                    counter++;
                } while (QFile::exists(outputFilenameBase+".evalsettings.txt"));
                //qDebug()<<outputFilenameBase;
                QString configFilename=outputFilenameBase+".evalsettings.txt";
                QString crFilenameBin=outputFilenameBase+".photoncounts.dat";
                QList<QFETCSPCImporterJobThread::ccfFileConfig> ccfFilenames;

                QString localFileDirectory=QFileInfo(d.absoluteFilePath(configFilename)).dir().absolutePath();

                if (d.mkpath(localFileDirectory)) {
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // TOUCH OUTPUT FILE (.evalsettings.txt)
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    emit messageChanged(tr("preparing data processing ..."));
                    emit progressIncrement(10);
                    touchFile(configFilename);

                    if (mutexFilename) mutexFilename->unlock();



                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // DETERMINE EVALUATION SETTINGS
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    channels=reader->inputChannels();
                    file_duration=reader->measurementDuration();
                    starttime=0;
                    if (job.range_min>0) starttime=qMin(job.range_min, file_duration);
                    range_duration=file_duration-starttime;
                    if (job.range_max>0) range_duration=qMin(job.range_max, file_duration)-starttime;
                    if (range_duration<0) range_duration=0;
                    countrate_items=0;

                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // OPEN COUNTRATE FILE
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    QFile crFile(crFilenameBin);
                    if (job.doCountrate) {
                        countrate_items=uint64_t(ceil(range_duration/job.countrate_binning));
                        if (crFile.open(QIODevice::WriteOnly)) {
                            addFiles.append(qMakePair(QStringList(crFilenameBin), QString("photoncounts_binary")));
                            crFile.write("QF3.0CNTRT");
                            binfileWriteUint16(crFile, job.countrate_channels.size());
                            binfileWriteUint64(crFile, countrate_items);
                            binfileWriteDouble(crFile, job.countrate_binning);
                            for (uint16_t i=0; i<channels; i++) {
                                if (job.countrate_channels.contains(i)) {
                                    binfileWriteUint16(crFile, i+1);
                                }
                            }
                        } else {
                            m_status=-1; emit statusChanged(m_status);
                            emit messageChanged(tr("could not create countrate file '%1': %2!").arg(crFilenameBin).arg(crFile.errorString()));
                            crFile.close();
                        }
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // CONFIGURE FCS EVALUATION
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    if (job.doFCS) {
                        createCorrelators();
                    }


                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // RUN THE EVALUATION
                    ////////////////////////////////////////////////////////////////////////////////////////////
                    emit messageChanged(tr("running data processing ..."));
                    emit progressIncrement(10);
                    if ((m_status==1) && !was_canceled) {
                        runEval(reader, &crFile);

                        if (crFile.isOpen()) {
                            // write real measurement parameters!
                            crFile.seek(0);
                            crFile.write("QF3.0CNTRT");
                            binfileWriteUint16(crFile, job.countrate_channels.size());
                            binfileWriteUint64(crFile, real_countrate_items);

                            crFile.close();
                        }
                    }



                    ////////////////////////////////////////////////////////////////////////////////////////////
                    // SAVE THE RESULTS
                    ////////////////////////////////////////////////////////////////////////////////////////////

                    //************** SAVE FCS
                    if ((m_status==1) && !was_canceled && job.doFCS ) {

                        emit messageChanged(tr("saving FCS results ..."));

                        for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
                             QPair<int, int> ccf=*i;

                             QString localFilename=outputFilenameBase+QString(".ccf%1_%2.csv").arg(ccf.first).arg(ccf.second);
                             if (ccf.first==ccf.second) localFilename=outputFilenameBase+QString(".acf%1.csv").arg(ccf.first);

                             QFile f(localFilename);
                             if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
                                 QTextStream text(&f);
                                 text.setLocale(outLocale);
                                 for (int i=0; i<fcs_tau.size(); i++) {
                                     text<<outLocale.toString(fcs_tau[i]);
                                     for (int r=0; r<job.fcs_segments; r++) {
                                         uint64_t id=xyzAdressToUInt64(ccf.first, ccf.second, r);
                                         text<<", "<<outLocale.toString(fcs_ccfs[id].value(i, 0));
                                     }
                                     text<<"\n";
                                 }
                             }
                             QStringList localFilenameCR;
                             if (ccf.first==ccf.second) {
                                 localFilenameCR<<outputFilenameBase+QString(".fcscnt%1.csv").arg(ccf.first);
                                 QFile f(localFilenameCR[0]);
                                 if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
                                     QTextStream text(&f);
                                     text.setLocale(outLocale);
                                     int items=0;
                                     for (int r=0; r<job.fcs_segments; r++) {
                                         uint32_t id=xyAdressToUInt32(r, ccf.first);
                                         if (r==0) items=fcs_crs[id].size();
                                         else items=qMax(items, fcs_crs[id].size());
                                     }
                                     for (int i=0; i<items; i++) {
                                         text<<outLocale.toString(double(i)*job.fcs_crbinning);
                                         for (int r=0; r<job.fcs_segments; r++) {
                                             uint32_t id=xyAdressToUInt32(r, ccf.first);
                                             text<<", "<<outLocale.toString(fcs_crs[id].value(i, 0));
                                         }
                                         text<<"\n";
                                     }
                                 } else {
                                     localFilenameCR.clear();
                                 }
                             } else {
                                 localFilenameCR<<outputFilenameBase+QString(".fcscnt%1_%2_1.csv").arg(ccf.first).arg(ccf.second);
                                 localFilenameCR<<outputFilenameBase+QString(".fcscnt%1_%2_2.csv").arg(ccf.first).arg(ccf.second);
                                 {
                                     QFile f(localFilenameCR[0]);
                                     if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
                                         QTextStream text(&f);
                                         text.setLocale(outLocale);
                                         int items=0;
                                         for (int r=0; r<job.fcs_segments; r++) {
                                             uint32_t id=xyAdressToUInt32(r, ccf.first);
                                             if (r==0) items=fcs_crs[id].size();
                                             else items=qMax(items, fcs_crs[id].size());
                                         }
                                         for (int i=0; i<items; i++) {
                                             text<<outLocale.toString(double(i)*job.fcs_crbinning);
                                             for (int r=0; r<job.fcs_segments; r++) {
                                                 uint32_t id=xyAdressToUInt32(r, ccf.first);
                                                 text<<", "<<outLocale.toString(fcs_crs[id].value(i, 0));
                                             }
                                             text<<"\n";
                                         }
                                     } else {
                                         localFilenameCR.removeAt(0);
                                     }
                                 }
                                 {
                                     QFile f(localFilenameCR[localFilenameCR.size()-1]);
                                     if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
                                         QTextStream text(&f);
                                         text.setLocale(outLocale);
                                         int items=0;
                                         for (int r=0; r<job.fcs_segments; r++) {
                                             uint32_t id=xyAdressToUInt32(r, ccf.second);
                                             if (r==0) items=fcs_crs[id].size();
                                             else items=qMax(items, fcs_crs[id].size());
                                         }
                                         for (int i=0; i<items; i++) {
                                             text<<outLocale.toString(double(i)*job.fcs_crbinning);
                                             for (int r=0; r<job.fcs_segments; r++) {
                                                 uint32_t id=xyAdressToUInt32(r, ccf.second);
                                                 text<<", "<<outLocale.toString(fcs_crs[id].value(i, 0));
                                             }
                                             text<<"\n";
                                         }
                                     } else {
                                         localFilenameCR.removeAt(localFilenameCR.size()-1);
                                     }
                                 }
                             }

                             QFETCSPCImporterJobThread::ccfFileConfig fn;
                             fn.filename=localFilename;
                             fn.channel1=ccf.first;
                             fn.channel2=ccf.second;
                             fn.filenameCR=localFilenameCR;
                             ccfFilenames.append(fn);
                             if (ccf.first==ccf.second) {
                                 if (localFilenameCR.isEmpty()) {
                                     addFiles.append(qMakePair(QStringList(localFilename), QString("fcs_csv")));
                                 } else {
                                     QStringList sl;
                                     sl<<localFilename<<localFilenameCR;
                                     addFiles.append(qMakePair(sl, QString("fcs_csv")));
                                 }
                             } else {
                                 QStringList sl;
                                 sl<<localFilename<<localFilenameCR;
                                 addFiles.append(qMakePair(sl, QString("fcs_cross_csv")));
                             }

                        }

                    }
                    emit progressIncrement(10);

                    //************** SAVE SETTINGS
                    if ((m_status==1) && !was_canceled) {
                        emit messageChanged(tr("saving settings ..."));
                        QString& localFilename=configFilename;
                        QFile f(localFilename);
                        QDir d=QFileInfo(localFilename).absoluteDir();
                        if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
                            QTextStream text(&f);
                            text.setLocale(outLocale);

                            text<<"date/time                   : "<<QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << "\n";
                            text<<"input file                  : "<<d.relativeFilePath(job.filename) << "\n";
                            text<<"input filetype              : "<<d.relativeFilePath(reader->formatName()) << "\n";
                            //if (!acfFilename.isEmpty())             text<<"autocorrelation file        : " << d.relativeFilePath(acfFilename) << "\n";
                            //if (!acfFilenameBin.isEmpty())          text<<"bin. autocorrelation file   : " << d.relativeFilePath(acfFilenameBin) << "\n";
                            text<<"range start                 : "<<outLocale.toString(starttime) << "\n";
                            text<<"range duration              : "<<outLocale.toString(range_duration) << "\n";
                            if (job.doFCS) {
                                text<<"FCS: segments               : "<<outLocale.toString(job.fcs_segments) << "\n";
                                text<<"FCS: correlator S           : "<<outLocale.toString(job.fcs_S) << "\n";
                                text<<"FCS: correlator m           : "<<outLocale.toString(job.fcs_m) << "\n";
                                text<<"FCS: correlator P           : "<<outLocale.toString(job.fcs_P) << "\n";
                                text<<"FCS: correlator type        : "<<outLocale.toString(job.fcs_correlator) << "\n";
                                text<<"FCS: correlator type name   : ";
                                switch(job.fcs_correlator) {
                                    case CORRELATOR_MTAUALLMON:  text<<"multi-tau with monitors for all channels\n"; break;
                                    case CORRELATOR_MTAUONEMON:  text<<"multi-tau with a single monitor\n"; break;

                                    default: text<<"FCS: correlator type name        : unknown\n"; break;
                                }
                                text<<"FCS: smallest tau [s]            : "<<outLocale.toString(job.fcs_taumin) << "\n";
                                text<<"FCS: count rate binning [s]      : "<<outLocale.toString(job.fcs_crbinning) << "\n";
                                for (int i=0; i<ccfFilenames.size(); i++) {
                                    if (!ccfFilenames[i].filename.isEmpty()) {
                                        if (ccfFilenames[i].channel1==ccfFilenames[i].channel2) text<<QString("FCS: acf %1 file               : ").arg(ccfFilenames[i].channel1,4)<<ccfFilenames[i].filename << "\n";
                                        else text<<QString("FCS: ccf %1 %2 file          : ").arg(ccfFilenames[i].channel1,4).arg(ccfFilenames[i].channel2,4)<<ccfFilenames[i].filename << "\n";
                                    }
                                    if (!ccfFilenames[i].filenameCR.isEmpty()) {
                                        if (ccfFilenames[i].channel1==ccfFilenames[i].channel2) text<<QString("FCS: acf %1 countrate file     : ").arg(ccfFilenames[i].channel1,4)<<ccfFilenames[i].filenameCR.value(0, "") << "\n";
                                        else{
                                            text<<QString("FCS: ccf %1 %2 countrate file 1: ").arg(ccfFilenames[i].channel1,4).arg(ccfFilenames[i].channel2,4)<<ccfFilenames[i].filenameCR.value(0, "") << "\n";
                                            text<<QString("FCS: ccf %1 %2 countrate file 2: ").arg(ccfFilenames[i].channel1,4).arg(ccfFilenames[i].channel2,4)<<ccfFilenames[i].filenameCR.value(1, "") << "\n";
                                        }
                                    }
                                }
                            }
                            if (job.doCountrate) {
                                text<<"PhotonsCounts: binning [s]       : "<<outLocale.toString(job.countrate_binning) << "\n";
                            }
                            text<<"duration [s]                : "<<ptime.elapsed()/1000.0 << "\n";

                            f.close();
                        } else {
                            m_status=-1; emit statusChanged(m_status);
                            emit messageChanged(tr("could not create settings file '%1': %2!").arg(localFilename).arg(f.errorString()));
                        }

                    }
                    emit progressIncrement(10);
                } else {
                    if (mutexFilename) mutexFilename->unlock();
                    m_status=-1; emit statusChanged(m_status);
                    emit messageChanged(tr("could not create output subdirectory '%1' in '%2'!").arg(localFileDirectory).arg(d.absolutePath()));
                }
                if (m_status==1) {
                    if (was_canceled) {
                        m_status=-1; emit statusChanged(m_status);
                        duration=ptime.elapsed();
                        emit messageChanged(tr("canceled by user"));
                    } else {
                        m_status=2; emit statusChanged(m_status);
                        duration=ptime.elapsed();
                        emit messageChanged(tr("correlation ... done [duration = %1 s]").arg(duration/1.0e3));
                    }
                    emit statusChanged(m_status);
                }


                reader->close();
            }
            delete reader;
        } else {
            m_status=-1; emit statusChanged(m_status);
            duration=ptime.elapsed();
            emit messageChanged(tr("could not create image reader object"));
        }
    }


    duration=ptime.elapsed();
}

void QFETCSPCImporterJobThread::runEval(QFTCSPCReader *reader,  QFile* countrateFile) {
    double nextcrInterval=job.countrate_binning;
    uint16_t* countrate=(uint16_t*)calloc(channels, sizeof(uint16_t));
    uint16_t* fcs_countrate=NULL;
    if (job.doFCS) fcs_countrate=(uint16_t*)calloc(channels*FCS_CHANNELBUFFER_ITEMS, sizeof(uint16_t));
    uint16_t* fcs_storecountrate=(uint16_t*)calloc(channels, sizeof(uint16_t));
    uint64_t crCounter=0;
    uint32_t fcs_countrate_counter=0;
    double pos=0;
    double fcsNextSegmentValue=range_duration/double(job.fcs_segments);
    double fcsNextInterval=job.fcs_taumin;
    double fcsNextStoreInterval=job.fcs_crbinning;
    double nextReporterStep=range_duration/1000.0;
    uint16_t fcs_segment=0;
    fcs_ccfs.clear();
    real_countrate_items=0;
    do {
        QFTCSPCRecord record=reader->getCurrentRecord();
        const register double t=record.absoluteTime()-starttime;
        const register int c=record.input_channel;
        //qDebug()<<c<<t;
        if (record.isPhoton && t>=0 && t<=range_duration) {
            real_duration=t;

            // PROCESS COUNTRATE
            if (job.doCountrate) {
                if (t<nextcrInterval) {
                    countrate[c]++;
                } else if (crCounter<countrate_items) {
                    uint64_t emptyrecords=uint64_t(qMax(int64_t(0),(int64_t)floor((t-nextcrInterval)/job.countrate_binning)+1));
                    //qDebug()<<emptyrecords;
                    for (register uint64_t i=0; i<channels*emptyrecords; i++) {
                        int c=i%channels;
                        if (job.countrate_channels.contains(c)) binfileWriteUint16(*countrateFile, countrate[c]);
                        countrate[c]=0;
                    }
                    real_countrate_items+=emptyrecords;

                    countrate[c]++;
                    crCounter+=emptyrecords;
                    nextcrInterval=nextcrInterval+emptyrecords*job.countrate_binning;
                    if (crCounter>=countrate_items) nextcrInterval=2*range_duration;
                }
            }

            // PROCESS FCS
            if (job.doFCS) {
                // binning and processing of ccfs
                if (t<fcsNextInterval) {
                    fcs_countrate[fcs_countrate_counter+c*FCS_CHANNELBUFFER_ITEMS]++;
                } else {
                    const int64_t emptyrecords=(int64_t)floor((t-fcsNextInterval)/job.fcs_taumin)+1;
                    fcs_countrate_counter=fcs_countrate_counter+emptyrecords;

                    if (fcs_countrate_counter>=FCS_CHANNELBUFFER_ITEMS) {
                        //qDebug()<<"  loop shift @t="<<t<<"   emptyrecords="<<emptyrecords<<"  fcs_countrate_counter="<<fcs_countrate_counter;
                        shiftIntoCorrelators(fcs_countrate, FCS_CHANNELBUFFER_ITEMS);

                        memset(fcs_countrate, 0, channels*FCS_CHANNELBUFFER_ITEMS*sizeof(uint16_t));
                        fcs_countrate_counter=fcs_countrate_counter-FCS_CHANNELBUFFER_ITEMS;
                    }
                    //qDebug()<<emptyrecords;
/*                    for (register uint64_t r=0; r<emptyrecords+1; r++) {
                        //if (job.countrate_channels.contains(c)) binfileWriteUint16(*countrateFile, countrate[c]);
                        if (job.fcs_correlator==CORRELATOR_MTAUALLMON) {
                            for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
                                 QPair<int, int> ccf=*i;
                                 uint32_t id=xyAdressToUInt32(ccf.first, ccf.second);
                                 corrjk[id]->crosscorrelate_step(double(fcs_countrate[ccf.first]), double(fcs_countrate[ccf.second]));
                            }

                        } else if (job.fcs_correlator==CORRELATOR_MTAUONEMON) {
                            for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
                                 QPair<int, int> ccf=*i;
                                 uint32_t id=xyAdressToUInt32(ccf.first, ccf.second);
                                 corrjb[id]->run(fcs_countrate[ccf.first], fcs_countrate[ccf.second]);
                            }

                        }
                        for (register int cc=0; cc<channels; cc++) fcs_countrate[cc]=0;
                    }*/

                    fcs_countrate[fcs_countrate_counter+c*FCS_CHANNELBUFFER_ITEMS]++;
                    fcsNextInterval=fcsNextInterval+emptyrecords*job.fcs_taumin;
                }

                // binning and processing of countrates, stored with ccfs
                if (t<fcsNextStoreInterval) {
                    fcs_storecountrate[c]++;
                } else {
                    int64_t emptyrecords=(int64_t)floor((t-fcsNextStoreInterval)/job.fcs_crbinning)+1;
                    //qDebug()<<emptyrecords;
                    for (register uint64_t i=0; i<channels*emptyrecords; i++) {
                        int cc=i%channels;
                        fcs_crs[xyAdressToUInt32(fcs_segment, cc)].append(fcs_storecountrate[cc]/job.fcs_crbinning/1000.0);
                        fcs_storecountrate[cc]=0;
                    }

                    fcs_storecountrate[c]++;
                    crCounter+=emptyrecords;
                    fcsNextStoreInterval=fcsNextStoreInterval+emptyrecords*job.fcs_crbinning;
                }

                if (t>=fcsNextSegmentValue) {
                    if (fcs_countrate_counter>0) shiftIntoCorrelators(fcs_countrate, fcs_countrate_counter);
                    copyCorrelatorIntermediateResults(fcs_segment);
                    //qDebug()<<"-- fcs segment "<<fcs_segment<<" @ t="<<t<<" -----------------------------";
                    for (register int cc=0; cc<channels; cc++) {
                        fcs_countrate[cc]=0;
                        fcs_storecountrate[cc]=0;
                    }
                    createCorrelators();
                    memset(fcs_countrate, 0, channels*FCS_CHANNELBUFFER_ITEMS*sizeof(uint16_t));
                    fcs_countrate_counter=0;
                    fcs_segment++;
                    fcsNextSegmentValue=fcsNextSegmentValue+range_duration/double(job.fcs_segments);
                    //qDebug()<<"-- fcsNextSegmentValue "<<fcsNextSegmentValue;
                }
            }
        }
        if (reader->percentCompleted()-pos>0.1) {
            emit progressIncrement(1);
            pos=reader->percentCompleted();
        }

        if (t>nextReporterStep) {
            emit progressIncrement(1);
            nextReporterStep=nextReporterStep+range_duration/1000.0;
            emit messageChanged(tr("running data processing [t=%1s, runtime: %2s] ...").arg(t).arg(range_duration));
        }

    } while (reader->nextRecord() && (m_status==1) && (!was_canceled));
    if (job.doFCS) {
        if (fcs_countrate_counter>0) shiftIntoCorrelators(fcs_countrate, fcs_countrate_counter);
        while (fcs_segment<job.fcs_segments) {
            //qDebug()<<"-- fcs segment "<<fcs_segment<<" -----------------------------";
            copyCorrelatorIntermediateResults(fcs_segment);
            fcs_segment++;
        }
        free(fcs_storecountrate);
    }
    free(countrate);
    free(fcs_countrate);
}

void QFETCSPCImporterJobThread::clearCorrelators() {
    //qDebug()<<"clearCorrelators() ... ";
    QMap<uint32_t, corrjb_type*>::iterator i;
    for (i=corrjb.begin(); i!=corrjb.end(); ++i) {
        delete i.value();
    }
    corrjb.clear();

    QMap<uint32_t, corrjk_type*>::iterator ii;
    for (ii=corrjk.begin(); ii!=corrjk.end(); ++ii) {
        delete ii.value();
    }
    corrjk.clear();
    //qDebug()<<"clearCorrelators() ... DONE";
}

void QFETCSPCImporterJobThread::createCorrelators() {
    //qDebug()<<"createCorrelators() ... ";
    clearCorrelators();
    fcs_tau.clear();
    for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
         QPair<int, int> ccf=*i;
         uint32_t id=xyAdressToUInt32(ccf.first, ccf.second);
         if (job.fcs_correlator==CORRELATOR_MTAUALLMON) {
             corrjk[id]=new corrjk_type(job.fcs_S, job.fcs_m, job.fcs_P, job.fcs_taumin);
             if (fcs_tau.isEmpty()) {
                 fcs_tau.clear();
                 double* tau=corrjk[id]->getCorTau();
                 uint32_t cslots=corrjk[id]->getSlots();
                 for (uint32_t s=0; s<cslots; s++) {
                     fcs_tau.append(tau[s]);
                 }
             }

         } else if (job.fcs_correlator==CORRELATOR_MTAUONEMON) {
             corrjb[id]=new corrjb_type(job.fcs_S, job.fcs_P, 0.0);
             if (fcs_tau.isEmpty()) {
                 fcs_tau.clear();
                 double** corr1=corrjb[id]->get_array_G();
                 for (int j=0; j<job.fcs_S*job.fcs_P; j++) {
                     fcs_tau.append(corr1[0][j]*job.fcs_taumin);
                 }
                 free(corr1[0]);
                 free(corr1[1]);
                 free(corr1);
             }
         }

    }
    //qDebug()<<"createCorrelators() ... DONE";
}

void QFETCSPCImporterJobThread::copyCorrelatorIntermediateResults(uint16_t fcs_segment) {
    //qDebug()<<"copyCorrelatorIntermediateResults("<<fcs_segment<<") ... ";

    if (job.fcs_correlator==CORRELATOR_MTAUALLMON) {
        for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
             QPair<int, int> ccf=*i;
             //qDebug()<<ccf.first<<ccf.second<<fcs_segment;
             uint64_t id=xyzAdressToUInt64(ccf.first, ccf.second, fcs_segment);
             uint32_t idc=xyAdressToUInt32(ccf.first, ccf.second);
             corrjk[idc]->crossnormalize();
             //qDebug()<<QString::number(id, 16)<<QString::number(idc, 16);
             double* data=corrjk[idc]->getCor();
             //qDebug()<<"got data";
             fcs_ccfs[id]=QVector<double>();
             //qDebug()<<"added empty vector";
             for (int tau=0; tau<qMin(int(corrjk[idc]->getSlots()), fcs_tau.size()); tau++) {
                 fcs_ccfs[id].append(data[tau]-1.0);
             }
             //qDebug()<<"appended data";
        }

    } else if (job.fcs_correlator==CORRELATOR_MTAUONEMON) {
        for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
             QPair<int, int> ccf=*i;
             //qDebug()<<ccf.first<<ccf.second<<fcs_segment;
             uint64_t id=xyzAdressToUInt64(ccf.first, ccf.second, fcs_segment);
             uint32_t idc=xyAdressToUInt32(ccf.first, ccf.second);
             double** data=corrjb[idc]->get_array_G();
             fcs_ccfs[id]=QVector<double>();
             for (int tau=0; tau<qMin(job.fcs_P*job.fcs_S, fcs_tau.size()); tau++) {
                 fcs_ccfs[id].append(data[1][tau]-1.0);
             }
             free(data[0]);
             free(data[1]);
             free(data);
        }

    }
    //qDebug()<<"copyCorrelatorIntermediateResults("<<fcs_segment<<") ... DONE";

}


void QFETCSPCImporterJobThread::shiftIntoCorrelators(uint16_t* fcs_countrate, uint32_t count) {
    //qDebug()<<"shift "<<count<<" items ... ";
    if (job.fcs_correlator==CORRELATOR_MTAUALLMON) {
        for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
             QPair<int, int> ccf=*i;
             uint32_t id=xyAdressToUInt32(ccf.first, ccf.second);
             corrjk_type* corr=corrjk[id];
             for (uint32_t ci=0; ci<count; ci+=8192) {
                 corr->crosscorrelate_series(&(fcs_countrate[ccf.first*FCS_CHANNELBUFFER_ITEMS+ci]), &(fcs_countrate[ccf.second*FCS_CHANNELBUFFER_ITEMS+ci]), 8192);
                 if (was_canceled) break;
             }
        }

    } else if (job.fcs_correlator==CORRELATOR_MTAUONEMON) {
        for (QSet<QPair<int, int> >::iterator i = job.fcs_correlate.begin(); i != job.fcs_correlate.end(); ++i) {
             QPair<int, int> ccf=*i;
             uint32_t id=xyAdressToUInt32(ccf.first, ccf.second);
             corrjb_type* corr=corrjb[id];
             for (register uint32_t ci=0; ci<count; ci++) {
                 corr->run(fcs_countrate[ccf.first*FCS_CHANNELBUFFER_ITEMS+ci], fcs_countrate[ccf.second*FCS_CHANNELBUFFER_ITEMS+ci]);
                 if (was_canceled) break;
             }
        }

    }
    //qDebug()<<"shift "<<count<<" items ... DONE!";
}

