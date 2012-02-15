#ifndef QFRDRIMAGINGFCSDATA_H
#define QFRDRIMAGINGFCSDATA_H

#include <QList>
#include <QObject>
#include <QMap>
#include <QProgressBar>
#include <QStatusBar>
#include <QWidget>
#include <QMenu>
#include <QPair>
#include <QList>
#include <QAction>
#include "qfrawdatarecord.h"
#include "qfrdrimagingfcs_dataeditor.h"
#include "qfrdrimagingfcs_dataeditor_image.h"
#include "qfrawdatarecordfactory.h"
#include "../interfaces/qfrdrfcsdatainterface.h"
#include "../interfaces/qfrdrimagetoruninterface.h"
#include "../interfaces/qfrdroverviewimageinterface.h"
#include "csvtools.h"
#include "qtriple.h"
#include "qfrdrimagingfcsoverviewrateeditor.h"



/*! \brief QFRawDataRecord implementation
    \ingroup qf3rdrdp_imaging_fcs

*/
class QFRDRImagingFCSData : public QFRawDataRecord, public QFRDRFCSDataInterface, public QFRDRImageToRunInterface, public QFRDROverviewImageInterface {
        Q_OBJECT
        Q_INTERFACES(QFRDRFCSDataInterface QFRDRImageToRunInterface QFRDROverviewImageInterface)
    public:
        /** Default constructor */
        QFRDRImagingFCSData(QFProject* parent);
        /** Default destructor */
        virtual ~QFRDRImagingFCSData();

        /** \brief return type (short type string) */
        virtual QString getType() const { return "imaging_fcs"; }
        /** \brief return type (longer type string, user readable) */
        virtual QString getTypeName() const { return tr("Imaging FCS Data"); }
        /** \brief return a small icon (16x16) */
        virtual QIcon getSmallIcon() const { return QIcon(":/imaging_fcs/qfrdrimagingfcs_small.png"); }
        /** \brief return type description */
        virtual QString getTypeDescription() const { return tr("manages FCS correlation curves from an imaging FCS experiment (SPIM-FCS, TIR-FCS ...)"); }
        /** \brief return a large icon (32x32) */
        virtual QIcon getLargeIcon() const { return QIcon(":/imaging_fcs/qfrdrimagingfcs.png"); }
        /** \brief returns the number of additional editor panes for this record */
        virtual int getEditorCount() { return 3; }
        /** \brief returns the name for the i-th editor pane */
        virtual QString getEditorName(int i);
        /** \brief create an object for the i-th editor pane */
        virtual QFRawDataEditor* createEditor(QFPluginServices* services, int i=0, QWidget* parent=NULL);
        /** \brief export the raw data into the specified format */
        virtual void exportData(const QString& format, const QString& filename)const ;
        /** \brief returns a list of filetypes which correspond to the filetypes returned by getExportFiletypes() */
        QStringList getExportFiletypes();
        /** \brief returns the title of the Export file dialog */
        QString getExportDialogTitle();
        /** \brief returns the filetype of the Export file dialog */
        QString getExportDialogFiletypes();



        /** \copydoc QFRDRFCSDataInterface::getCorrelationRuns() */
        virtual int getCorrelationRuns();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationN() */
        virtual long long getCorrelationN();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationT() */
        virtual double* getCorrelationT();
        /** \copydoc QFRDRFCSDataInterface::getCorrelation() */
        virtual double* getCorrelation();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRun() */
        virtual double* getCorrelationRun(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunError() */
        virtual double* getCorrelationRunError(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunName() */
        virtual QString getCorrelationRunName(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationMean() */
        virtual double* getCorrelationMean();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationStdDev() */
        virtual double* getCorrelationStdDev();
        /** \copydoc QFRDRFCSDataInterface::isCorrelationRunVisible() */
        virtual bool isCorrelationRunVisible(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunErrors() */
        virtual double* getCorrelationRunErrors();






        /** \brief returns whether to leave out a run */
        virtual bool leaveoutRun(int run);
        /** \brief add a run to the leaveouts */
        virtual void leaveoutAddRun(int run);
        /** \brief remove a run from the leaveouts */
        virtual void leaveoutRemoveRun(int run);
        /** \brief clear all leaveouts */
        virtual void leaveoutClear();

        void maskClear();
        void maskSetAll();
        void maskSet(uint16_t x, uint16_t y);
        void maskUnset(uint16_t x, uint16_t y, bool value=true);
        void maskToggle(uint16_t x, uint16_t y);
        void maskInvert();
        bool maskGet(uint16_t x, uint16_t y);
        bool* maskGet();

        /** \brief recalculate the averages/std. deviations */
        void recalcCorrelations();


        /** \brief return the number of datapoints in the statistics dataset */
        uint32_t getStatisticsN() const;
        /** \brief return the array of average values of the statsitics */
        double* getStatisticsMean() const;
        /** \brief return the array of standard deviation values of the statsitics */
        double* getStatisticsStdDev() const;
        double* getStatisticsMin() const;
        double* getStatisticsMax() const;
        /** \brief return the array of time points [seconds] for the statsitics */
        double *getStatisticsT() const;

    protected:
        /** \brief write the contents of the object to a XML file */
        virtual void intWriteData(QXmlStreamWriter& w);
        /** \brief read in external data files <b>and</b> data stored in the project file <b>IMPLEMENT IN CHILD CLASSES!</b>
         *
         * If \a e is \c NULL then this method should only read the datafiles already saved in the files property.
         */
        virtual void intReadData(QDomElement* e=NULL);

        /** \brief load data file */
        bool loadVideoCorrelatorFile(const QString& filename);

        /** \brief load data file */
        bool loadRadhard2File(const QString& filename);

        /** \brief load overview image file */
        bool loadOverview(const QString& filename);

        /** \brief load an image file into the given arrays */
        bool loadImage(const QString &filename, double **data, int *width, int *height);

        /** \brief load the statistics file */
        bool loadStatistics(const QString& filename);

    private:
        /** \brief width of the image */
        int width;
        /** \brief height of the image */
        int height;
        /** \brief number of points in correlation curve */
        int N;
        /** \brief points to the correlation curves */
        double* correlations;
        /** \brief average over all correlation curves */
        double* correlationMean;
        /** \brief average over all correlation curves */
        double* correlationStdDev;
        /** \brief points to the correlation curve erorrs */
        double* sigmas;
        /** \brief time axis [seconds] */
        double* tau;

        /** \brief number of data points in the statistics  */
        uint32_t statN;
        /** \brief statistics: average vector */
        double* statAvg;
        /** \brief statistics: standard deviation vector */
        double* statStdDev;
        /** \brief statistics: time [seconds] vector */
        double* statT;
        double* statMin;
        double* statMax;

        /** \brief overview image */
        uint16_t* overview;
        double* overviewF;

        struct ovrImageData {
            double* image;
            QString name;
            int width;
            int height;
        };

        QList<ovrImageData> ovrImages;

        void clearOvrImages();

        /** \brief the leaveout list */
        bool* leaveout;
        //QList<int> leaveout;
    protected:
        /** \brief allocate memory to store a \a x by \a y set of correlation curves (+ additional data, like average and sigmas) with \a N datapoints each */
        virtual void allocateContents(int x, int y, int N);
        void allocateStatistics(uint32_t N);


    public:
        /** \copydoc QFRDRImageToRunInterface::getImageFromRunsWidth() */
        virtual int getImageFromRunsWidth() const;
        /** \copydoc QFRDRImageToRunInterface::getImageFromRunsHeight() */
        virtual int getImageFromRunsHeight() const;
        /** \copydoc QFRDRImageToRunInterface::xyToRun() */
        virtual int xyToRun(int x, int y) const;
        /** \copydoc QFRDRImageToRunInterface::runToX() */
        virtual int runToX(int run) const;
        /** \copydoc QFRDRImageToRunInterface::runToY() */
        virtual int runToY(int run) const;
        /** \copydoc QFRDRImageToRunInterface::xyToIndex() */
        virtual int xyToIndex(int x, int y) const;
        /** \copydoc QFRDRImageToRunInterface::getImageFromRunsPreview() */
        virtual uint16_t* getImageFromRunsPreview() const;


        /** \copydoc QFRDROverviewImageInterface::getPreviewImageCount() */
        virtual int getPreviewImageCount() const;
        /** \copydoc QFRDROverviewImageInterface::getPreviewImageWidth() */
        virtual int getPreviewImageWidth(int image) const;
        /** \copydoc QFRDROverviewImageInterface::getPreviewImageHeight() */
        virtual int getPreviewImageHeight(int image) const;
        /** \copydoc QFRDROverviewImageInterface::getPreviewImageName() */
        virtual QString getPreviewImageName(int image) const;
        /** \copydoc QFRDROverviewImageInterface::getPreviewImage() */
        virtual double* getPreviewImage(int image) const;
        /** \copydoc QFRDROverviewImageInterface::getPreviewImageGeoElements() */
        virtual QList<QFRDROverviewImageInterface::OverviewImageGeoElement> getPreviewImageGeoElements(int image) const;
};


#endif // QFRDRIMAGINGFCSDATA_H
