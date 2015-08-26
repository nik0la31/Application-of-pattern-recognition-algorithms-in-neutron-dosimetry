#-------------------------------------------------
#
# Neutron Dosimetry Trace Detector App
#
#-------------------------------------------------

QT       += core gui
QT       += printsupport

CONFIG   += console

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ndtr_app
TEMPLATE = app

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    utils.cpp \
    projectparser.cpp \
    projectitem.cpp \
    workspace.cpp \

HEADERS += \
    mainwindow.h \
    utils.h \
    projectparser.h \
    projectitem.h \
    workspace.h \

FORMS += \
    mainwindow.ui

OTHER_FILES +=

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD/../ndtr_engine
DEPENDPATH += $$PWD/../ndtr_engine

win32 {
    INCLUDEPATH += \
        C:/opencv/build/include \
        C:/opencv/build/include/opencv \
        C:/opencv/build/include/opencv2 \

    DEPENDPATH += \
        C:/opencv/build/x86/vc12/

    CONFIG(release, debug|release) {
        LIBS += -LC:/opencv/build/x86/vc12/lib/ -lopencv_world300
        LIBS += -L$$OUT_PWD/../ndtr_engine/release/ -lndtr_engine

    }

    CONFIG(debug, debug|release) {
        LIBS += -LC:/opencv/build/x86/vc12/lib/ -lopencv_world300d
        LIBS += -L$$OUT_PWD/../ndtr_engine/debug/ -lndtr_engine
    }

    DEFINES += \
        WINDOWS
}

unix {
    target.path = /usr/lib
    INSTALLS += target

    INCLUDEPATH += \
        /usr/local/include/opencv \
        /usr/local/include/opencv2 \

    LIBS += -L$$OUT_PWD/../ndtr_engine/ -lndtr_engine

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

    PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/libndtr_engine.a
}

DISTFILES += \
    ../../../docs/master/master-rad.pdf

docroot.files  = $$DISTFILES
docroot.path   = $${OUT_PWD}
INSTALLS       += docroot
