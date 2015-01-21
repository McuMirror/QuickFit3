#
#    Copyright (c) 2008-2014 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg
#	
#	 last modification: $LastChangedDate$  (revision $Rev$)
#
#    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).
#
#    This software is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib
CONFIG += plugin
TARGET = servo_pololu_maestro
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./


include(../plugins.pri)


DESTDIR = $$QFOUTPUT/plugins


include(../../libquickfitwidgets.pri)

# Input
HEADERS += servo_pololu_maestro.h \
           ../interfaces/qfextensionshutter.h \
           ../../../../../LIB/trunk/jkserialconnection.h \
    ../base_classes/qfbinaryserialprotocolhandler.h \
    ../base_classes/qf3comportmanager.h \
    dlgpololumaestrotester.h \
    pololu_tools.h \
    ../interfaces/qfextensionfilterchanger.h

SOURCES += servo_pololu_maestro.cpp \
           ../../../../../LIB/trunk/jkserialconnection.cpp \
    ../base_classes/qfbinaryserialprotocolhandler.cpp \
    ../base_classes/qf3comportmanager.cpp \
    dlgpololumaestrotester.cpp \
    pololu_tools.cpp

FORMS = \
    dlgpololumaestrotester.ui

RESOURCES += servo_pololu_maestro.qrc

TRANSLATIONS= ./translations/de.servo_pololu_maestro.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/ \
               ../../../../../LIB/trunk/ \
               ../../../../../LIB/trunk/qt/

include(../../quickfit3_configqtwidgets.pri)






