MAJOR = 1
MINOR = 0
PATCH = 0

TARGET = se_win
TEMPLATE = lib
QT += core gui qml

include(../system.pri)

DEFINES += WIN32 _WINDOWS NOMINMAX
#_USRDLL _AFXDLL _WINDLL
DEFINES -= UNICODE

INCLUDEPATH += C:/WinDDK/7600.16385.1/inc/atl71 \
               C:/WinDDK/7600.16385.1/inc/mfc42 \
               "$$SEINCLUDE" \
               "$$SETYPELIBS"

LIBS += -L"$$SEINCLUDE" -L"$$SETYPELIBS" -LC:/WinDDK/7600.16385.1/lib/Mfc/amd64 -LC:/WinDDK/7600.16385.1/lib/ATL/amd64

LIBS += -lhtmlhelp

LIBS += -L../extension/$$DESTDIR \
        -lextension

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

IDLFLAGS += -I\"$$SETYPELIBS\" /mktyplib203 /W1 /nologo /char signed /env win64

idl_h.output = ${QMAKE_FILE_IN_BASE}_i.h
idl_h.input = IDL
idl_h.commands = $${QMAKE_IDL} $${IDLFLAGS} \
                 /h ${QMAKE_FILE_IN_BASE}_i.h ${QMAKE_FILE_IN}
idl_h.variable_out = HEADERS
idl_h.name = MIDL

QMAKE_EXTRA_COMPILERS += idl_h

DEF_FILE = src/se_win.def
RC_FILE = src/se_win.rc
IDL = src/se_win.idl

QMAKE_DISTCLEAN += se_win.tlb \
                   se_win_i.c

HEADERS += \
    src/SEAddin.h \
    src/commands.h \
    src/util.h \
    src/hlp/AsmLochelp.h \
    src/SolidEdgeExtensionImpl.h \
    src/TopWindow.h \
    src/command.h \
    src/document.h \
    src/GeometryUtils.h \
    src/Utils.h \
    src/Octree.h \
    src/Point3D.h

SOURCES += \
    src/se_win.cpp \
    src/SEAddin.cpp \
    src/commands.cpp \
    src/command.cpp \
    src/util.cpp \
    src/document.cpp \
    src/RegSEAddin.cpp \
    src/SolidEdgeExtensionImpl.cpp \
    src/TopWindow.cpp \
    src/Post/Sketch.cpp \
    src/Post/CSysUtils.cpp \
    src/Post/CreateGeometry.cpp \
    src/Post/Select.cpp \
    src/GeometryUtils.cpp \
    src/Pre/Preprocess.cpp \
    src/Pre/Naming.cpp \
    src/DumpTopology.cpp \
    src/Utils.cpp \
    src/Octree.cpp \
    src/NamingTest.cpp \
    src/Pre/SketchCreatePre.cpp
