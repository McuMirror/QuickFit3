#ifndef QFPRDRFCS_H
#define QFPRDRFCS_H

#include "qfpluginrawdata.h"
#include "qfrdrfcsfitfunctionsimulator.h"

/*!
    \defgroup qf3rdrdp_fcs FCS Raw Data Record Plugin
    \ingroup qf3rawdataplugins
*/

/*! \brief FCS raw data record plugin class
    \ingroup qf3rdrdp_fcs
    */

class QFPRDRFCS : public QObject, public QFPluginRawDataRecordBase {
        Q_OBJECT
        Q_INTERFACES(QFPluginRawDataRecord)
    public:
        /** Default constructor */
        QFPRDRFCS(QObject* parent=NULL);
        /** Default destructor */
        virtual ~QFPRDRFCS();

        /** \brief Create a new QFRawDataRecord in the given project \a parent. Also adds the record to the project. */
        virtual QFRawDataRecord* createRecord(QFProject* parent);

        /*! \brief create menu items in \a menu that start insert of record

            The menu items should be connected to slots in the plugin.
        */
        virtual void registerToMenu(QMenu* menu);

        /** \brief short ID for the plugin */
        virtual QString getID() const  { return tr("fcs"); };

        /** \brief name for the plugin */
        virtual QString getName() const  { return tr("FCS/DLS Correlation Curve Plugin"); };

        /** \brief short description for the plugin */
        virtual QString getDescription() const  { return tr("manages Fluorescence Correlations Spectroscopy (FCS) and Dynamic Light Scattering (DLS) correlation curve data."); };

        /** \brief author the plugin */
        virtual QString getAuthor() const  { return tr("Jan Krieger, Benedikt H�usele"); };

        /** \brief copyright information the plugin */
        virtual QString getCopyright() const  { return tr("(c) 2010-2013 by Jan Krieger, Benedikt H�usele"); };

        /** \brief weblink for the plugin */
        virtual QString getWeblink() const  { return tr(""); };
        /** \brief plugin version  */
        virtual void getVersion(int& major, int& minor) const {
            major=1;
            minor=4;
        };

        /** \brief icon for the plugin */
        virtual QString getIconFilename() const  { return QString(":/fcs_logo.png"); };
    protected slots:
        /** \brief insert FCS data from (multiple) file(s) into (multiple) RDRs*/
        void insertFCS();
        /** \brief insert FCS data from multiple files into one RDR */
        void insertMultiFileFCS();

        void openSimulator();
        void insertSimulated();
    protected:
        QFRDRFCSFitFunctionSimulator* dlgSimulator;

        /*! \brief add a ALV5000 file to the current project

            \param filename filename of the ALV5000 file
            \param paramValues values for parameters in paramNames
            \param paramReadonly names of initial parameters that should be declared readonly
        */
        void insertALV5000File(const QStringList &filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);

        /*! \brief add a ALV6000 file to the current project

            \param filename filename of the ALV5000 file
            \param paramValues values for parameters in paramNames
            \param paramReadonly names of initial parameters that should be declared readonly
        */
        void insertALV6000File(const QStringList &filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);

        /*! \brief add a CSV file to the current project

            \param filename filename of the CSV file
            \param paramValues values for parameters in paramNames
            \param paramReadonly names of initial parameters that should be declared readonly
        */
        void insertCSVFile(const QStringList& filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);

        /*! \brief add an ALBA file to the current project

            \param filename filename of the CSV file
            \param paramValues values for parameters in paramNames
            \param paramReadonly names of initial parameters that should be declared readonly
        */
        void insertALBAFile(const QStringList& filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);
        void insertDiffusion4File(const QStringList& filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);
        void insertOlegFile(const QStringList &filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);

        /*! \brief add a Zeiss Confocor3 file to the current project

            \param filename filename of the Confocor3 file
            \param paramValues values for parameters in paramNames
            \param paramReadonly names of initial parameters that should be declared readonly
        */
        void insertConfocor3File(const QStringList &filename, const QMap<QString, QVariant>& paramValues, const QStringList& paramReadonly);
    private:
};

#endif // QFPRDRFCS_H
