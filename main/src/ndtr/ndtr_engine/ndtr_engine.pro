#-------------------------------------------------
#
# Neutron Dosimetry Trace Detector Engine
#
#-------------------------------------------------

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = ndtr_engine
TEMPLATE = lib
CONFIG += staticlib
#CONFIG   += console

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

    CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x64/vc12/lib/ \
        -lopencv_world300 \

    CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x64/vc12/lib/ \
        -lopencv_world300d \
}

unix {
    target.path = /usr/lib
    INSTALLS += target

    INCLUDEPATH += \
        /usr/local/include/opencv \
        /usr/local/include/opencv2 \

    LIBS += -L/usr/local/lib
    LIBS += -lopencv_core
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_highgui
    LIBS += -lopencv_ml
    LIBS += -lopencv_video
    LIBS += -lopencv_features2d
    LIBS += -lopencv_calib3d
    LIBS += -lopencv_objdetect
    LIBS += -lopencv_imgcodecs
    #LIBS += -lopencv_contrib
    #LIBS += -lopencv_legacy
    LIBS += -lopencv_flann
    #LIBS += -lopencv_nonfree
}
