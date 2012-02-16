#include "qfrdrimagingfcs_data.h"
#include <QtXml>
#include "libtiff_tools.h"
#include "qfrdrimagereaderrh.h"
#include "csvtools.h"
#include <QTextStream>
#include "tools.h"
#include "qfrdrimagingfcstools.h"

//#define DEBUG_SIZES
#undef DEBUG_SIZES
#define DEBUG_TIMING

QFRDRImagingFCSData::QFRDRImagingFCSData(QFProject* parent):
    QFRawDataRecord(parent)
{
    correlations=NULL;
    correlationMean=NULL;
    correlationStdDev=NULL;
    sigmas=NULL;
    tau=NULL;
    N=0;
    width=0;
    height=0;
    overview=NULL;
    overviewF=NULL;
    leaveout=NULL;
    statAvg=NULL;
    statStdDev=NULL;
    statMin=NULL;
    statMax=NULL;
    statT=NULL;
    statN=0;
    video=NULL;
    video_width=0;
    video_height=0;
    video_frames=0;
    setResultsInitSize(1000);
    setEvaluationIDMetadataInitSize(1000);
}

QFRDRImagingFCSData::~QFRDRImagingFCSData() {
     allocateContents(0,0,0);
     allocateStatistics(0);
     clearOvrImages();
     if (video) free(video);
     video=NULL;
     video_width=video_height=video_frames=0;
}

QString QFRDRImagingFCSData::getEditorName(int i) {
    if (i==0) return tr("Parameter Image");
    if (i==1) return tr("Correlation Curves");
    if (i==2) return tr("Images, Videos & Timetrace");
    return QString("");
};

QFRawDataEditor* QFRDRImagingFCSData::createEditor(QFPluginServices* services, int i, QWidget* parent) {
    if (i==0) return new QFRDRImagingFCSImageEditor(services, parent);
    if (i==1) return new QFRDRImagingFCSDataEditor(services, parent);
    if (i==2) return new QFRDRImagingFCSOverviewRateEditor(services, parent);
    return NULL;
};


QStringList QFRDRImagingFCSData::getExportFiletypes() {
    QStringList sl;
    return sl;
};

QString QFRDRImagingFCSData::getExportDialogTitle() {
    return tr("");
}

QString QFRDRImagingFCSData::getExportDialogFiletypes() {
    return tr("");
}

void QFRDRImagingFCSData::exportData(const QString& format, const QString& filename)const  {
	// here you may export the data of the record into the specified format (see getExportFiletypes() )
	// THIS IS OPTIONAL
}


void QFRDRImagingFCSData::intWriteData(QXmlStreamWriter& w) {
	// write data to the project XML file using the QXmlStreamWriter
    /*if (leaveout.size()>0) {
        QString l="";
        for (int i=0; i<leaveout.size(); i++) {
            if (!l.isEmpty()) l=l+",";
            l=l+QString::number(leaveout[i]);
        }
        w.writeStartElement("leaveout");
        w.writeAttribute("list", l);
        w.writeEndElement();
    }*/
    QString l="";
    for (int i=0; i<getCorrelationRuns(); i++) {
        if (leaveout[i]!=0) {
            if (!l.isEmpty()) l=l+",";
            l=l+QString::number(i);
        }
    }
    if (l.size()>0) {
        w.writeStartElement("leaveout");
        w.writeAttribute("list", l);
        w.writeEndElement();
    }
}

void QFRDRImagingFCSData::intReadData(QDomElement* e) {
    leaveoutClear();
	// read data from the project XML file



    width=getProperty("WIDTH", 0).toInt();
    height=getProperty("HEIGHT", 0).toInt();
    QString filetype=getProperty("FILETYPE", "unknown").toString();

    // now also load the data file(s) this record is linked to
    // an error is reported when no file is given!
    if (files.size()<=0) {
        setError(tr("there are no files in the %1 record!").arg(getName()));
        return;
    }
    bool dataLoaded=false;
    QString acquisitionSettingsFile=getFileForType("acquisition_settings");
    if (files.size()>0) {
        // now we check whether the experiment config file contains additional files, that may be useful for this object and load them
        // (more overview images ...)
        //qDebug()<<"opening qcquisition settings "<<acquisitionSettingsFile;
        if (QFile::exists(acquisitionSettingsFile)) {
            QSettings settings(acquisitionSettingsFile, QSettings::IniFormat);
            //qDebug()<<"  ... success!!!";
            loadQFPropertiesFromB040SPIMSettingsFile(settings);

            QStringList lfiles, lfiles_types, lfiles_descriptions;
            appendCategorizedFilesFromB040SPIMConfig(settings, lfiles, lfiles_types, lfiles_descriptions);
            //qDebug()<<lfiles<<"\n"<<lfiles_types;
            for (int i=0; i<lfiles.size(); i++) {
                // now we have to do a really thorough check of all files in files, as the file pathes may be a bit
                // different although they point to the same file.
                bool found=false;
                for (int j=0; j<files.size(); j++) {
                    if (QFileInfo(files[j]).canonicalFilePath()==QFileInfo(lfiles[i]).canonicalFilePath()) {
                        found=true;
                        break;
                    }
                }
                if (!found) {
                    files<<QFileInfo(lfiles[i]).canonicalFilePath();
                    files_types<<lfiles_types[i];
                }


            }
        }

        // finally we load all known and useful associated files into memory:
        for (int i=0; i<files.size(); i++) {
            if (i<files_types.size()) {
                QString ft=files_types[i].toLower().trimmed();
                if (ft=="overview") {
                    loadOverview(files[i]);
                } else if (ft=="statistics") {
                    loadStatistics(files[i]);
                } else if (ft=="video") {
                    loadVideo(files[i], &video, &video_width, &video_height, &video_frames);
                } else if (ft=="background") {
                    QFRDRImagingFCSData::ovrImageData img;
                    loadImage(files[i], &(img.image), &(img.width), &(img.height));
                    img.name=tr("background frame");
                    ovrImages.append(img);
                } else if (ft=="overview_before") {
                    QFRDRImagingFCSData::ovrImageData img;
                    loadImage(files[i], &(img.image), &(img.width), &(img.height));
                    img.name=tr("overview before acquisition");
                    if (propertyExists("ROI_X_START") && propertyExists("ROI_X_END") && propertyExists("ROI_Y_START") && propertyExists("ROI_Y_END")) {
                        QFRDROverviewImageInterface::OverviewImageGeoElement rect;
                        rect.title=tr("ROI");
                        rect.type=QFRDROverviewImageInterface::PIGErectangle;
                        rect.x=qMin(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.width=qMax(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.y=qMin(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        rect.height=qMax(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        img.geoElements.append(rect);;
                    }
                    ovrImages.append(img);
                } else if (ft=="overview_after") {
                    QFRDRImagingFCSData::ovrImageData img;
                    loadImage(files[i], &(img.image), &(img.width), &(img.height));
                    img.name=tr("overview after acquisition");
                    if (propertyExists("ROI_X_START") && propertyExists("ROI_X_END") && propertyExists("ROI_Y_START") && propertyExists("ROI_Y_END")) {
                        QFRDROverviewImageInterface::OverviewImageGeoElement rect;
                        rect.title=tr("ROI");
                        rect.type=QFRDROverviewImageInterface::PIGErectangle;
                        rect.x=qMin(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.width=qMax(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.y=qMin(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        rect.height=qMax(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        img.geoElements.append(rect);;
                    }
                    ovrImages.append(img);
                } else if (ft=="overview_before_transmission") {
                    QFRDRImagingFCSData::ovrImageData img;
                    loadImage(files[i], &(img.image), &(img.width), &(img.height));
                    img.name=tr("overview before acquisition (transm. illumination)");
                    if (propertyExists("ROI_X_START") && propertyExists("ROI_X_END") && propertyExists("ROI_Y_START") && propertyExists("ROI_Y_END")) {
                        QFRDROverviewImageInterface::OverviewImageGeoElement rect;
                        rect.title=tr("ROI");
                        rect.type=QFRDROverviewImageInterface::PIGErectangle;
                        rect.x=qMin(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.width=qMax(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.y=qMin(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        rect.height=qMax(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        img.geoElements.append(rect);;
                    }
                    ovrImages.append(img);
                } else if (ft=="overview_after_transmission") {
                    QFRDRImagingFCSData::ovrImageData img;
                    loadImage(files[i], &(img.image), &(img.width), &(img.height));
                    img.name=tr("overview after acquisition (transm. illumination)");
                    if (propertyExists("ROI_X_START") && propertyExists("ROI_X_END") && propertyExists("ROI_Y_START") && propertyExists("ROI_Y_END")) {
                        QFRDROverviewImageInterface::OverviewImageGeoElement rect;
                        rect.title=tr("ROI");
                        rect.type=QFRDROverviewImageInterface::PIGErectangle;
                        rect.x=qMin(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.width=qMax(getProperty("ROI_X_START").toDouble(), getProperty("ROI_X_END").toDouble());
                        rect.y=qMin(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        rect.height=qMax(getProperty("ROI_Y_START").toDouble(), getProperty("ROI_Y_END").toDouble());
                        img.geoElements.append(rect);;
                    }
                    ovrImages.append(img);
                } else if (ft=="acf" || ft=="ccf" || ft=="dccf"){
                    if (!dataLoaded) {
                        if (filetype.toUpper()=="VIDEO_CORRELATOR") {
                            loadVideoCorrelatorFile(files[i]);
                            //qDebug()<<"loading Video correlator file '"<<files[i]<<"'\n";
                            dataLoaded=true;
                        } else if (filetype.toUpper()=="RADHARD2") {
                            loadRadhard2File(files[i]);
                            width=getProperty("WIDTH", 0).toInt();
                            height=getProperty("HEIGHT", 0).toInt();
                            dataLoaded=true;
                        } else {
                          setError(tr("filetype '%1' is unknown for Imaging FCS data files (file is '%2')").arg(filetype).arg(files[i]));
                          break;
                        }
                    } else {
                        setError(tr("correlation data has already been loaded for Imaging FCS (2nd data file is '%2')").arg(filetype).arg(files[i]));
                        break;
                    }
                }
            }
        }


    }

    if (e) {
        QDomElement te=e->firstChildElement("leaveout");
        QString l=te.attribute("list");
        QStringList li=l.split(",");
        //qDebug()<<li;
        for (int i=0; i<li.size(); i++) {
            bool ok=false;
            int lo=li[i].toUInt(&ok);
            if (ok) leaveoutAddRun(lo);
            //qDebug()<<lo<<ok;
        }
        recalcCorrelations();
    }

    if (!dataLoaded) {
        setError(tr("did not find a correlation data file (acf, ccf, dccf, ...) for record"));
    }
}

bool QFRDRImagingFCSData::loadOverview(const QString& filename) {
    bool ok=false;

    if (!overview || !overviewF) return false;

    if (QFile::exists(filename)) {
        TIFF* tif=TIFFOpen(filename.toAscii().data(), "r");
        if (tif) {
            uint32 nx,ny;
            TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
            TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
            uint16_t* ovw=(uint16_t*)malloc(nx*ny*sizeof(uint16_t));
            ok=TIFFReadFrame<uint16_t>(tif, ovw);

            //qDebug()<<"TIFF: "<<nx<<"x"<<ny<<"    data: "<<width<<"x"<<height;

            if (width*height<=(int64_t)nx*(int64_t)ny) {
                for (int32_t y=0; y<height; y++) {
                    for (int32_t x=0; x<width; x++) {
                        overview[y*width+x]=ovw[y*nx+x];
                        overviewF[y*width+x]=ovw[y*nx+x];
                    }

                }
            } else {
                ok=false;
            }

            free(ovw);
            TIFFClose(tif);
        }
    }

    if (!ok && overview && overviewF) {
        for (int i=0; i<width*height; i++) {
            overview[i]=0;
            overviewF[i]=0;
        }
    }
    return ok;
}


bool QFRDRImagingFCSData::loadImage(const QString& filename, double** data, int* width, int* height) {
    bool ok=false;

    if (*data) free(*data);
    *data=NULL;
    *width=0;
    *height=0;

    if (QFile::exists(filename)) {
        TIFF* tif=TIFFOpen(filename.toAscii().data(), "r");
        if (tif) {
            uint32 nx,ny;
            TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
            TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
            *width=nx;
            *height=ny;
            *data=(double*)malloc(nx*ny*sizeof(double));
            ok=TIFFReadFrame<double>(tif, *data);
            TIFFClose(tif);
        }
    }
    return ok;
}

bool QFRDRImagingFCSData::loadVideo(const QString& filename, double** data, int* width, int* height, uint32_t* frames) {
    bool ok=false;

    if (*data) free(*data);
    *data=NULL;
    *width=0;
    *height=0;
    *frames=0;

    if (QFile::exists(filename)) {
        TIFF* tif=TIFFOpen(filename.toAscii().data(), "r");
        if (tif) {
            *frames=TIFFCountDirectories(tif);
            uint32 nx,ny;
            TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
            TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
            *width=nx;
            *height=ny;
            *data=(double*)malloc(nx*ny*(*frames)*sizeof(double));
            uint32_t i=0;
            do {
                ok=ok & TIFFReadFrame<double>(tif, &((*data)[i*nx*ny]));
                i++;
            } while (TIFFReadDirectory(tif) && i<=(*frames));
            TIFFClose(tif);
        }
    }
    return ok;
}

bool QFRDRImagingFCSData::loadVideoCorrelatorFile(const QString &filename) {
	bool ok=true;
	QString errorDescription="";

#ifdef DEBUG_TIMING
    QElapsedTimer time;
    time.start();
#endif

	// LOAD YOUR DATAFILE
	//      and set ok=false, if an error occured
	// example:
	QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		ok=false;
	} else {
        int taucolumn=getProperty("TAU_COLUMN", 0).toInt();
        double taufactor=getProperty("TAU_FACTOR", 1).toDouble();
        double corroffset=getProperty("CORR_OFFSET", 0).toDouble();
        if (taucolumn<0) taucolumn=0;
        int corrcolumn=getProperty("CORRELATION_COLUMN", 1).toInt();
        if (corrcolumn<0) corrcolumn=1;
        int correrrcolumn=getProperty("CORRELATION_ERROR_COLUMN", -1).toInt();
        //int maxCol=qMax(corrcolumn, taucolumn);

        QByteArray dataFromFile=file.readAll();
        file.close();
#ifdef DEBUG_TIMING
        qDebug()<<"opened file     "<<time.elapsed()<<"ms"; time.start();
#endif
        //QTextStream stream(&file);
        QTextStream stream(&dataFromFile);
#ifdef DEBUG_TIMING
        qDebug()<<"read file contents into memory buffer     "<<time.elapsed()<<"ms"; time.start();
#endif
        bool last_empty, empty=true;
        QList<QList<QTriple<double, double, double> > > data_matrix;
        QList<QTriple<double, double, double> > current_set;
        int NN=0;
        int runs=0;
        while ((!stream.atEnd()) && (runs<=width*height)) {
            QVector<double> data;
            if (filename.toLower().endsWith(".qf.dat")) data=csvReadline(stream, ' ', '#', 0);
            else data=csvReadline(stream, ',', '#', 0);
            last_empty=empty;
            empty=data.isEmpty();
            if ((!empty) && (corrcolumn<data.size()) && (taucolumn<data.size()) && (correrrcolumn<data.size()) && (correrrcolumn>=0)) {
                current_set.append(qMakeTriple(data[taucolumn]*taufactor, data[corrcolumn]-corroffset,  data[correrrcolumn]));
            } else if ((!empty) && (corrcolumn<data.size()) && (taucolumn<data.size())) {
                current_set.append(qMakeTriple(data[taucolumn]*taufactor, data[corrcolumn]-corroffset, 0.0));
                //qDebug()<<"  tau="<<data[0]<<"   c="<<data[1];
            }
            if (((last_empty&&empty)||(stream.atEnd()))&&(!current_set.isEmpty())) {
                // this cuts all lines from current_set which are equal to the lastval (if this is 0.0 or 1.0)
                // this cuts away channels where the correlations have not bee calculated!
                double lastval=0.0;
                if (current_set.size()>0) {
                    lastval=current_set[current_set.size()-1].second;
                    if (lastval==0.0 || lastval==1.0) {
                        while ((current_set.size()>10) && (current_set[current_set.size()-1].second==lastval)) current_set.pop_back();
                    }
                }

                data_matrix.append(current_set);
                if (NN<current_set.size()) NN=current_set.size();
                //qDebug()<<"runs="<<runs<<"     NN="<<NN<<"     current_set.size()="<<current_set.size()<<"     data_matrix.size()="<<data_matrix.size()<<"    all0="<<all0<<"    all1="<<all1;
                current_set.clear();
                runs++;
                if (runs%50==0) QApplication::processEvents();
            }
            //if (stream.atEnd()) //qDebug()<<"runs="<<runs<<"     NN="<<NN<<"     width*height="<<width*height<<"     stream.atEnd()="<<stream.atEnd()<<"    data="<<data;

        }
#ifdef DEBUG_TIMING
        qDebug()<<"parsed CSV     "<<time.elapsed()<<"ms"; time.start();
#endif
        dataFromFile.clear();
        width=getProperty("WIDTH").toInt();
        height=getProperty("HEIGHT").toInt();
        //std::cout<<"width="<<width<<"   height="<<height<<"   NN="<<NN<<std::endl;
        if (NN>0) {
            if (runs+1>=width*height) {
                allocateContents(width, height, NN);
                for (int i=0; i<width*height; i++) {
                    if (i>=data_matrix.size()) {
                        for (int j=0; j<NN; j++) {
                            correlations[i*NN+j]=0;
                            sigmas[i*NN+j]=0;
                        }
                    } else {
                        for (int j=0; j<NN; j++) {
                            //std::cout<<"i="<<i<<" data_matrix.size()="<<data_matrix.size()<<"    j="<<j<<" data_matrix[i].size()="<<data_matrix[i].size()<<std::endl;
                            if (j<data_matrix[i].size()) {
                                tau[j]=data_matrix[i].at(j).first;
                                correlations[i*NN+j]=data_matrix[i].at(j).second;
                                sigmas[i*NN+j]=data_matrix[i].at(j).third;
                            } else {
                                correlations[i*NN+j]=0;
                                sigmas[i*NN+j]=0;
                            }
                        }
                    }
                }
                QApplication::processEvents();
                recalcCorrelations();
                QApplication::processEvents();
            } else {
                ok=false;
                errorDescription=tr("too few records in file: %1 records found, but %2 * %3 = %4 required").arg(NN).arg(width).arg(height).arg(width*height);
            }
        } else {
            ok=false;
            errorDescription=tr("no records found in file");
        }
	}
#ifdef DEBUG_TIMING
        qDebug()<<"read into internal arrays     "<<time.elapsed()<<"ms"; time.start();
#endif

	if (!ok) setError(tr("Error while reading file '%1': %2").arg(filename).arg(errorDescription));
    return ok;
}



bool QFRDRImagingFCSData::loadRadhard2File(const QString& filename) {
    bool ok=true;

    QString errorDescription="";

    QFRDRImageReaderRH ReaderRH;

    ok=ReaderRH.open(filename);

    // LOAD FILE

    if (ok) {
        width= ReaderRH.frameWidth();
        height=ReaderRH.frameHeight();
        if((width==(128*8))&&(height==32)) {

            uint32_t **hwc_dump = (uint32_t **)malloc(1024 * sizeof(uint32_t*));
                for(int i=0;i<1024;i++)
                    hwc_dump[i]=(uint32_t*)malloc((128) * sizeof(uint32_t));
            unsigned char *hwc_line = (unsigned char*) malloc(2*4+(64*128)*4+2);
            uint32_t header;
            //read data
            do {
                //ReaderRH.readFrameCharRaw(hwc_line,&header);
                uint32_t id= (header >> 8) & 0x00FFFFFFU;
                uint32_t *src=(uint32_t *) &hwc_line[4]; //first dword: tics
                uint32_t ticks=*((uint32_t *)hwc_line);
                for(int x=0;x<32;x++) {
                    uint32_t *dst=hwc_dump[(id&31)*32+x];
                    for(int i=0;i<128;i++)
                    {
                        *dst=*src;
                        dst++;
                        src++;
                    }
                    src+=128; //skip delay registers
                }

            }while(true);
            for(unsigned int correlator=0;correlator<1024;correlator++)
            {
                    uint32_t *hwc_value=hwc_dump[correlator];
                    unsigned int tau=0;

                    uint32_t channel0=*hwc_value & 0xFFFFFFFFU;
                    uint32_t count_global=hwc_dump[correlator][126];
                    uint32_t count_local=hwc_dump[correlator][127];
                    unsigned int blocks=14;
                    unsigned int lags=8;
                    unsigned int steps=blocks*lags;
                    for(unsigned int block=0;block<blocks;block++)
                    {


                            for(unsigned int lag=0;lag<lags;lag++)
                            {
                                    uint32_t dword=*hwc_value;
                                    hwc_value++;
                                    unsigned int value=(dword & 0xFFFFFFFFU);

                                    //G, taken from correlator_multitau.h, should use buitl-in normalization function
                                    float result=1.0;
                                    if(steps>tau)
                                    {
                                            float correction_factor=(float(steps-tau))/(float)steps;
                                            result=((float)value*(float)steps)/((float)(1<<block)*(float)(correction_factor*count_global*count_local));// see "A distributed algorithm for multi-tau autocorrelation"
                                    }
                                    //\G

                                    //correlator_hw_values[correlator][block*lags+lag].tau=tau*10;//*10E-6s
                                    //correlator_hw_values[correlator][block*lags+lag].raw=value;
                                    //6correlator_hw_values[correlator][block*lags+lag].val=result;
                                    tau+=(1<<block);
                            }
                    }
            }
                        allocateContents(32,height,14*8);
           // correlations[]


        } else {
            ok=false;
        }
        if(ok) {
            width=32;
            height=32;
            int NN=100; // number of lags per correlation function
            //setQFProperty("WIDTH", width, false, true);
            //setQFProperty("HEIGHT", height, false, true);




            for (int i=0; i<width*height; i++) {
                for (int j=0; j<NN; j++) {
                    tau[j]=j;//data_matrix[i].at(j).first;
                    correlations[i*NN+j]=0;//data_matrix[i].at(j).second;
                    sigmas[i*NN+j]=0;//data_matrix[i].at(j).third;
                }
            }
        }

        QApplication::processEvents();
        recalcCorrelations();
        QApplication::processEvents();
    } else {
        ok=false;
        errorDescription=tr("error loading file");
    }
    ReaderRH.close();


    if (!ok) setError(tr("Error while reading Radhard2 file '%1': %2").arg(filename).arg(errorDescription));
    return ok;
}






int QFRDRImagingFCSData::getCorrelationRuns() {
    return width*height;
}

long long QFRDRImagingFCSData::getCorrelationN() {
    return N;
}

double* QFRDRImagingFCSData::getCorrelationT() {
    return tau;
}

double* QFRDRImagingFCSData::getCorrelation() {
    return correlations;
}

double* QFRDRImagingFCSData::getCorrelationRunErrors() {
    return sigmas;
}

bool QFRDRImagingFCSData::leaveoutRun(int run) {
    return maskGet(runToX(run), runToY(run));
}

double* QFRDRImagingFCSData::getCorrelationRun(int run) {
    return &(correlations[run*N]);
}

double* QFRDRImagingFCSData::getCorrelationRunError(int run) {
    return &(sigmas[run*N]);
}

QString QFRDRImagingFCSData::getCorrelationRunName(int run) {
    int x=runToX(run);
    int y=runToY(run);
    return QString("[%1, %2]").arg(x).arg(y);
}

double* QFRDRImagingFCSData::getCorrelationMean() {
    return correlationMean;
}

double* QFRDRImagingFCSData::getCorrelationStdDev() {
    return correlationStdDev;
}

bool QFRDRImagingFCSData::isCorrelationRunVisible(int run) {
    return !leaveoutRun(run);
}

void QFRDRImagingFCSData::allocateContents(int x, int y, int N) {
    if (correlations) free(correlations);
    if (correlationMean) free(correlationMean);
    if (correlationStdDev) free(correlationStdDev);
    if (sigmas) free(sigmas);
    if (tau) free(tau);
    if (overview) free(overview);
    if (overviewF) free(overviewF);
    if (leaveout) free(leaveout);
    correlations=NULL;
    correlationMean=NULL;
    correlationStdDev=NULL;
    sigmas=NULL;
    tau=NULL;
    overview=NULL;
    overviewF=NULL;
    leaveout=NULL;
    if ((x>0) && (y>0) && (N>0)) {
        correlations=(double*)calloc(x*y*N,sizeof(double));
        sigmas=(double*)calloc(x*y*N,sizeof(double));
        correlationMean=(double*)calloc(N,sizeof(double));
        correlationStdDev=(double*)calloc(N,sizeof(double));
        overview=(uint16_t*)calloc(x*y,sizeof(uint16_t));
        overviewF=(double*)calloc(x*y,sizeof(double));
        leaveout=(bool*)calloc(x*y,sizeof(bool));
        tau=(double*)calloc(N,sizeof(double));
        width=x;
        height=y;
        this->N=N;
        setQFProperty("WIDTH", x, false, true);
        setQFProperty("HEIGHT", y, false, true);
    }
#ifdef DEBUG_SIZES
    qDebug()<<"allocateContents( x="<<x<<" y="<<y<<" N="<<N<<"):  "<<bytestostr(N*x*y*2*sizeof(double)+x*y*sizeof(bool)+x*y*sizeof(double)+x*y*sizeof(uint16_t)+N*3*sizeof(double)).c_str();
#endif
}

void QFRDRImagingFCSData::allocateStatistics(uint32_t N) {
    if (statAvg) free(statAvg);
    if (statStdDev) free(statStdDev);
    if (statT) free(statT);
    if (statMin) free(statMin);
    if (statMax) free(statMax);
    statN=N;
    statAvg=NULL;
    statT=NULL;
    statStdDev=NULL;
    statMin=NULL;
    statMax=NULL;
    if (statN>0) {
        statAvg=(double*)calloc(statN, sizeof(double));
        statStdDev=(double*)calloc(statN, sizeof(double));
        statMin=(double*)calloc(statN, sizeof(double));
        statMax=(double*)calloc(statN, sizeof(double));
        statT=(double*)calloc(statN, sizeof(double));
    }
#ifdef DEBUG_SIZES
    qDebug()<<"allocateStatistics( N="<<N<<"):  "<<bytestostr(statN*5*sizeof(double)).c_str();
#endif

}

void QFRDRImagingFCSData::recalcCorrelations() {
    if (correlations && correlationMean && correlationStdDev) {
        for (int i=0; i<N; i++) {
            double norm=0;
            double sum=0;
            double sum2=0;
            for (int j=0; j<width*height; j++) {
                const double& v=correlations[j*N+i];
                if (QFFloatIsOK(v) && !leaveout[j]) {
                    sum+=v;
                    sum2+=v*v;
                    norm++;
                }
            }
            if (norm>0) {
                correlationMean[i]=sum/norm;
                correlationStdDev[i]=sqrt(sum2/norm-sum*sum/norm/norm);
            } else {
                correlationMean[i]=0;
                correlationStdDev[i]=0;
            }
        }
    }
}

uint32_t QFRDRImagingFCSData::getStatisticsN() const {
    return statN;
}

double *QFRDRImagingFCSData::getStatisticsMean() const {
    return statAvg;
}

double *QFRDRImagingFCSData::getStatisticsT() const {
    return statT;
}

double *QFRDRImagingFCSData::getStatisticsStdDev() const {
    return statStdDev;
}

double *QFRDRImagingFCSData::getStatisticsMin() const {
    return statMin;
}

bool QFRDRImagingFCSData::loadStatistics(const QString &filename) {
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream txt(&f);
        QVector<double> mean, stddev, min, max, time;
        while (!txt.atEnd()) {
            QVector<double> line=csvReadline(txt, ',', '#');
            if (line.size()>1) {
                time.append(line[0]);
                mean.append(line[1]);
                stddev.append(line.value(2, 0));
                min.append(line.value(3, 0));
                max.append(line.value(4, 0));
            }
        }
        //qDebug()<<"line read: "<<time.size();
        allocateStatistics(time.size());

        for (uint32_t i=0; i<statN; i++) {
            statT[i]=time[i];
            statAvg[i]=mean[i];
            statStdDev[i]=stddev[i];
            statMin[i]=min[i];
            statMax[i]=max[i];
        }


        f.close();
        return true;
    }
    return false;
}

void QFRDRImagingFCSData::loadQFPropertiesFromB040SPIMSettingsFile(QSettings &settings) {
    if (!propertyExists("MEASUREMENT_DURATION_MS") && settings.contains("acquisition/duration_milliseconds")) {
        setQFProperty("MEASUREMENT_DURATION_MS", settings.value("acquisition/duration_milliseconds").toDouble(), true, true);
    }
    if (!propertyExists("MEASUREMENT_DURATION_MS") && settings.contains("acquisition/duration")) {
        setQFProperty("MEASUREMENT_DURATION_MS", settings.value("acquisition/duration").toDouble()*1000.0, true, true);
    }
    if (!propertyExists("FRAMETIME_MS") && settings.contains("acquisition/frame_time")) {
        setQFProperty("FRAMETIME_MS", settings.value("acquisition/frame_time").toDouble()*1000.0, true, true);
    }
    if (!propertyExists("FRAMETIME_MS") && settings.contains("acquisition/frame_rate")) {
        setQFProperty("FRAMETIME_MS", 1.0/settings.value("acquisition/frame_rate").toDouble()*1000.0, true, true);
    }
    if (!propertyExists("MAGNIFICATION") && settings.contains("acquisition/magnification")) {
        setQFProperty("MAGNIFICATION", settings.value("acquisition/magnification").toDouble(), true, true);
    }
    if (!propertyExists("ROI_X_START") && settings.contains("acquisition/roi_xstart")) {
        setQFProperty("ROI_X_START", settings.value("acquisition/roi_xstart").toInt(), true, true);
    }
    if (!propertyExists("ROI_X_END") && settings.contains("acquisition/roi_xend")) {
        setQFProperty("ROI_X_END", settings.value("acquisition/roi_xend").toInt(), true, true);
    }
    if (!propertyExists("ROI_Y_START") && settings.contains("acquisition/roi_ystart")) {
        setQFProperty("ROI_Y_START", settings.value("acquisition/roi_ystart").toInt(), true, true);
    }
    if (!propertyExists("ROI_Y_END") && settings.contains("acquisition/roi_yend")) {
        setQFProperty("ROI_Y_END", settings.value("acquisition/roi_yend").toInt(), true, true);
    }
    if (!propertyExists("PIXEL_WIDTH") && settings.contains("acquisition/pixel_width")) {
        double mag=settings.value("acquisition/magnification", 1.0).toDouble();
        double cpw=settings.value("acquisition/camera_pixel_width", settings.value("acquisition/pixel_width").toDouble()).toDouble();
        double pw=settings.value("acquisition/pixel_width").toDouble();
        if (fabs(cpw/mag-pw)<0.01*fabs(pw) && (mag>1)) setQFProperty("PIXEL_WIDTH", cpw/mag, true, true);
        else setQFProperty("PIXEL_WIDTH", pw, true, true);

    }
    if (!propertyExists("PIXEL_HEIGHT") && settings.contains("acquisition/pixel_height")) {
        double mag=settings.value("acquisition/magnification", 1.0).toDouble();
        double cpw=settings.value("acquisition/camera_pixel_height", settings.value("acquisition/pixel_height").toDouble()).toDouble();
        double pw=settings.value("acquisition/pixel_height").toDouble();
        if (fabs(cpw/mag-pw)<0.01*fabs(pw) && (mag>1)) setQFProperty("PIXEL_HEIGHT", cpw/mag, true, true);
        else setQFProperty("PIXEL_HEIGHT", pw, true, true);

    }

}

void QFRDRImagingFCSData::clearOvrImages() {
    for (int i=0; i<ovrImages.size(); i++) {
        free(ovrImages[i].image);
    }
    ovrImages.clear();
}



















int QFRDRImagingFCSData::getImageFromRunsWidth() const {
    return width;
}

int QFRDRImagingFCSData::getImageFromRunsHeight() const {
    return height;
}

int QFRDRImagingFCSData::xyToRun(int x, int y) const {
    return y*width+x;
}

int QFRDRImagingFCSData::runToX(int run) const {
    return run%width;
}

int QFRDRImagingFCSData::runToY(int run) const {
    return run/width;
}

int QFRDRImagingFCSData::xyToIndex(int x, int y) const {
    return (y*width+x)*N;
}

uint16_t* QFRDRImagingFCSData::getImageFromRunsPreview() const {
    return overview;
}

void QFRDRImagingFCSData::leaveoutClear() {
    maskClear();
}

void QFRDRImagingFCSData::maskClear() {
    if (!leaveout) return;
    for (uint16_t i=0; i<width*height; i++) {
        leaveout[i]=false;
    }
}

void QFRDRImagingFCSData::maskSetAll() {
    if (!leaveout) return;
    for (uint16_t i=0; i<width*height; i++) {
        leaveout[i]=true;
    }
}

void QFRDRImagingFCSData::leaveoutRemoveRun(int run) {
    if (run>=0 && run<getCorrelationRuns()) leaveout[run]=false;
}

void QFRDRImagingFCSData::leaveoutAddRun(int run) {
    if (run>=0 && run<getCorrelationRuns()) leaveout[run]=true;
}

bool *QFRDRImagingFCSData::maskGet() {
    return leaveout;
}

bool QFRDRImagingFCSData::maskGet(uint16_t x, uint16_t y) {
    return leaveout[y*width+x];
}

void QFRDRImagingFCSData::maskUnset(uint16_t x, uint16_t y, bool value) {
    if (!leaveout) return;
    leaveout[y*width+x]=value;
}

void QFRDRImagingFCSData::maskToggle(uint16_t x, uint16_t y) {
    leaveout[y*width+x]=!leaveout[y*width+x];
}

void QFRDRImagingFCSData::maskInvert() {
    if (!leaveout) return;
    for (uint16_t i=0; i<width*height; i++) {
        leaveout[i]=!leaveout[i];
    }
}

void QFRDRImagingFCSData::maskSet(uint16_t x, uint16_t y) {
    if (!leaveout) return;
    leaveout[y*width+x]=false;
}

double *QFRDRImagingFCSData::getStatisticsMax() const {
    return statMax;
}





int QFRDRImagingFCSData::getPreviewImageCount() const {
    return 1+ovrImages.size();
}

int QFRDRImagingFCSData::getPreviewImageWidth(int image) const {
    if (image==0) return getImageFromRunsWidth();
    if (image<=ovrImages.size()) return ovrImages[image-1].width;
    return 0;
}

int QFRDRImagingFCSData::getPreviewImageHeight(int image) const {
    if (image==0) return getImageFromRunsHeight();
    if (image<=ovrImages.size()) return ovrImages[image-1].height;
    return 0;
}

QString QFRDRImagingFCSData::getPreviewImageName(int image) const {
    if (image==0) return tr("overview image (time average)");
    if (image<=ovrImages.size()) return ovrImages[image-1].name;
    return QString("");
}

double *QFRDRImagingFCSData::getPreviewImage(int image) const {
    if (image==0) return overviewF;
    if (image<=ovrImages.size()) return ovrImages[image-1].image;
    return NULL;
}

QList<QFRDROverviewImageInterface::OverviewImageGeoElement> QFRDRImagingFCSData::getPreviewImageGeoElements(int image) const {
    QList<QFRDROverviewImageInterface::OverviewImageGeoElement> result;
    if (image>0 && image<=ovrImages.size()) return ovrImages[image-1].geoElements;
    return result;
}

int QFRDRImagingFCSData::getImageStackCount() const {
    return 1;
}

uint32_t QFRDRImagingFCSData::getImageStackFrames(int stack) const {
    if (stack==0) return video_frames;
    return 0;
}

int QFRDRImagingFCSData::getImageStackWidth(int stack) const {
    if (stack==0) return video_width;
    return 0;
}

int QFRDRImagingFCSData::getImageStackHeight(int stack) const {
    if (stack==0) return video_height;
    return 0;
}

int QFRDRImagingFCSData::getImageStackChannels(int stack) const {
    if (stack==0) return 1;
    return 0;
}

double *QFRDRImagingFCSData::getImageStack(int stack, uint32_t frame, uint32_t channel) const {
    if (stack==0) return &(video[frame*video_width*video_height]);
    return NULL;
}

double QFRDRImagingFCSData::getImageStackXUnitFactor(int stack) const {
    return getProperty("PIXEL_WIDTH", 1.0).toDouble();
}

QString QFRDRImagingFCSData::getImageStackXUnitName(int stack) const {
    return QString("micrometer");
}

double QFRDRImagingFCSData::getImageStackYUnitFactor(int stack) const {
    return getProperty("PIXEL_HEIGHT", 1.0).toDouble();
}

QString QFRDRImagingFCSData::getImageStackYUnitName(int stack) const {
    return QString("micrometer");
}

double QFRDRImagingFCSData::getImageStackTUnitFactor(int stack) const {
    if (video_frames<=0) return 1;
    double stat=1;
    if (statT) stat=(statT[statN]-statT[0]);
    return getProperty("MEASUREMENT_DURATION_MS", stat*1000.0).toDouble()/1000.0/double(video_frames);
    return 1;
}

QString QFRDRImagingFCSData::getImageStackTUnitName(int stack) const {
    if (stack==0) return tr("time [seconds]");
    return QString("");
}

double QFRDRImagingFCSData::getImageStackCUnitFactor(int stack) const {
    return 1;
}

QString QFRDRImagingFCSData::getImageStackCUnitName(int stack) const {
    return QString("");
}

QString QFRDRImagingFCSData::getImageStackDescription(int stack) const {
    if (stack==0) return tr("averaged video");
    return QString("");
}


