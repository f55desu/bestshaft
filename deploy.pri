win32{
    system(@if not exist $$system_path($$SHADOWED_PWD/$$DESTDIR) ($$QMAKE_MKDIR $$system_path($$SHADOWED_PWD/$$DESTDIR)))
}
unix{
    system(if [ ! -f $$system_path($$SHADOWED_PWD/$$DESTDIR) ]; then $$QMAKE_MKDIR $$system_path($$SHADOWED_PWD/$$DESTDIR); fi) )
}

target_file = $$SHADOWED_PWD/$$DESTDIR/$${EXE_PREF}$${TARGET}$${EXE_EXT}
deploy_target_name = $$relative_path($$PWD,$$OUT_PWD)
deploy_target_name = $$replace(deploy_target_name, -, _)
eval($${deploy_target_name}.depends = $$DESTDIR/$${EXE_PREF}$${TARGET}$${EXE_EXT})
eval($${deploy_target_name}.target = deploy)
eval($${deploy_target_name}.commands = $$QMAKE_COPY \$\$system_path(\$\$\{$${deploy_target_name}.depends\}) \
    \$\$system_path(\$\$\{target_file\}))

QMAKE_EXTRA_TARGETS += $$deploy_target_name

eval(QMAKE_DISTCLEAN += \$\$\{target_file\})

all.depends += $$deploy_target_name
QMAKE_EXTRA_TARGETS += all
