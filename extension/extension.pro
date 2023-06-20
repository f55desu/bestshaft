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

TRANSLATIONS += \
            ui/extension_en.ts \
            ui/extension_ru.ts \
            ui/extension_jp.ts \
            ui/extension_it.ts \
            ui/extension_de.ts \
            ui/extension_zh.ts

include(../system.pri)

DISTFILES += \
    ui/extension_en.qm \
    ui/extension_jp.qm
