#-------------------------------------------------
#
# Neutron Dosimetry Trace Detector App
#
#-------------------------------------------------

QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ndtr_app
TEMPLATE = app

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    utils.cpp \
    projectparser.cpp \
    projectitem.cpp
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
    LIBS += -L$$OUT_PWD/../ndtr_engine/ -lndtr_engine

    PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/libndtr_engine.a
}

DISTFILES += \
    ../../../docs/master/master-rad.pdf

docroot.files  = $$DISTFILES
docroot.path   = $${OUT_PWD}
INSTALLS       += docroot
