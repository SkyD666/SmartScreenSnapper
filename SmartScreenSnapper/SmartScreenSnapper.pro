QT       += core gui printsupport svg network multimedia

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
    src/MyGlobalShortcut/MyGlobalShortCut.cpp \
    src/MyGlobalShortcut/MyWinEventFilter.cpp \
    src/aboutdialog.cpp \
    src/application.cpp \
    src/const.cpp \
    src/freehandsnapdialog.cpp \
    src/freehandsnap/freehandsnapgraphicsscene.cpp \
    src/freesnap/freesnapgraphicsview.cpp \
    src/freesnap/freesnaprect.cpp \
    src/freesnapdialog.cpp \
    src/graphicsitem/graphicspixmapitem.cpp \
    src/graphicsscene.cpp \
    src/screenshotdisplaydialog.cpp \
    src/snapbypoint/snapbypointgraphicsscene.cpp \
    src/gif-h/gif.cpp \
    src/gifdialog.cpp \
    src/graphicsview.cpp \
    src/longsnapdialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mdiwindow.cpp \
    src/publicdata.cpp \
    src/screenshothelper.cpp \
    src/settingdialog.cpp \
    src/snapfrompointdialog.cpp \
    src/undo/undomove.cpp \
    src/updatedialog.cpp \
    src/updateutil.cpp \
    src/util.cpp \
    src/windowsinfo.cpp

HEADERS += \
    src/MyGlobalShortcut/MyGlobalShortCut.h \
    src/MyGlobalShortcut/MyWinEventFilter.h \
    src/aboutdialog.h \
    src/application.h \
    src/const.h \
    src/freehandsnapdialog.h \
    src/freehandsnap/freehandsnapgraphicsscene.h \
    src/freesnap/freesnapgraphicsview.h \
    src/freesnap/freesnaprect.h \
    src/freesnapdialog.h \
    src/gif-h/gif.h \
    src/gifdialog.h \
    src/graphicsitem/graphicspixmapitem.h \
    src/graphicsscene.h \
    src/graphicsview.h \
    src/longsnapdialog.h \
    src/mainwindow.h \
    src/mdiwindow.h \
    src/publicdata.h \
    src/screenshotdisplaydialog.h \
    src/screenshothelper.h \
    src/settingdialog.h \
    src/snapbypoint/snapbypointgraphicsscene.h \
    src/snapfrompointdialog.h \
    src/undo/undomove.h \
    src/updatedialog.h \
    src/updateutil.h \
    src/util.h \
#    src/psd_sdk/PsdExport.h \
#    src/psd_sdk/PsdExportColorMode.h \
#    src/psd_sdk/PsdExportChannel.h \
#    src/psd_sdk/PsdExportDocument.h \
#    src/psd_sdk/PsdExportMetaDataAttribute.h \
#    src/psd_sdk/PsdExportLayer.h \
#    src/psd_sdk/PsdCompressionType.h \
#    src/psd_sdk/PsdAlphaChannel.h \
#    src/psd_sdk/PsdFixedSizeString.h \
#    src/psd_sdk/PsdAssert.h \
#    src/psd_sdk/PsdAllocator.h \
#    src/psd_sdk/PsdMallocAllocator.h \
#    src/psd_sdk/PsdFile.h \
#    src/psd_sdk/PsdNativeFile.h \
    src/windowsinfo.h

FORMS += \
    ui/screenshotdisplaydialog.ui \
    ui/mdiwindowwidget.ui \
    ui/snapfrompointdialog.ui \
    ui/freehandsnapdialog.ui \
    ui/longsnapdialog.ui \
    ui/aboutdialog.ui \
    ui/freesnapdialog.ui \
    ui/gifdialog.ui \
    ui/mainwindow.ui \
    ui/settingdialog.ui \
    ui/updatedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/res.qrc

RC_FILE += res/RCData.rc

unix|win32: LIBS += \
    -L$$PWD/./lib/ -lGDI32

INCLUDEPATH += $$PWD/./lib/
DEPENDPATH += $$PWD/./lib/

unix|win32: LIBS += \
    -L$$PWD/./lib/ -lpsd_sdk

#win32: LIBS += $$PWD/./lib/Psd_MD.lib

INCLUDEPATH += $$PWD/./lib/
DEPENDPATH += $$PWD/./lib/
