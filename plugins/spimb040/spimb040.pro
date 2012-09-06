TEMPLATE = lib
CONFIG += plugin


TARGET = spimb040
DEPENDPATH += ./


include(../plugins.pri)
include(../../extlibs/lmfit.pri)
include(../../extlibs/tiff.pri)

DESTDIR = $$QFOUTPUT/plugins
include(../../libquickfitwidgets.pri)

# Input
HEADERS += spimb040.h \
           qfespimb040cameraview.h \
           qfespimb040samplestageconfig.h \
           ../interfaces/qfextensioncamera.h \
           ../../../../../LIB/trunk/jkimage.h  \
           ../../../../../LIB/trunk/highrestimer.h \
           ../../../../../LIB/trunk/statistics_tools.h \
           ../../../../../LIB/trunk/tinytiffwriter.h \
           ../../../../../LIB/trunk/libtiff_tools.h \
           ../interfaces/qfextensionlinearstage.h \
           ../interfaces/qfextensionparameterdevice.h \
    qfespimb040opticssetup.h \
    qfespimb040simplecameraconfig.h \
    qfespimb00histogrammaskdialog.h \
    ../../../../../LIB/trunk/qt/qfasttablelabel.h \
    ../../../../../LIB/trunk/histogram.h \
    qfespimb040mainwindow2.h \
    qfespimb040experimentdescription.h \
    qfespimb040samplestageconfigthread.h \
    qfespimb040imagestackconfigwidget2.h \
    qfespimb040acquisitionconfigwidget2.h \
    qfespimb040configcombobox.h \
    qfespimb040samplestagestepconfigdialog.h \
    qfespimb040shortcutconfigdialog.h \
    qfespimb040camparamstackconfigwidget2.h \
    qfespimb040filenametool.h \
    qfespimb040lightpathsavedialog.h \
    ../interfaces/qfextensionmeasurementdevice.h \
    qfespimb040acquisitiondescription.h \
    ../interfaces/qfextensionglobalsettingsreadwrite.h \
    qfespimb040deviceparamstackconfigwidget.h \
    qfespimb040acquisitiontools.h

SOURCES += spimb040.cpp \
           qfespimb040cameraview.cpp \
           qfespimb040samplestageconfig.cpp \
           ../../../../../LIB/trunk/highrestimer.cpp \
           ../../../../../LIB/trunk/statistics_tools.cpp \
           ../../../../../LIB/trunk/tinytiffwriter.cpp \
           ../../../../../LIB/trunk/libtiff_tools.cpp \
    qfespimb040opticssetup.cpp \
    qfespimb040simplecameraconfig.cpp \
    qfespimb00histogrammaskdialog.cpp \
    ../../../../../LIB/trunk/qt/qfasttablelabel.cpp \
    qfespimb040mainwindow2.cpp \
    qfespimb040experimentdescription.cpp \
    qfespimb040samplestageconfigthread.cpp \
    qfespimb040imagestackconfigwidget2.cpp \
    qfespimb040acquisitionconfigwidget2.cpp \
    qfespimb040configcombobox.cpp \
    qfespimb040samplestagestepconfigdialog.cpp \
    qfespimb040shortcutconfigdialog.cpp \
    qfespimb040camparamstackconfigwidget2.cpp \
    qfespimb040filenametool.cpp \
    qfespimb040lightpathsavedialog.cpp \
    qfespimb040acquisitiondescription.cpp \
    qfespimb040deviceparamstackconfigwidget.cpp
FORMS = \
    qfespimb040opticssetup.ui \
    qfespimb00histogrammaskdialog.ui \
    qfespimb040experimentdescription.ui \
    qfespimb040imagestackconfigwidget2.ui \
    qfespimb040acquisitionconfigwidget2.ui \
    qfespimb040samplestagestepconfigdialog.ui \
    qfespimb040shortcutconfigdialog.ui \
    qfespimb040camparamstackconfigwidget2.ui \
    qfespimb040lightpathsavedialog.ui \
    qfespimb040acquisitiondescription.ui \
    qfespimb040deviceparamstackconfigwidget.ui

RESOURCES += spimb040.qrc

TRANSLATIONS= ./translations/de.spimb040.ts

INCLUDEPATH += . \
               ../../lib/ \
               ../../libqf3widgets/ \
               ../../../../../LIB/trunk/ \
               ../../../../../LIB/trunk/qt/

QT += gui xml svg opengl

CONFIG += exceptions rtti stl
























