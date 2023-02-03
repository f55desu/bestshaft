// SEAddin.cpp : Implementation of ISEAddin
#include "stdafx.h"
#include "SEAddin.h"
#include "util.h"
#include "SolidEdgeExtensionImpl.h"

CSEAddIn::CSEAddIn()
{
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    ::_CrtSetReportHook( ::MemoryLeakDetector::ReportHook );
    ::atexit( ::MemoryLeakDetector::Report );
#endif

    m_pCommands = NULL;
}

CSEAddIn::~CSEAddIn()
{
    delete m_pCommands;
    m_pCommands = NULL;
}

STDMETHODIMP CSEAddIn::raw_OnConnection( IDispatch* pAppDispatch,
                                         SeConnectMode,
                                         AddIn* pAddIn )
{
    HRESULT hr = S_OK;
    const int GUI_VERSION = 7;

    CCommandsObj::CreateInstance( &m_pCommands );
    m_pCommands->AddRef();

    hr = m_pCommands->SetApplicationObject( pAppDispatch );

    if ( SUCCEEDED( hr ) )
    {
        hr = m_pCommands->SetAddInObject( pAddIn );

        if ( SUCCEEDED( hr ) )
            m_pCommands->GetAddIn()->put_GuiVersion( GUI_VERSION );
    }

    SolidEdgeExtensionImpl& extension = SolidEdgeExtensionImpl::Instance();
    extension.Initialize();

    return hr;
}

void func()
{
    return;
}

STDMETHODIMP CSEAddIn::raw_OnConnectToEnvironment( BSTR EnvironmentCatid,
                                                   LPDISPATCH,
                                                   VARIANT_BOOL )
{
    HRESULT hr = S_OK;

//  if (m_pCommands->GetApplicationPtr()->Documents->Count == 0 ||
//      m_pCommands->GetApplicationPtr()->ActiveDocumentType != igPartDocument)
//    return hr;

    const int NUMBER_OF_COMMANDS = 1;
    const QString COMMAND_BAR_NAME = "Paratran";
    const QString COMMAND_NAME = "Editor";

    _bstr_t bszCmdStringsCat( qPrintable( "\n " + COMMAND_NAME ) );

    SAFEARRAYBOUND saCmdStringsBoundCat;
    saCmdStringsBoundCat.lLbound = 0;
    saCmdStringsBoundCat.cElements = NUMBER_OF_COMMANDS;
    SAFEARRAY* commandNamesArray = SafeArrayCreate( VT_BSTR, 1, &saCmdStringsBoundCat );

    SAFEARRAYBOUND saCmdIDsBoundCat;
    saCmdIDsBoundCat.lLbound = 0;
    saCmdIDsBoundCat.cElements = NUMBER_OF_COMMANDS;
    SAFEARRAY* commandsIDsArray = SafeArrayCreate( VT_I4, 1, &saCmdIDsBoundCat );

    if ( commandNamesArray && commandsIDsArray )
    {
        long index = 0;
        SafeArrayPutElement( commandNamesArray, &index, ( wchar_t* )bszCmdStringsCat );
        SafeArrayPutElement( commandsIDsArray, &index, &index );

        ISEAddInExPtr pAddInEx = m_pCommands->GetAddIn();

        if ( pAddInEx )
        {
            TCHAR ResourceFilename[MAX_PATH];
            HMODULE hModule = 0x0;
            ::GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                 ( LPCSTR ) & ( func ), &hModule );
            ::GetModuleFileName( hModule, ResourceFilename, sizeof( ResourceFilename ) );
            VERIFY_OK( pAddInEx->SetAddInInfoEx( ResourceFilename,
                                                 EnvironmentCatid,
                                                 qPrintable( COMMAND_BAR_NAME ),
                                                 IDR_TOOLBAR_MEDIUM,
                                                 IDR_TOOLBAR_LARGE,
                                                 IDR_TOOLBAR_MEDIUMMONOCHROME,
                                                 IDR_TOOLBAR_LARGEMONOCHROME,
                                                 NUMBER_OF_COMMANDS,
                                                 &commandNamesArray,
                                                 &commandsIDsArray ) );
        }

        CommandBarButtonPtr pButton = pAddInEx->AddCommandBarButton( EnvironmentCatid,
                                                                     qPrintable( COMMAND_BAR_NAME ), 0 );

        if ( pButton )
            pButton->Style = seButtonIconAndCaptionBelow;

        SafeArrayDestroy( commandNamesArray );
        SafeArrayDestroy( commandsIDsArray );
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

STDMETHODIMP CSEAddIn::raw_OnDisconnection( SeDisconnectMode DisconnectMode )
{
    HRESULT hr = S_OK;

    if ( m_pCommands )
    {
        m_pCommands->UnadviseFromEvents();
        m_pCommands->Release();
        m_pCommands = NULL;
    }

    // TODO: Perform any cleanup work here

    SolidEdgeExtensionImpl& extension = SolidEdgeExtensionImpl::Instance();
    extension.Terminate();

    return hr;
}

STDMETHODIMP CSEAddIn::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID* arr[] =
    {
        &__uuidof( ISEAddIn ),
    };

    for ( int i = 0; i < sizeof( arr ) / sizeof( arr[0] ); i++ )
    {
        if ( InlineIsEqualGUID( *arr[i], riid ) )
            return S_OK;
    }

    return S_FALSE;
}
