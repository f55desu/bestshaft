MAJOR = 1
MINOR = 0
PATCH = 0

TARGET = extension
TEMPLATE = lib
QT += qml xml widgets
CONFIG += staticlib

HEADERS += \
    src/BestShaftWindow.h \
    src/Stable.h \
    src/ExtensionWindow.h \
    src/IAbstractModeler.h \
    src/BaseExtension.h \
    src/Syntaxhighlighter.h \
    src/Point.h

SOURCES += \
    src/BestShaftWindow.cpp \
    src/ExtensionWindow.cpp \
    src/BaseExtension.cpp \
    src/Postprocess.cpp \
    src/Syntaxhighlighter.cpp

FORMS += \
    src/BestShaftWindow.ui \
    src/ExtensionWindow.ui

RESOURCES += resources.qrc

TRANSLATIONS = ui/extension_ru.ts

include(../system.pri)
