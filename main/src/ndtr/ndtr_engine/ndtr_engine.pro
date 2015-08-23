#-------------------------------------------------
#
# Project created by QtCreator 2014-07-24T01:39:54
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

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += C:/opencv/build/include \
                C:/opencv/build/include/opencv \
                C:/opencv/build/include/opencv2

#INCLUDEPATH += C:/opencv/build/x86/vc10
DEPENDPATH += C:/opencv/build/x86/vc12/

##########

win32:CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300 \
#    -lopencv_core249 \
#    -lopencv_imgproc249 \
#    -lopencv_highgui249 \
#    -lopencv_ml249 \
#    -lopencv_video249 \
#    -lopencv_features2d249 \
#    -lopencv_calib3d249 \
#    -lopencv_objdetect249 \
#    -lopencv_contrib249 \
#    -lopencv_legacy249 \
#    -lopencv_flann249 \

else:win32:CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300d \
#    -lopencv_core300d \
#    -lopencv_imgproc249d \
#    -lopencv_highgui249d \
#    -lopencv_ml249d \
#    -lopencv_video249d \
#    -lopencv_features2d249d \
#    -lopencv_calib3d249d \
#    -lopencv_objdetect249d \
#    -lopencv_contrib249d \
#    -lopencv_legacy249d \
#    -lopencv_flann249d \
