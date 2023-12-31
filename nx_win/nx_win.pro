MAJOR = 1
MINOR = 0
PATCH = 0

TARGET = nx_win
TEMPLATE = lib

CONFIG += shared

QT += qml widgets

include(../system.pri)

DEFINES += LIBRARY DLL WNT

INCLUDEPATH += "$$NXINCLUDE"

LIBS += -L"$$NXINCLUDE"

win32-msvc*: LIBS += user32.lib advapi32.lib

LIBS += -lextension -llibufun -llibugopenint

DLLDESTDIR = ../$$DESTDIR/startup

LIBS += -L../extension/$$DESTDIR

DISTFILES = src/bestshaft.men

CONFIG(debug, debug|release) {
  win32-msvc* {

    LIBS += -llog4cppd \
            -lcsvparser \
            -lmemoryleakdetector

    #Targets depend on
    PRE_TARGETDEPS += $$LOG4CPP_LIB_DEBUG/log4cppd.lib \
                      $$CVS_PARSER_LIB_DEBUG/csvparser.lib \
                      ../extension/$$DESTDIR/extension.lib \
                      $$MEMEORY_LEAK_DETECTOR_LIB/memoryleakdetector.lib
  }
}

CONFIG(release, debug|release) {
  win32-msvc* {

    LIBS += -llog4cpp \
            -lcsvparser

    #Targets depend on
    PRE_TARGETDEPS += $$LOG4CPP_LIB_RELEASE/log4cpp.lib \
                      $$CVS_PARSER_LIB_RELEASE/csvparser.lib \
                      ../extension/$$DESTDIR/extension.lib
  }
}

HEADERS += \
    src/TopWindow.h \
    src/Stable.h \
    src/NXExtensionImpl.h

SOURCES += \
    src/TopWindow.cpp \
    src/main.cpp \
    src/Windows.cpp \
    src/Callbacks.cpp \
    src/NXExtensionImpl.cpp

STARTUP_MENU_FILE = $$relative_path($$PWD/src/bestshaft.men,$$OUT_PWD)

startupmenufile.target = $$system_path($$DLLDESTDIR/$$basename(STARTUP_MENU_FILE))
win32:startupmenufile.commands = copy /Y $$system_path($$STARTUP_MENU_FILE) \
                                 $$system_path($$DLLDESTDIR/$$basename(STARTUP_MENU_FILE))
startupmenufile.depends = $$system_path($$STARTUP_MENU_FILE)
QMAKE_EXTRA_TARGETS += startupmenufile

QMAKE_DISTCLEAN += $$startupmenufile.target version.h

#Add custom targets to build queue without TARGET dependency
all.depends += startupmenufile
QMAKE_EXTRA_TARGETS += all
