######################################################################
# project file for dmCtrlGUI
######################################################################

TEMPLATE = app
TARGET = dmCtrlGUI
DESTDIR = bin/ 
DEPENDPATH += ./ ../../lib 

MOC_DIR = moc/
OBJECTS_DIR = obj/
RCC_DIR = res/
UI_DIR = ../../widgets/dmCtrl

CONFIG(release, debug|release) {
    CONFIG += optimize_full
}

CONFIG += c++14
CONFIG += qwt
exists( $$(CONDA_PREFIX)/include ) {
    INCLUDEPATH += $$(CONDA_PREFIX)/include
}

MAKEFILE = makefile.dmCtrlGUI

# Input
INCLUDEPATH += ../../lib ../../widgets/dmCtrl

HEADERS += ../../widgets/dmCtrl/dmCtrl.hpp \
           ../../lib/multiIndi.hpp \
           ../../lib/multiIndiManager.hpp
           
SOURCES += dmCtrlGUI_main.cpp 
#\
#           ../../widgets/dmCtrl/dmCtrl.cpp \
           
FORMS += ../../widgets/dmCtrl/dmCtrl.ui
     
LIBS += ../../../INDI/libcommon/libcommon.a \
        ../../../INDI/liblilxml/liblilxml.a

RESOURCES += ../../resources/MagAOXStyleSheets/MagAOXStyle.qrc 

QT += widgets
