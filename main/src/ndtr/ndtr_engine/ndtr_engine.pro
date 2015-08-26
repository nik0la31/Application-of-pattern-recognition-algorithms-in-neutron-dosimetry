#-------------------------------------------------
#
# Neutron Dosimetry Trace Detector Engine
#
#-------------------------------------------------

QT       -= gui

TARGET = ndtr_engine
TEMPLATE = lib
CONFIG += staticlib

CONFIG   += console

DEFINES += NDTR_ENGINE_LIBRARY \
            _CRT_SECURE_NO_WARNINGS

SOURCES += \
    project.cpp \
    document.cpp \
    viewoptions.cpp \
    stats.cpp \
    trace.cpp \
    processingoptions.cpp

HEADERS +=\
    project.h \
    document.h \
    viewoptions.h \
    stats.h \
    trace.h \
    processingoptions.h

win32 {
    INCLUDEPATH += C:/opencv/build/include \
                C:/opencv/build/include/opencv \
                C:/opencv/build/include/opencv2

    DEPENDPATH += C:/opencv/build/x86/vc12/

    CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
        -lopencv_world300 \

    CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
        -lopencv_world300d \
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
