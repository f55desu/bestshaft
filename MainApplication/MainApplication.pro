MAJOR = 1
MINOR = 0
PATCH = 0

TARGET = MainApplication
TEMPLATE = app
QT += qml widgets

include(../system.pri)

LIBS += -lextension \
        -lcsvparser

LIBS += -L../extension/$$DESTDIR

CONFIG(debug, debug|release) {
  win32-msvc* {

    LIBS += -llog4cppd \
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

    LIBS += -llog4cpp

    #Targets depend on
    PRE_TARGETDEPS += $$LOG4CPP_LIB_RELEASE/log4cpp.lib \
                      $$CVS_PARSER_LIB_RELEASE/csvparser.lib \
                      ../extension/$$DESTDIR/extension.lib
  }
}

SOURCES += src/main.cpp\
           src/MainWindow.cpp \
           src/ExtensionImpl.cpp

HEADERS  += src/MainWindow.h \
            src/Stable.h \
            src/ExtensionImpl.h

FORMS += src/MainWindow.ui