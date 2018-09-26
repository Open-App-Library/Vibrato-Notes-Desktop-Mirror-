#-------------------------------------------------
#
# Project created by QtCreator 2018-09-19T15:41:41
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = VibratoNotes-Desktop
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
  src/appconfig.cpp \
  src/main.cpp \
  src/mainwindow.cpp \
  src/userwindow.cpp \
  src/notelistmanager.cpp \
  src/notelistitem.cpp \
  src/note.cpp \
  src/treemanager.cpp

HEADERS += \
  src/appconfig.h \
  src/appinfo.h \
  src/mainwindow.h \
  src/userwindow.h \
  src/notelistmanager.h \
  src/notelistitem.h \
  src/note.h \
  src/helper-io.h \
  src/treemanager.h

FORMS += \
    ui/mainwindow.ui \
    ui/userwindow.ui \
    ui/notelistitem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Mac-specific things
macx {

}

# Mac & Linux
unix {
    #RESOURCES += # elementary-icons.qrc # Add Elementary icons as fallback icons
}

# Linux / BSD (NOT Mac) specfic things
unix:!macx {

}

# Windows-Specific things
win32 {
    # TODO: Add Colibre icon theme as fallback
}


RESOURCES += \
    resources/breeze-icons.qrc \
    resources/dummy-data.qrc
