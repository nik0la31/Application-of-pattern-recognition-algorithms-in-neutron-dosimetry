#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T12:58:25
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ndtr-generator
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=gnu++0x

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += C:/opencv/build/include \
                C:/opencv/build/include/opencv \
                C:/opencv/build/include/opencv2

#INCLUDEPATH += C:/opencv/build/x86/vc10
DEPENDPATH += C:/opencv/build/x86/vc12/

##########

win32:CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300 \

else:win32:CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300d \

HEADERS +=
