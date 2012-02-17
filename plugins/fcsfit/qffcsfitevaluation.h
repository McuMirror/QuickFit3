#ifndef QFFCSFITEVALUATION_H
#define QFFCSFITEVALUATION_H

#include <QMap>
#include <QSettings>

#include "qfevaluationitem.h"
#include "../interfaces/qfrdrfcsdatainterface.h"
#include "../interfaces/qfrdrsimplecountrateinterface.h"
#include "../interfaces/qfrdrcountratesinterface.h"
#include "qfevaluationitemfactory.h"
#include "qffitfunction.h"
#include "qffitalgorithm.h"
#include "qffitparameterbasicinterface.h"
#include "../base_classes/qffitresultsevaluation.h"
#include "../base_classes/qffitresultsbyindexevaluation.h"
#include "qffitalgorithm.h"

/*! \brief evaluation item class for FCS least square fits
    \ingroup qf3evalp_fcsfit


    \note This evaluation is applicable to all QFRawDataRecords that implement the QFRDRFCSDataInterface interface! Also it makes use of all
          fit functions (see QFFitFunction and QFFitFunctionManager) registered to QuickFit where the id starts with \c "fcs_" !

    \note If a fit function has a parameter with name count_rate this is filled with the count rate stored in the FCS dataset (if
          implemented, i.e. if the QFRawDataset also implements QFRDRCountRatesInterface)
*/
class QFFCSFitEvaluation : public QFFitResultsByIndexEvaluation {
        Q_OBJECT
    public:
        /** \brief which data weighting should be applied */
        enum DataWeight {
            EqualWeighting=0,
            StdDevWeighting=1,
            RunErrorWeighting=2
        };

        /** Default constructor */
        QFFCSFitEvaluation(QFProject* parent);
        /** Default destructor */
        virtual ~QFFCSFitEvaluation();

        /** \brief return type (short type string) */
        virtual QString getType() const { return QString("fcs_fit"); }
        /** \brief return type (longer type string, user readable) */
        virtual QString getTypeName() const { return tr("FCS Fit"); }
        /** \brief return a small icon (16x16) */
        virtual QIcon getSmallIcon() const { return QIcon(":/fcs_fit.png"); }
        /** \brief return type description */
        virtual QString getTypeDescription() const { return tr("Least squares fitting for fluorescence correlation spectroscopy data"); }
        /** \brief return a large icon (32x32) */
        virtual QIcon getLargeIcon() const { return QIcon(":/fcs_fit_logo.png"); }
        /** \brief returns the number of additional editor panes for this record */
        /** \brief returns the name for the i-th editor pane */
        virtual QString getEditorName() { return QString("Fit"); }
        /** \brief create an object for the i-th editor pane */
        virtual QFEvaluationEditor* createEditor(QFPluginServices* services, QWidget* parent=NULL);
        /** \brief determines whether this evaluation is applicable to a given raw data record. This method is used to generate the
         *         list of raw data records presented to the user */
        virtual bool isApplicable(QFRawDataRecord* record);


        /** \brief set the current fitting algorithm */
        void setFitDataWeighting(DataWeight weighting) { m_weighting=weighting; }
        /** \brief get the current fitting algorithm */
        DataWeight getFitDataWeighting() const { return m_weighting; }


        /** \brief create an ID to reference results that belong to this evaluation \b object (includes the evaluation id) and the
         *         current fit function for a given fitFunction ID */
        virtual QString getEvaluationResultID(QString fitFunction, int currentRun);

        /** \brief extract the index number (or -1 for average) from the supplied resultID, which has to have been created with getEvaluationResultID() from this class */
        virtual int getIndexFromEvaluationResultID(const QString& resultID);
        /** \brief return the smallest available index */
        virtual int getIndexMin(QFRawDataRecord* r);
        /** \brief return the largest available index */
        virtual int getIndexMax(QFRawDataRecord* r);

        /*! \brief perform a fit for the given \a record and \a run

            The parameters \a defaultMinDatarange and \a defaultMaxDatarange set the range of data points taken for the fit.
            If both are -1, the full range is used

            The object \a dlgFitProgress (if supplied) is used to report the progress and to check whether the user clicked "Cancel".
          */
        virtual void doFit(QFRawDataRecord* record, int run, int defaultMinDatarange=-1, int defaultMaxDatarange=-1, QFFitAlgorithmReporter* dlgFitProgress=NULL);

        /** \brief calculates fit statistics for the given fit function and dataset. */
        QFFitFunction::FitStatistics calcFitStatistics(QFFitFunction* ffunc, long N, double* tauvals, double* corrdata, double* weights, int datacut_min, int datacut_max, double* fullParams, double* errors, bool* paramsFix, int runAvgWidth, int residualHistogramBins, QFRawDataRecord* record=NULL, int run=-1);

        /** \brief allocate an array for the weights (using calloc(), so use free() to delete the array) and fill
         *         it with the appropriate values, according to the current settings */
        virtual double* allocWeights(bool* weightsOK=NULL, QFRawDataRecord* record=NULL, int run=-100, int data_start=-1, int data_end=-1);

    protected:
        /*! \copydoc QFFitResultsEvaluation::intWriteDataAlgorithm()      */
        virtual void intWriteDataAlgorithm(QXmlStreamWriter& w) const;

        /*! \copydoc QFFitResultsEvaluation::intReadDataAlgorithm()      */
        virtual void intReadDataAlgorithm(QDomElement& e);

        virtual bool hasSpecial(QFRawDataRecord* r, const QString& id, const QString& paramid, double& value, double& error);


        /** \brief type of data weighting */
        DataWeight m_weighting;


    public:

        /* explicitly make some functions visible again, as the C++ compiler hides function definitions
           from a base class that are also declared in the derived class, but with different parameter sets!

           see http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.9
         */

        using QFFitResultsByIndexEvaluation::getEvaluationResultID;


};

#endif // QFFCSFITEVALUATION_H

