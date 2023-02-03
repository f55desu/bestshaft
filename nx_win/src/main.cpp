#include "Stable.h"
#include "NXExtensionImpl.h"

class QWinMsgHandlerCriticalSection
{
    CRITICAL_SECTION cs;
public:
    QWinMsgHandlerCriticalSection()
    {
        ::InitializeCriticalSection( &cs );
    }
    ~QWinMsgHandlerCriticalSection()
    {
        ::DeleteCriticalSection( &cs );
    }

    void lock()
    {
        ::EnterCriticalSection( &cs );
    }
    void unlock()
    {
        ::LeaveCriticalSection( &cs );
    }
};

void MessageOutput( QtMsgType, const QMessageLogContext&, const QString& msg )
{
    // OutputDebugString is not threadsafe.

    // cannot use QMutex here, because qWarning()s in the QMutex
    // implementation may cause this function to recurse
    static QWinMsgHandlerCriticalSection staticCriticalSection;

    /*if ( !msg )
        msg = "(null)";*/

    staticCriticalSection.lock();

    QString s( msg );
    s += QLatin1Char( '\n' );
    OutputDebugString( ( wchar_t* )s.utf16() );

    staticCriticalSection.unlock();
}

//------------------------------------------------------------------------------
// Startup entrypoint for NX
//------------------------------------------------------------------------------
extern "C" __declspec( dllexport ) void ufsta( char* /*param*/, int* /*retcod*/, int /*param_len*/ )
{
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    ::_CrtSetReportHook( ::MemoryLeakDetector::ReportHook );
    ::atexit( ::MemoryLeakDetector::Report );
#endif
    // Install default debug handler
    qInstallMessageHandler( MessageOutput );
    BaseExtension& extension = NXExtensionImpl::Instance();
    extension.Initialize();
}

//------------------------------------------------------------------------------
// Public method GetUnloadOption
// This method specifies how a shared image is unloaded from memory
// within NX.
//------------------------------------------------------------------------------
extern "C" __declspec( dllexport ) int ufusr_ask_unload()
{
    return UF_UNLOAD_UG_TERMINATE;
}

//------------------------------------------------------------------------------
// Method: UnloadLibrary()
// You have the option of coding the cleanup routine to perform any housekeeping
// chores that may need to be performed. If you code the cleanup routine, it is
// automatically called by NX.
//------------------------------------------------------------------------------
extern "C" __declspec( dllexport ) void ufusr_cleanup( void )
{
    // do your cleanup here if necessary
    NXExtensionImpl::Instance().Terminate();
    return;
}
