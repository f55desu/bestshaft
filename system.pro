TEMPLATE = subdirs
SUBDIRS = extension

!unigraphics : !opencascade : !solidedge : CONFIG += unigraphics

unigraphics { 
    SUBDIRS += nx_win
    nx_win.depends = extension
}
          
opencascade { 
    SUBDIRS += MainApplication
    MainApplication.depends = extension
}

solidedge {
    SUBDIRS += se_win
    se_win.depends = extension
}

DISTFILES = .astylerc
