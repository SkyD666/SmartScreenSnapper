QT       += core gui winextras printsupport

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
    MyGlobalShortcut/MyGlobalShortCut.cpp \
    MyGlobalShortcut/MyWinEventFilter.cpp \
    aboutdialog.cpp \
    application.cpp \
    freesnapdialog.cpp \
    graphicsview.cpp \
    main.cpp \
    mainwindow.cpp \
    mdiwindow.cpp \
    publicdata.cpp \
    settingdialog.cpp \
    star.cpp \
    updatedialog.cpp \
    windowsinfo.cpp

HEADERS += \
    MyGlobalShortcut/MyGlobalShortCut.h \
    MyGlobalShortcut/MyWinEventFilter.h \
    aboutdialog.h \
    application.h \
    freesnapdialog.h \
    graphicsview.h \
    mainwindow.h \
    mdiwindow.h \
    minifmod.h \
    publicdata.h \
    settingdialog.h \
    star.h \
    updatedialog.h \
    windowsinfo.h

FORMS += \
    aboutdialog.ui \
    freesnapdialog.ui \
    mainwindow.ui \
    settingdialog.ui \
    updatedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

RC_FILE += RCData.rc

unix|win32: LIBS += -L$$PWD/./ -lminifmod

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix|win32: LIBS += -L$$PWD/./ -lWinMM

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix|win32: LIBS += -L$$PWD/./ -lGDI32

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix|win32: LIBS += -L$$PWD/./ -lMSImg32

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix|win32: LIBS += -L$$PWD/./ -lWININET

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
