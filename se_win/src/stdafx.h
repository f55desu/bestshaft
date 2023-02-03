// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

//#ifndef STDAFX_H
//#define STDAFX_H

//#if _MSC_VER >= 1000
#pragma once
//#endif // _MSC_VER >= 1000

#define STRICT

/*#ifndef _SECURE_ATL
#define _SECURE_ATL 0
#endif*/

#ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN    // Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER // Allow use of features specific to Windows XP or later.
    #define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT // Allow use of features specific to Windows XP or later.
    #define _WIN32_WINNT 0x0501
#endif

//#ifndef _WIN32_WINDOWS // Allow use of features specific to Windows 98 or later.
//#define _WIN32_WINDOWS 0x0410
//#endif

#ifndef _WIN32_IE // Allow use of features specific to IE 6.0 or later.
    #define _WIN32_IE 0x0600
#endif

#include <ObjBase.h>
#define ASSERT(f)

#define _ATL_APARTMENT_THREADED

#define AFX_CRT_ERRORCHECK(expr) \
    AtlCrtErrorCheck(expr)

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include <winnt.h>
#include <initguid.h>
#include <comcat.h>

#define _ATL_STATIC_REGISTRY

#pragma warning (disable : 4786 )

#import "constant.tlb"
#import "framewrk.tlb" exclude ("UINT_PTR", "LONG_PTR") rename ("GetOpenFileName", "SEGetOpenFileName")
#import "fwksupp.tlb"

#import "geometry.tlb"
#import "part.tlb"
#import "assembly.tlb"
#import "draft.tlb"

#ifdef _DEBUG
    // generated from tlb header files
    #include "../../tmp/debug/assembly.tlh"
    #include "../../tmp/debug/constant.tlh"
    #include "../../tmp/debug/framewrk.tlh"
    #include "../../tmp/debug/fwksupp.tlh"
    #include "../../tmp/debug/part.tlh"
    #include "../../tmp/debug/geometry.tlh"
#endif // _DEBUG

using namespace SolidEdgeFramework;
using namespace SolidEdgeAssembly;
using namespace SolidEdgeFrameworkSupport;
using namespace SolidEdgeGeometry;
using namespace SolidEdgePart;

#define C_RELEASE(x) if (x) { x->Release(); x = NULL; }

AddInPtr GetAddInPtr();
ApplicationPtr GetApplicationPtr();

HINSTANCE hMyInstance();

STDAPI RegisterSEAddIn( const CLSID& AddInCLSID );
STDAPI UnRegisterSEAddIn( const CLSID& AddInCLSID );

#ifdef _DEBUG

void GetLastErrorDescription( CComBSTR& bstr );
#define VERIFY_OK(f) \
  { \
    HRESULT hr = (f); \
    if (hr != S_OK) \
    { \
      if (FAILED(hr)) \
      { \
        CComBSTR bstr; \
        GetLastErrorDescription(bstr); \
        _RPTF2(_CRT_ASSERT, "Object call returned %lx\n\n%S", hr, (BSTR) bstr); \
      } \
      else \
        _RPTF1(_CRT_ASSERT, "Object call returned %lx", hr); \
    } \
  }

#else //_DEBUG
#define VERIFY_OK(f) (f);
#endif //_DEBUG

//Enable detecting memory leak functionality
#if defined(WIN32) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    #include <stdlib.h>
    #include <crtdbg.h>
    #ifdef DEBUG_NEW
        #undef DEBUG_NEW
    #endif // DEBUG_NEW
    #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define new DEBUG_NEW
#endif

#define VERIFY(x) x

#include <cmath>
#include <float.h>
#include <limits.h>
#include <algorithm>
using namespace std;

// Solid Edge headers
//#include <addins_i.c>
#include <secatids.h>

#include "MemoryLeakDetector.h"

//Disable memory leak detection in order to compile without errors
#include "DisableMemoryLeak.h"

//Qt support
#include <QtGui>
#include <QObject>
#include <QWidget>
#include <QtScript>
#include <QString>
#include <QDir>
#include <QSettings>
#include <QVector3D>
#include <QTime>

#include "EnableMemoryLeak.h"

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

#define DUMP_EDGE_SEGMENTS
