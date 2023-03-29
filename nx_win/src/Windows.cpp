#include "Stable.h"
#include "NXExtensionImpl.h"
#include "TopWindow.h"

//Code in this module is specific to Windows

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
    DWORD ProcID;
    ::GetWindowThreadProcessId( hwnd, &ProcID );

    if ( ::GetCurrentProcessId() == ProcID )
    {
        hwnd = ::GetAncestor( hwnd, GA_ROOTOWNER );
        *( ( LONG_PTR* )lParam ) = ( __int64 )hwnd;
        return FALSE;
    }

    return TRUE;
}

void NXExtensionImpl::InitQt()
{
    if ( m_hHook == 0x0 )
        m_hHook = ::SetWindowsHookEx( WH_GETMESSAGE, QtFilterProc, 0, GetCurrentThreadId() );

    BaseExtension::InitQt();
}

void NXExtensionImpl::Terminate()
{
    if ( m_topWindow )
    {
        delete m_topWindow;
        m_topWindow = 0x0;
    }

    //Terminate NX session
    UF_CALL( ::UF_terminate() );

    BaseExtension::TermQt();

    if ( m_hHook )
    {
        ::UnhookWindowsHookEx( m_hHook );
        m_hHook = 0x0;
    }
}

QWidget* NXExtensionImpl::GetTopWindow()
{
    if ( m_topWindow )
        return m_topWindow;

    //Get main window handle (code is specific to Windows)
    HWND hMainWnd = 0x0;
    ::EnumWindows( EnumWindowsProc, ( LPARAM )&hMainWnd );
    Q_ASSERT( hMainWnd );

    //For windows system Qt has HWND mapped to WId
    return m_topWindow = new TopWindow( hMainWnd );
}

LRESULT CALLBACK NXExtensionImpl::QtFilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp )
    {
        // don't process deferred-deletes while in a modal loop
        if ( BaseExtension::GetModalState() )
            qApp->sendPostedEvents();
        else
            qApp->sendPostedEvents( 0, -1 );
    }

    return ::CallNextHookEx( m_hHook, nCode, wParam, lParam );
}

