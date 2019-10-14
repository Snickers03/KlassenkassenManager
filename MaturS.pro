#-------------------------------------------------
#
# Project created by QtCreator 2019-05-03T17:50:46
#
#-------------------------------------------------

QT       += core gui sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KlassenkassenManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    commands.cpp \
    export.cpp \
        main.cpp \
        mainwindow.cpp \
    payments.cpp \
    sure.cpp \
    addstudent.cpp \
    student.cpp

HEADERS += \
    commands.h \
    export.h \
        mainwindow.h \
    payments.h \
    sure.h \
    addstudent.h \
    student.h

FORMS += \
    export.ui \
        mainwindow.ui \
    sure.ui \
    addstudent.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES += \
    images/logo.png

include(3rdParty/qtxlsx/src/xlsx/qtxlsx.pri)
RC_FILE = manager.rc
