QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calarmwidget.cpp \
    cdevicelistwidget.cpp \
    cframelesswidgetbase.cpp \
    clogquerywidget.cpp \
    copenfilebutton.cpp \
    cptzcontrolwidget.cpp \
    cryptstring.cpp \
    csystemsettingswidget.cpp \
    ctimeslider.cpp \
    ctopmenubar.cpp \
    cvideowidgettopwidget.cpp \
    cwindowinfowidget.cpp \
    faceapimanager.cpp \
    faceregisterdialog.cpp \
    ffmpegkits.cpp \
    main.cpp \
    mainwidget.cpp \
    notificationserver.cpp

HEADERS += \
    calarmwidget.h \
    cdevicelistwidget.h \
    cframelesswidgetbase.h \
    clogquerywidget.h \
    copenfilebutton.h \
    cptzcontrolwidget.h \
    cryptstring.h \
    csystemsettingswidget.h \
    ctimeslider.h \
    ctopmenubar.h \
    cvideowidgettopwidget.h \
    cwindowinfowidget.h \
    faceapimanager.h \
    faceregisterdialog.h \
    ffmpegkits.h \
    mainwidget.h \
    notificationserver.h

FORMS += \
    mainwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lavcodec

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lavdevice

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lavfilter

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lavformat

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lavutil

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -llibpostproc

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -llibswresample

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

win32: LIBS += -L$$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/lib/x86/ -lswscale

INCLUDEPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include
DEPENDPATH += $$PWD/../../libffmpeg_4.4.r101753_msvc16_x86/include

RESOURCES += \
    sources.qrc

DISTFILES +=
