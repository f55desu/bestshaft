#pragma once
//Enable detecting memory leak functionality
#if defined(WIN32) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    #include <stdlib.h>
    #include <crtdbg.h>
    #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define new DEBUG_NEW
#endif

#define NOMINMAX

//RTL support for native C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#include <stddef.h>
#include <memory.h>
#include <float.h>
#include <intrin.h>
#include <malloc.h>

#if defined(DEBUG) && !defined(SHOW_TIME)
    #define SHOW_TIME
#endif
#ifdef SHOW_TIME
    #include <sys/timeb.h>
#endif

#if defined __cplusplus

    #include <iostream>
    #include <iomanip>
    #include <ctime>
    #include <sstream>

    //STL support
    #include <set>
    #include <map>
    #include <list>
    #include <vector>
    #include <string>
    #include <algorithm>
    #define _USE_MATH_DEFINES
    #include <cmath>
    #include <memory>
    #include <limits>
    #include <ostream>
    #include <fstream>
    #include <iostream>
    #include <limits>
    #include <utility>
    #include <cassert>
    #include <queue>
    #include <stack>
    #include <exception>
    #include <functional>

    #include <strstream>
    #include <codecvt>
    #include <locale>

    using namespace std;

    #ifdef _MSC_VER
        #include <unordered_set>
        #include <unordered_map>
    #else
        #include <ext/hash_set>
        #include <ext/hash_map>
    #endif

    #include <uf.h>
    #include <uf_mb.h>
    #include <uf_sket.h>
    #include <uf_obj.h>
    #include <uf_modl.h>
    #include <uf_part.h>
    #include <uf_assem.h>
    #include <uf_csys.h>
    #include <uf_mtx.h>
    #include <uf_so.h>
    #include <uf_sc.h>
    #include <uf_eval.h>
    #include <uf_point.h>
    #include <uf_evalsf.h>
    #include <uf_brep.h>
    #include <uf_facet.h>
    #include <uf_ui.h>
    #include <uf_attr.h>

    //Logging
    #include <log4cpp/Category.hh>
    #include <log4cpp/RollingFileAppender.hh>
    #include <log4cpp/Appender.hh>
    #include <log4cpp/OstreamAppender.hh>
    #include <log4cpp/FileAppender.hh>
    #include <log4cpp/Layout.hh>
    #include <log4cpp/BasicLayout.hh>
    #include <log4cpp/Priority.hh>
    #include <log4cpp/NDC.hh>
    #include <log4cpp/PatternLayout.hh>
    #include <log4cpp/PropertyConfigurator.hh>
    using namespace log4cpp;

    #include "MemoryLeakDetector.h"

    //Disable memory leak detection in order to compile without errors
    #include "DisableMemoryLeak.h"

    //Qt support
    #include <QtGui>
    #include <QApplication>
    #include <QMessageBox>
    #include <QWidget>
    //    #include <QDebug>
    #include <QSettings>
    #include <QDir>
    #include <QProcess>
    #include <QProcessEnvironment>
    //    #include <QStandardPaths>
    #include <QCryptographicHash>

    //    #include <QJSEngine>

    #include "EnableMemoryLeak.h"

    #include <windows.h>

    //    // tetgen
    //    #include <tetgen.h>

    //    // CGAL lib
    //    #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
    //    #include <CGAL/Triangulation_3.h>
    //    #include <CGAL/Tetrahedral_mesh_3.h>

#endif
