######################################################################
# project file for dmModeGUI
######################################################################

TEMPLATE = app
TARGET = dmModeGUI
DESTDIR = bin/ 
DEPENDPATH += ./ ../../lib 

MOC_DIR = moc/
OBJECTS_DIR = obj/
RCC_DIR = res/
UI_DIR = ../../widgets/dmMode

CONFIG(release, debug|release) {
    CONFIG += optimize_full
}
CONFIG += c++14
CONFIG += qwt
exists( $$(CONDA_PREFIX)/include ) {
    INCLUDEPATH += $$(CONDA_PREFIX)/include
}

MAKEFILE = makefile.dmModeGUI

# Input
INCLUDEPATH += ../../lib ../../widgets/dmMode

HEADERS += ../../widgets/dmMode/dmMode.hpp \
           ../../lib/multiIndi.hpp \
           ../../lib/multiIndiManager.hpp
           
SOURCES += dmModeGUI_main.cpp
           
FORMS += ../../widgets/dmMode/dmMode.ui
     
LIBS += ../../../INDI/libcommon/libcommon.a \
        ../../../INDI/liblilxml/liblilxml.a 

RESOURCES += ../../resources/MagAOXStyleSheets/MagAOXStyle.qrc 

QT += widgets
