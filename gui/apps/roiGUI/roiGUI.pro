######################################################################
# project file for roiGUI
######################################################################

TEMPLATE = app
TARGET = roiGUI
DESTDIR = bin/ 
DEPENDPATH += ./ ../../lib 

MOC_DIR = moc/
OBJECTS_DIR = obj/
RCC_DIR = res/
UI_DIR = ../../widgets/roi

CONFIG(release, debug|release) {
    CONFIG += optimize_full
}

CONFIG += c++14
CONFIG += qwt
exists( $$(CONDA_PREFIX)/include ) {
    INCLUDEPATH += $$(CONDA_PREFIX)/include
}

MAKEFILE = makefile.roiGUI

# Input
INCLUDEPATH += ../../lib ../../widgets/roi

HEADERS += ../../widgets/roi/roi.hpp \
           ../../lib/multiIndi.hpp \
           ../../lib/multiIndiManager.hpp
           
SOURCES += roiGUI_main.cpp 
#\
#           ../../widgets/roi/roi.cpp \
           
FORMS += ../../widgets/roi/roi.ui
     
LIBS += ../../../INDI/libcommon/libcommon.a \
        ../../../INDI/liblilxml/liblilxml.a

RESOURCES += ../../resources/MagAOXStyleSheets/MagAOXStyle.qrc 

QT += widgets
