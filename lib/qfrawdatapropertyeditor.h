#ifndef QFRAWDATAPROPERTYEDITOR_H
#define QFRAWDATAPROPERTYEDITOR_H

#include <QtGui>
#include <QPointer>
#include <QMap>
#include <QSortFilterProxyModel>
#include "qfproject.h"
#include "qfrawdatarecord.h"
#include "programoptions.h"
#include "qfpluginservices.h"
#include "qftools.h"
#include "qenhancedtableview.h"
#include "lib_imexport.h"
#include <QMenu>
#include <QAction>
#include <QList>
#include <QPair>
#include <QTimer>
#include <QModelIndexList>
#include "qfenhancedlineedit.h"
#include "qfstyledbutton.h"
#include "qfcompleterfromfile.h"
#include "qfpropertyitemdelegate.h"
#include "dlg_qffileeditdialog.h"

class QFRawDataPropertyEditor_private;

/*! \brief editor widget (window) for raw data items
    \ingroup qf3lib_project



 As there were problems when changing the displayed editor for different types of raw data items. One editor window
 is only useable for one type of  raw data!
*/
class QFLIB_EXPORT QFRawDataPropertyEditor : public QWidget {
        Q_OBJECT
    public:
        /** Default constructor */
        QFRawDataPropertyEditor(QFPluginServices* services, ProgramOptions* set, QFRawDataRecord* current=NULL, int id=0, QWidget* parent=NULL, Qt::WindowFlags f = Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
        /** Default destructor */
        virtual ~QFRawDataPropertyEditor();
        /** \brief set the current record */
        void setCurrent(QFRawDataRecord* c);
        /** \brief get the current record */
        QFRawDataRecord* getCurrent() const;
        /** \brief set a Programoptions object to use for storing application settings */
        void setSettings(ProgramOptions* settings);
        /** \brief write the settings */
        void writeSettings();
        /** \brief returns the menubar of this widget */
        QMenuBar* getMenuBar() const;
        /** \brief chages the visiblity of the menubar */
        void setMenuBarVisible(bool visible);
        /** \brief add a new menu to te menubar that is visible for the specified editor only, if editor>=0 or always, if editor<0 */
        QMenu* addMenu(const QString &title, int editor=-1);
        /** \brief tells the editor to switch the visibility of the specified menu according to the set editor (see addMenu() ), but does not add the menu to the main menu bar */
        void registerMenu(QMenu* menu, int editor=-1);
        /** \brief returns a pointer to the help menu of this editor */
        QMenu* getHelpMenu() const;
        /** \brief activate the given tab in the editor
         *
         *  If N (>2) tabs are available in the window, then:
         *    - 0: the basic properties editor
         *    - 1...N-2: the editors of the current RDR
         *    - N-1: the results tab
         *  .
         */
        void showTab(int tab);
        /** \brief return the number of available tabs */
        int getTabs() const;
        /** \brief send the given command string to all edtor tabs */
        void sendEditorCommand(const QString& command, const QVariant& param1=QVariant(), const QVariant& param2=QVariant(), const QVariant& param3=QVariant(), const QVariant& param4=QVariant(), const QVariant& param5=QVariant());
    protected:

        /** \brief read the settings */
        void readSettings();
        void closeEvent( QCloseEvent * event );
        void resizeEvent ( QResizeEvent * event );


    private slots:


    private:
        QFRawDataPropertyEditor_private* p;
};

#endif // QFRAWDATAPROPERTYEDITOR_H
