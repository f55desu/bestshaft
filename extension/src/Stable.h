#pragma once
//Enable detecting memory leak functionality
#if defined(WIN32) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    #include <stdlib.h>
    #include <crtdbg.h>
    #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define new DEBUG_NEW
#endif

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

#ifdef LINUX_GCC
    #include <mmintrin.h>
    #include <emmintrin.h>
#else
    #include <intrin.h>
#endif

#if defined(DEBUG) && !defined(SHOW_TIME)
    #define SHOW_TIME
#endif
#ifdef SHOW_TIME
    #include <sys/timeb.h>
#endif

#if defined __cplusplus

    //STL support
    #include <set>
    #include <map>
    #include <list>
    #include <vector>
    #include <string>
    #include <algorithm>
    //#define _USE_MATH_DEFINES
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
    using namespace std;

    #ifdef _MSC_VER
        #include <unordered_set>
        #include <unordered_map>
    #else
        #include <ext/hash_set>
        #include <ext/hash_map>
    #endif

    //Logging
    #define throw(...)
    #include <log4cpp/Category.hh>
    #undef throw

    #include <log4cpp/RollingFileAppender.hh>
    #include <log4cpp/Appender.hh>
    #include <log4cpp/OstreamAppender.hh>
    #include <log4cpp/FileAppender.hh>
    #include <log4cpp/Layout.hh>
    #include <log4cpp/BasicLayout.hh>
    #include <log4cpp/Priority.hh>
    #include <log4cpp/NDC.hh>
    #define throw(...)
    #include <log4cpp/PatternLayout.hh>
    #include <log4cpp/PropertyConfigurator.hh>
    #undef throw
    using namespace log4cpp;

    //CSV support
    #include <csvparser.hpp>

    //Disable memory leak detection in order to compile without errors
    #include "DisableMemoryLeak.h"

    //Qt support
    #include <QtGui>
    #include <QLabel>
    #include <QMessageBox>
    #include <QFileDialog>
    //    #include <QtXml>
    //    #include <QJSEngine>
    //#include <QTextCodec>

    #include "EnableMemoryLeak.h"

#endif
