#include "Stable.h"
#include "ExtensionImpl.h"
#include "MainWindow.h"

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

//    if ( !msg )
//        msg = "(null)";

    staticCriticalSection.lock();

    QString s( /*QString::fromLocal8Bit(*/msg/*)*/ );
    s += QLatin1Char( '\n' );
    OutputDebugString( ( wchar_t* )s.utf16() );

    staticCriticalSection.unlock();
}

int main( int argc, char* argv[] )
{
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    ::_CrtSetReportHook( ::MemoryLeakDetector::ReportHook );
    ::atexit( ::MemoryLeakDetector::Report );
#endif

    // Install default debug handler
    qInstallMessageHandler( MessageOutput );

    QApplication* mainApplication = new QApplication( argc, argv );
    MainWindow* mainWindow = new MainWindow();
    mainWindow->setAttribute( Qt::WA_DeleteOnClose );

    BaseExtension& extension = ExtensionImpl::Instance();
    extension.Initialize();
    extension.InitScriptEngine();
    extension.SetTopWindow( mainWindow );

    mainWindow->showMaximized();

    int res = mainApplication->exec();

    delete mainApplication;

    extension.TermQt();

    return res;
}
