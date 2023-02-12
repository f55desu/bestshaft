TEMPLATE = subdirs
SUBDIRS = extension

CONFIG += unigraphics

unigraphics { 
    SUBDIRS += nx_win
    nx_win.depends = extension
}
          
DISTFILES = .astylerc
