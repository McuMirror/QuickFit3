TEMPLATE = lib
TARGET = quickfit3widgets

include(../quickfit3.pri)

release {
    message("building $$TARGET $$TEMPLATE in RELEASE mode, output is in $$QFOUTPUT")
} else {
    message("building $$TARGET $$TEMPLATE in DEBUG mode, output is in $$QFOUTPUT")
}

DEPENDPATH += .

INCLUDEPATH += ../../../../LIB/trunk/ \
               . \
               ../lib/ \
               ../plugins/interfaces/

include(../libquickfit.pri)

QT += gui xml svg opengl



DESTDIR = $$QFOUTPUT

#DEFINES += JKQTBP_AUTOTIMER
# Input
HEADERS += ../../../../LIB/trunk/qt/jkqtpbaseplotter.h \
           ../../../../LIB/trunk/qt/jkqtpdatastorage.h \
           ../../../../LIB/trunk/qt/jkqtpelements.h \
           ../../../../LIB/trunk/qt/jkqtpgeoelements.h \
           ../../../../LIB/trunk/qt/jkqtmathtext.h \
           ../../../../LIB/trunk/qt/jkqtpbaseelements.h \
           ../../../../LIB/trunk/qt/jkqtplotter.h \
           ../../../../LIB/trunk/qt/jkqtptools.h \
           ../../../../LIB/trunk/qt/jkqttools.h \
           ../../../../LIB/trunk/qt/flowlayout.h \
           ../../../../LIB/trunk/qt/jkqtfastplotter.h \
           ../../../../LIB/trunk/qt/qtlogfile.h \
           ../../../../LIB/trunk/tools.h \
           ../../../../LIB/trunk/datatable2.h \
           ../../../../LIB/trunk/qt/jkverticalscrollarea.h \
           ../../../../LIB/trunk/qt/colorcombobox.h \
           ../../../../LIB/trunk/qt/jkstyledbutton.h \
           ../../../../LIB/trunk/qt/qenhancedlineedit.h \
           ../../../../LIB/trunk/qt/qspecialtoolbutton.h \
           ../../../../LIB/trunk/qt/qmodernprogresswidget.h \
           ../../../../LIB/trunk/qt/qmoretextobject.h \
           qehtreeview.h \
           datacutslider.h \
           qffitparameterwidget.h \
           qfdlgcsvparameters.h \
    qfstagecombobox.h \
    qfcameraconfigcombobox.h \
    qfcameracombobox.h \
    objectives.h \
    ../plugins/interfaces/qfrdrfcsdatainterface.h \
    ../plugins/interfaces/qfrdrcountratesinterface.h \
    ../plugins/interfaces/qfextensionparameterdevice.h \
    ../plugins/interfaces/qfextensionlinearstage.h \
    ../plugins/interfaces/qfextensioncamera.h \
    libwid_imexport.h \
    ../../../../LIB/trunk/lib_imexport.h \
    ../../../../LIB/trunk/qt/jkqtpimagetools.h \
    ../../../../LIB/trunk/qt/jkqtpimageelements.h \
    filters.h \
    ../../../../LIB/trunk/qt/qenhancedcombobox.h \
    qfshuttercombobox.h \
    ../../../../LIB/trunk/qt/qrecentfilesmenu.h \
    qfplayercontrols.h \
    ../../../../LIB/trunk/qt/jkqtpoverlayelements.h \
    qfsimplefitparameterinterface.h \
    qfsimplefitparameterswidget.h \
    ../../../../LIB/trunk/image_tools.h \
    ../../../../LIB/trunk/qt/qkeysequenceedit.h \
    qffilterchangercombobox.h \
    ../plugins/interfaces/qfextensionlightsource.h \
    ../plugins/interfaces/qfextensionfilterchanger.h \
    qflightsourcecombobox.h \
    qfshutterconfigwidget.h \
    qffilterchangerconfigwidget.h \
    qflightsourceconfigwidget.h \
    qflightsourceconfigwidgetthread.h \
    ../../../../LIB/trunk/qt/qprogresslistwidget.h \
    ../../../../LIB/trunk/qt/qsettingsmodel.h \
    ../../../../LIB/trunk/qt/linestylecombobox.h \
    qfacquisitionconfigcombobox.h \
    qfacquisitioncombobox.h \
    qfshutterconfigwidgetthread.h \
    qffilterchangerconfigwidgetthread.h \
    qffitfunctioncombobox.h \
    qffitalgorithmcombobox.h \
    qffitfunctionplottools.h \
    qftablemodel.h \
    qfhistogramview.h \
    qfselectionlistdialog.h \
    qfcompletertextedit.h \
    qfqtscripthighlighter.h \
    qfcompletertextedit.h \
    qfqtscripthighlighter.h \
    ../../../../LIB/trunk/qt/jkqtpparsedfunctionelements.h \
    ../../../../LIB/trunk/jkmathparser.h \
    qfhtmlhighlighter.h \
    qfplotter.h \
    qfplotterprivate.h \
    qfselectrdrdialog.h \
    ../plugins/interfaces/qfrdrtableinterface.h \
    ../plugins/interfaces/qfrdrcolumngraphsinterface.h \
    qfrdrtablecombobox.h \
    qfplottercopytotabledialog.h \
    qfmathparserxfunctionlinegraph.h \
    doubledatacutsliders.h \
    ../../../../LIB/trunk/qt/jkautooutputtimer.h \
    qfrecentfilesmenu.h \
    qfstageconfigwidgetthread.h \
    qfstageconfigwidget.h \
    ../../../../LIB/trunk/qt/fillstylecombobox.h \
    qfsimplecurvefitdialog.h \
    qfparametercorrelationview.h \
    ../../../../LIB/trunk/statistics_tools.h


SOURCES += ../../../../LIB/trunk/qt/jkqtpbaseplotter.cpp \
           ../../../../LIB/trunk/qt/jkqtpdatastorage.cpp \
           ../../../../LIB/trunk/qt/jkqtpelements.cpp \
           ../../../../LIB/trunk/qt/jkqtpgeoelements.cpp \
           ../../../../LIB/trunk/qt/jkqtmathtext.cpp \
           ../../../../LIB/trunk/qt/jkqtpbaseelements.cpp \
           ../../../../LIB/trunk/qt/jkqtplotter.cpp \
           ../../../../LIB/trunk/qt/jkqtptools.cpp \
           ../../../../LIB/trunk/qt/jkqttools.cpp \
            ../../../../LIB/trunk/qt/flowlayout.cpp \
           ../../../../LIB/trunk/qt/qtlogfile.cpp \
           ../../../../LIB/trunk/qt/jkqtfastplotter.cpp \
           ../../../../LIB/trunk/tools.cpp \
           ../../../../LIB/trunk/datatable2.cpp \
           ../../../../LIB/trunk/qt/jkverticalscrollarea.cpp \
           ../../../../LIB/trunk/qt/colorcombobox.cpp \
           ../../../../LIB/trunk/qt/jkstyledbutton.cpp \
           ../../../../LIB/trunk/qt/qenhancedlineedit.cpp \
           ../../../../LIB/trunk/qt/qspecialtoolbutton.cpp \
           ../../../../LIB/trunk/qt/qmodernprogresswidget.cpp \
           ../../../../LIB/trunk/qt/qmoretextobject.cpp \
           qehtreeview.cpp \
           datacutslider.cpp \
           qffitparameterwidget.cpp \
           qfdlgcsvparameters.cpp \
    qfstagecombobox.cpp \
    qfcameraconfigcombobox.cpp \
    qfcameracombobox.cpp \
    objectives.cpp \
    ../../../../LIB/trunk/qt/jkqtpimagetools.cpp \
    ../../../../LIB/trunk/qt/jkqtpimageelements.cpp \
    filters.cpp \
    ../../../../LIB/trunk/qt/qenhancedcombobox.cpp \
    qfshuttercombobox.cpp \
    ../../../../LIB/trunk/qt/qrecentfilesmenu.cpp \
    qfplayercontrols.cpp \
    ../../../../LIB/trunk/qt/jkqtpoverlayelements.cpp \
    qfsimplefitparameterswidget.cpp \
    ../../../../LIB/trunk/image_tools.cpp \
    ../../../../LIB/trunk/qt/qkeysequenceedit.cpp \
    qffilterchangercombobox.cpp \
    qflightsourcecombobox.cpp \
    qfshutterconfigwidget.cpp \
    qffilterchangerconfigwidget.cpp \
    qflightsourceconfigwidget.cpp \
    qflightsourceconfigwidgetthread.cpp \
    ../../../../LIB/trunk/qt/qprogresslistwidget.cpp \
    ../../../../LIB/trunk/qt/qsettingsmodel.cpp \
    ../../../../LIB/trunk/qt/linestylecombobox.cpp \
    qfacquisitionconfigcombobox.cpp \
    qfacquisitioncombobox.cpp \
    qfshutterconfigwidgetthread.cpp \
    qffilterchangerconfigwidgetthread.cpp \
    qffitfunctioncombobox.cpp \
    qffitalgorithmcombobox.cpp \
    qffitfunctionplottools.cpp \
    qftablemodel.cpp \
    qfhistogramview.cpp \
    qfselectionlistdialog.cpp \
    qfqtscripthighlighter.cpp \
    qfcompletertextedit.cpp \
    ../../../../LIB/trunk/qt/jkqtpparsedfunctionelements.cpp \
    ../../../../LIB/trunk/jkmathparser.cpp \
    qfhtmlhighlighter.cpp \
    qfplotter.cpp \
    qfplotterprivate.cpp \
    qfselectrdrdialog.cpp \
    qfrdrtablecombobox.cpp \
    qfplottercopytotabledialog.cpp \
    qfmathparserxfunctionlinegraph.cpp \
    doubledatacutsliders.cpp \
    ../../../../LIB/trunk/qt/jkautooutputtimer.cpp \
    qfrecentfilesmenu.cpp \
    qfstageconfigwidgetthread.cpp \
    qfstageconfigwidget.cpp \
    ../../../../LIB/trunk/qt/fillstylecombobox.cpp \
    qfsimplecurvefitdialog.cpp \
    qfparametercorrelationview.cpp \
    ../../../../LIB/trunk/statistics_tools.cpp


FORMS += qfdlg_csvparameters.ui \
    qf3objectiveeditor.ui \
    qf3filtereditor.ui \
    qfselectionlistdialog.ui \
    qfselectrdrdialog.ui \
    qfplottercopytotabledialog.ui \
    qfsimplecurvefitdialog.ui

RESOURCES += ../../../../LIB/trunk/qt/jkqtpbaseplotter.qrc \
             ../../../../LIB/trunk/qt/qspecialtoolbutton.qrc \
             ../../../../LIB/trunk/qt/jkstyledbutton.qrc \
             ../../../../LIB/trunk/qt/libqt.qrc \
             ../../../../LIB/trunk/qt/qkeysequenceedit.qrc \
             ../../../../LIB/trunk/qt/numberedit.qrc \
             libqf3widgets.qrc \
    ../../../../LIB/trunk/qt/qprogresslistwidget.qrc \
    ../../../../LIB/trunk/qt/qtlogfile.qrc \
    ../extlibs/xits.qrc

TRANSLATIONS= ./translations/de.quickfit3widgets.ts

INCLUDEPATH += ../../../../LIB/trunk/ \
               ../../../../LIB/trunk/qt/

CONFIG += dll lib_bundle exceptions rtti stl create_prl

DEFINES += QFWIDLIB_LIBRARY LIB_IN_DLL LIB_EXPORT_LIBRARY USE_XITS_FONTS
#AUTOLOAD_XITS_FONTS


ASSETSScript_FILES.files = ./assets/highlighter/qtscript/*.*
ASSETSScript_FILES.path = $${QFOUTPUT}/assets/highlighter/qtscript
ASSETSSHTML_FILES.files = ./assets/highlighter/html/*.*
ASSETSSHTML_FILES.path = $${QFOUTPUT}/assets/highlighter/html
ASSETSShigh_FILES.files = ./assets/highlighter/*.*
ASSETSShigh_FILES.path = $${QFOUTPUT}/assets/highlighter

INSTALLS += ASSETSScript_FILES ASSETSSHTML_FILES ASSETSShigh_FILES



XITS_SRC_DISTRIBUTED.files = ../extlibs/xits/*.zip
XITS_SRC_DISTRIBUTED.path = $${QFOUTPUT}/source/extlibs/xits/
XITS_SRC_DISTRIBUTED1.files = ../extlibs/xits/*.txt
XITS_SRC_DISTRIBUTED1.path = $${QFOUTPUT}/source/extlibs/xits/


INSTALLS += XITS_SRC_DISTRIBUTED XITS_SRC_DISTRIBUTED1
