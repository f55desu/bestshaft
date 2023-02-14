lessThan(QT_MAJOR_VERSION, 6) {
    error("Qt version 6.0.0 and above is supported only")
}

!exists(config.pri) {
    win32:config_content += "MEMORY_LEAK_DETECTOR_INCLUDE = set you external third part include path here"
    win32:config_content += "MEMORY_LEAK_DETECTOR_LIB = set you external third part library path here"
    config_content += "LOG4CPP_INCLUDE = C:\vcpkg\installed\x64-windows\include\log4cpp"
    config_content += "LOG4CPP_LIB_DEBUG = set you external third part library path here"
    config_content += "LOG4CPP_LIB_RELEASE = set you external third part library path here"
    config_content += "CVS_PARSER_INCLUDE = set you external third part library path here"
    config_content += "CVS_PARSER_LIB_DEBUG = set you external third part library path here"
    config_content += "CVS_PARSER_LIB_RELEASE = set you external third part library path here"
    config_content += "BOOST_INCLUDE = C:\vcpkg\installed\x64-windows\include\boost"
    config_content += "BOOST_LIB_DEBUG = set you external third part library path here"
    config_content += "BOOST_LIB_RELEASE = set you external third part library path here"
    config_content += "SEINCLUDE = set you external third part include path here"
    config_content += "SETYPELIBS = set you external third part library path here"
    config_content += "NXINCLUDE = C:/Program Files/Siemens/NX2007/UGOPEN"
    win32:config_content += "MSYS64 = set you MSYS path here"

    write_file(config.pri, config_content)
    error("Setup variables in config.pri and run qmake again")
}

include(config.pri)

# Print paths defined in the config.pri file
message( LOG4CPP_INCLUDE = $${LOG4CPP_INCLUDE} )
message( LOG4CPP_LIB_DEBUG = $${LOG4CPP_LIB_DEBUG} )
message( LOG4CPP_LIB_RELEASE = $${LOG4CPP_LIB_RELEASE} )
message( MEMEORY_LEAK_DETECTOR_INCLUDE = $${MEMEORY_LEAK_DETECTOR_INCLUDE} )
message( MEMEORY_LEAK_DETECTOR_LIB = $${MEMEORY_LEAK_DETECTOR_LIB} )
message( CVS_PARSER_INCLUDE = $${CVS_PARSER_INCLUDE} )
message( CVS_PARSER_LIB_DEBUG = $${CVS_PARSER_LIB_DEBUG} )
message( CVS_PARSER_LIB_RELEASE = $${CVS_PARSER_LIB_RELEASE} )
message( BOOST_INCLUDE = $${BOOST_INCLUDE} )
message( BOOST_LIB_DEBUG = $${BOOST_LIB_DEBUG} )
message( BOOST_LIB_RELEASE = $${BOOST_LIB_RELEASE} )
message( SEINCLUDE = $${SEINCLUDE} )
message( SETYPELIBS = $${SETYPELIBS} )
message( NXINCLUDE = $${NXINCLUDE} )
message( MSYS64 = $${MSYS64} )

win32:!system($${MSYS64}sh -c echo >nul 2>&1):error("Unable to launch \"$${MSYS64}sh -c echo\" for test purpose. Probably MSYS64 variable is not correctly set. Check config.pri file.")

REVISION = $$system(git rev-list --count HEAD)
SHA1 = $$system(git rev-list HEAD | \"$${MSYS64}head\" -n 1 | \"$${MSYS64}cut\" -c1-7)

version_content += "$${LITERAL_HASH}define MAJOR $${MAJOR}"
version_content += "$${LITERAL_HASH}define MINOR $${MINOR}"
version_content += "$${LITERAL_HASH}define PATCH $${PATCH}"
version_content += "$${LITERAL_HASH}define REVISION \"$${REVISION}\""
version_content += "$${LITERAL_HASH}define SHA1 \"$${SHA1}\""

!equals(version_content,$$cat($$_PRO_FILE_PWD_/version.h)) {
    write_file($$_PRO_FILE_PWD_/version.h, version_content)
    message("New version generated: $${MAJOR}.$${MINOR}.$${PATCH}.$${REVISION} ($${SHA1})")
}

#Use precompiled headers
CONFIG += precompile_header c++11
PRECOMPILED_HEADER = src/Stable.h
HEADERS += src/Stable.h

#Use shadow builds
CONFIG -= debug_and_release debug_and_release_target

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#win32:DEFINES += NOMINMAX

DEFINES += "SHARED_LIB_EXTENSION=\\\"$${QMAKE_EXTENSION_SHLIB}\\\""

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = bin
OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
PRECOMPILED_DIR = .pch
UI_DIR = .ui

INCLUDEPATH += $$MEMEORY_LEAK_DETECTOR_INCLUDE \
               $$CVS_PARSER_INCLUDE \
               $$LOG4CPP_INCLUDE \
               $$BOOST_INCLUDE \
               $$NXINCLUDE

CONFIG(debug, debug|release) {
    DEFINES += DEBUG _DEBUG

    LIBS += -L$$MEMEORY_LEAK_DETECTOR_LIB \
            -L$$LOG4CPP_LIB_DEBUG \
            -L$$BOOST_LIB_DEBUG \
            -L$$CVS_PARSER_LIB_DEBUG

    win32-msvc* {
        QMAKE_CXXFLAGS += /RTC1 /sdl /fp:except
        #QMAKE_LFLAGS *= /PROFILE
        QMAKE_LFLAGS *= /INCREMENTAL

        #Enable detecting memory leak (comment next line to disable)
        DEFINES += _CRTDBG_MAP_ALLOC
    }
}

CONFIG(release, debug|release) {
    DEFINES += NDEBUG

    LIBS += -L$$LOG4CPP_LIB_RELEASE \
            -L$$BOOST_LIB_RELEASE \
            -L$$CVS_PARSER_LIB_RELEASE

    win32-msvc* {
    }
}

for(translation_name, TRANSLATIONS) {
    qmtarget = $$replace(translation_name, (\\.[^.]*)$, ).qm
    eval($${translation_name}.target = $$system_path($$DESTDIR/$$basename(qmtarget)))
    eval($${qmtarget}.target = $$system_path($$relative_path( \
                                    $$shadowed($$PWD)/$$DESTDIR/startup/$$basename(qmtarget),$$OUT_PWD))))
    eval($${translation_name}.depends = $$system_path($$relative_path($$_PRO_FILE_PWD_/$$translation_name,$$OUT_PWD)))
    eval($${qmtarget}.depends = $$system_path($$relative_path($$DESTDIR/$$basename(qmtarget),$$OUT_PWD)))
    eval($${translation_name}.commands = lrelease \$\$\{$${translation_name}.depends\} \
                                         -qm \$\$\{$${translation_name}.target\})
    eval($${qmtarget}.commands = copy /Y \$\$\{$${qmtarget}.depends\} \$\$\{$${qmtarget}.target\})
    QMAKE_EXTRA_TARGETS += $$translation_name $$qmtarget
    all.depends += $$translation_name $$qmtarget
    eval(QMAKE_DISTCLEAN += \$\$\{$${translation_name}.target\} \$\$\{$${qmtarget}.target\})
}
QMAKE_EXTRA_TARGETS += all

lupdate.target = lupdate
lupdate.commands = lupdate -no-obsolete $$system_path($$_PRO_FILE_)
QMAKE_EXTRA_TARGETS += lupdate
