MAJOR = 1
MINOR = 0
PATCH = 0

LIBS += -lUser32

TARGET = extension
TEMPLATE = lib
QT += qml xml widgets
CONFIG += staticlib

HEADERS += \
    src/Stable.h \
    src/ExtensionWindow.h \
    src/BaseExtension.h \
    src/SettingsDialog.h

SOURCES += \
    src/ExtensionWindow.cpp \
    src/BaseExtension.cpp \
    src/SettingsDialog.cpp

FORMS += \
    src/ExtensionWindow.ui \
    src/SettingsDialog.ui

#RESOURCES += resources.qrc

TRANSLATIONS = ui/extension_ru.ts

include(../system.pri)
