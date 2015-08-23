#-------------------------------------------------
#
# Project created by QtCreator 2014-07-23T21:52:51
#
#-------------------------------------------------

QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ndtr_app
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    workspace.cpp \
    utils.cpp \
    projectparser.cpp \
    projectitem.cpp

HEADERS  += mainwindow.h \
    workspace.h \
    utils.h \
    projectparser.h \
    projectitem.h

FORMS    += mainwindow.ui

OTHER_FILES +=

RESOURCES += \
    resources.qrc

#####

#QMAKE_CXXFLAGS += /WX

#####

INCLUDEPATH += C:/opencv/build/include \
                C:/opencv/build/include/opencv \
                C:/opencv/build/include/opencv2

#INCLUDEPATH += C:/opencv/build/x86/vc10
DEPENDPATH += C:/opencv/build/x86/vc12/

win32:CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300 \

else:win32:CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x86/vc12/lib/ \
    -lopencv_world300d \

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ndtr_engine/release/ -lndtr_engine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ndtr_engine/debug/ -lndtr_engine
else:unix: LIBS += -L$$OUT_PWD/../ndtr_engine/ -lndtr_engine

INCLUDEPATH += $$PWD/../ndtr_engine
DEPENDPATH += $$PWD/../ndtr_engine

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/release/libndtr_engine.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/debug/libndtr_engine.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/release/ndtr_engine.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/debug/ndtr_engine.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ndtr_engine/libndtr_engine.a

DISTFILES += \
    ../../../docs/master/master-rad.pdf

docroot.files  = $$DISTFILES
docroot.path   = $${OUT_PWD}
INSTALLS       += docroot

win32: DEFINES += WINDOWS
unix:
