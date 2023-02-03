#include "stdafx.h"
#include "commands.h"
#include "command.h"
#include "document.h"
#include "resource.h"
#include "util.h"
#include "htmlhelp.h"

#include "SolidEdgeExtensionImpl.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif


// CCommands

ISEAddInExPtr CCommands::m_pSEAddIn = NULL;
ApplicationPtr CCommands::m_pApplication = NULL;

AddInPtr GetAddInPtr()
{
    return CCommands::GetAddIn();
}

ApplicationPtr GetApplicationPtr()
{
    return CCommands::GetApplicationPtr();
}

CCommands::CCommands()
{
    m_pApplication = NULL;
    m_pSEAddIn = NULL;
    m_pApplicationEventsObj = NULL;
    m_pAddInEventsObj = NULL;
    m_pAddInEventsExObj = NULL;
    m_pAddInEdgeBarEventsObj = NULL;
}

CCommands::~CCommands()
{
    m_pApplication = NULL;
    m_pSEAddIn = NULL;
    DestroyAllADDINDocuments();
}

HRESULT CCommands::SetApplicationObject( LPDISPATCH pApplicationDispatch,
                                         BOOL bWithEvents )
{
    ASSERT( pApplicationDispatch );

    HRESULT hr = S_OK;

    m_pApplication = pApplicationDispatch;

    if ( bWithEvents )
    {
        // Create Application event handlers
        XApplicationEventsObj::CreateInstance( &m_pApplicationEventsObj );

        if ( m_pApplicationEventsObj )
        {
            m_pApplicationEventsObj->AddRef();
            hr = m_pApplicationEventsObj->Connect( m_pApplication );
            m_pApplicationEventsObj->m_pCommands = this;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

ApplicationPtr CCommands::GetApplicationPtr()
{
    return m_pApplication;
}

HRESULT CCommands::UnadviseFromEvents()
{
    HRESULT hr = S_OK;

    if ( m_pAddInEventsObj )
    {
        hr = m_pAddInEventsObj->Disconnect( m_pSEAddIn );
        m_pAddInEventsObj->Release();
        m_pAddInEventsObj = NULL;
    }

    if ( m_pAddInEventsExObj )
    {
        hr = m_pAddInEventsExObj->Disconnect( m_pSEAddIn );
        m_pAddInEventsExObj->Release();
        m_pAddInEventsExObj = NULL;
    }

    if ( m_pApplicationEventsObj )
    {
        hr = m_pApplicationEventsObj->Disconnect( m_pApplication );
        m_pApplicationEventsObj->Release();
        m_pApplicationEventsObj = NULL;
    }

    if ( m_pAddInEdgeBarEventsObj )
    {
        hr = m_pAddInEdgeBarEventsObj->Disconnect( m_pSEAddIn );
        m_pAddInEdgeBarEventsObj->Release();
        m_pAddInEdgeBarEventsObj = NULL;
    }

    return hr;
}

HRESULT CCommands::SetAddInObject( AddIn* pSolidEdgeAddIn,
                                   BOOL bWithEvents )
{
    ASSERT( pSolidEdgeAddIn );

    HRESULT hr = S_OK;

    m_pSEAddIn = pSolidEdgeAddIn;

    if ( bWithEvents )
    {
        XAddInEventsExObj::CreateInstance( &m_pAddInEventsExObj );

        if ( m_pAddInEventsExObj )
        {
            m_pAddInEventsExObj->AddRef();
            hr = m_pAddInEventsExObj->Connect( m_pSEAddIn->GetAddInEvents() );
            m_pAddInEventsExObj->m_pCommands = this;
        }
        else
            hr = E_OUTOFMEMORY;

        XAddInEventsObj::CreateInstance( &m_pAddInEventsObj );

        if ( m_pAddInEventsObj )
        {
            m_pAddInEventsObj->AddRef();
            hr = m_pAddInEventsObj->Connect( m_pSEAddIn->GetAddInEvents() );
            m_pAddInEventsObj->m_pCommands = this;
        }
        else
            hr = E_OUTOFMEMORY;

        XAddInEdgeBarEventsObj::CreateInstance( &m_pAddInEdgeBarEventsObj );

        if ( m_pAddInEdgeBarEventsObj )
        {
            m_pAddInEdgeBarEventsObj->AddRef();
            hr = m_pAddInEdgeBarEventsObj->Connect( m_pSEAddIn->GetAddInEdgeBarEvents() );

            if ( SUCCEEDED( hr ) )
                m_pAddInEdgeBarEventsObj->m_pCommands = this;
            else
            {
                m_pAddInEdgeBarEventsObj->Release();
                m_pAddInEdgeBarEventsObj = NULL;
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

ISEAddInPtr CCommands::GetAddIn()
{
    return m_pSEAddIn;
}

HRESULT CCommands::XApplicationEvents::raw_AfterActiveDocumentChange( LPDISPATCH theDocument )
{
    if ( m_pCommands->m_pAddInEdgeBarEventsObj )
    {
        PartDocumentPtr pPartDoc = theDocument;

        if ( pPartDoc )
        {
            ADDINDocumentObj* pMyDoc;

            m_pCommands->CreateADDINDocument( theDocument, TRUE, &pMyDoc );

            if ( pMyDoc )
            {
//        pMyDoc->CreateEdgeBar();
                pMyDoc->Release();
            }
        }
    }

    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterDocumentOpen( LPDISPATCH theDocument )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterDocumentPrint( LPDISPATCH theDocument,
                                                               long hDC,
                                                               double* ModelToDC,
                                                               long* Rect )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterDocumentSave( LPDISPATCH theDocument )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterEnvironmentActivate( LPDISPATCH theEnvironment )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterNewDocumentOpen( LPDISPATCH theDocument )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterNewWindow( LPDISPATCH theWindow )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterWindowActivate( LPDISPATCH theWindow )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeCommandRun( long lCommandID )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_AfterCommandRun( long lCommandID )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeDocumentClose( LPDISPATCH theDocument )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeDocumentPrint( LPDISPATCH theDocument,
                                                                long hDC,
                                                                double* ModelToDC,
                                                                long* Rect )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeEnvironmentDeactivate( LPDISPATCH theEnvironment )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeWindowDeactivate( LPDISPATCH theWindow )
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeQuit()
{
    return S_OK;
}

HRESULT CCommands::XApplicationEvents::raw_BeforeDocumentSave( LPDISPATCH theDocument )
{
    return S_OK;
}

SAFEARRAY* pRefKey1 = 0;

void SetRefKey( SAFEARRAY* pRefKey )
{
    pRefKey1 = pRefKey;
}

static int s_RadioState = 0;
static int s_CheckState = 0;

HRESULT CCommands::XAddInEventsEx::raw_OnCommand( long nCmdID )
{
    if ( m_pCommands->m_pAddInEventsObj )
        return m_pCommands->m_pAddInEventsObj->raw_OnCommand( nCmdID );

    return E_UNEXPECTED;
}

HRESULT CCommands::XAddInEventsEx::raw_OnCommandHelp( long hFrameWnd,
                                                      long uHelpCommand,
                                                      long nCmdID )
{
    if ( m_pCommands->m_pAddInEventsObj )
    {
        return m_pCommands->m_pAddInEventsObj->raw_OnCommandHelp( hFrameWnd,
                                                                  uHelpCommand,
                                                                  nCmdID );
    }

    return E_UNEXPECTED;
}

HRESULT CCommands::XAddInEventsEx::raw_OnCommandUpdateUI( long nCmdID,
                                                          long* lCmdFlags,
                                                          BSTR* MenuItemText,
                                                          long* nIDBitmap )
{
    if ( m_pCommands->m_pAddInEventsObj )
    {
        return m_pCommands->m_pAddInEventsObj->raw_OnCommandUpdateUI( nCmdID,
                                                                      lCmdFlags,
                                                                      MenuItemText,
                                                                      nIDBitmap );
    }

    return E_UNEXPECTED;
}

HRESULT CCommands::XAddInEventsEx::raw_OnCommandOnLineHelp( long uHelpCommand,
                                                            long nCmdID,
                                                            BSTR* HelpURL )
{
    *HelpURL = SysAllocString( L"www.solidedge.com" );
    return S_OK;
}

HRESULT CCommands::XAddInEvents::raw_OnCommand( long nCmdID )
{
    HRESULT hr = S_OK;

    try
    {
        switch ( nCmdID )
        {
            case 0:
            {
                SolidEdgeExtensionImpl& extension = SolidEdgeExtensionImpl::Instance();
                extension.RunEditor();

                break;
            }

            default:
            {
            }
        }
    }
    catch ( _com_error& e )
    {
        hr = e.Error();
    }

    return hr;
}

HRESULT CCommands::XAddInEvents::raw_OnCommandHelp( long hFrameWnd,
                                                    long uHelpCommand,
                                                    long nCmdID )
{
    return S_OK;
}

HRESULT CCommands::XAddInEvents::raw_OnCommandUpdateUI( long nCmdID,
                                                        long* lCmdFlags,
                                                        BSTR* MenuItemText,
                                                        long* nIDBitmap )
{
    return S_OK;
}

HRESULT CCommands::XAddInEdgeBarEvents::raw_AddPage( LPDISPATCH pSEDocumentDispatch )
{
    HRESULT hr = S_OK;
    ADDINDocumentObj* pMyDoc;
    PartDocumentPtr pPartDoc = pSEDocumentDispatch;

    if ( pPartDoc )
    {
        m_pCommands->CreateADDINDocument( pSEDocumentDispatch, TRUE, &pMyDoc );

        if ( NULL != pMyDoc )
        {
//      pMyDoc->CreateEdgeBar();
            pMyDoc->Release();
        }
    }

    return hr;
}

HRESULT CCommands::XAddInEdgeBarEvents::raw_RemovePage( LPDISPATCH pSEDocumentDispatch )
{

    HRESULT hr = S_OK;
    /*ADDINDocumentObj* pDoc = NULL;
    m_pCommands->m_pDocuments.Lookup(pSEDocumentDispatch, pDoc);

    if (pDoc)
    {
    //    pDoc->RemoveEdgeBarPage();
    }*/

    return hr;
}

HRESULT CCommands::XAddInEdgeBarEvents::raw_IsPageDisplayable( IDispatch* pSEDocumentDispatch,
                                                               BSTR EnvironmentCatID,
                                                               VARIANT_BOOL* vbIsPageDisplayable )
{

    HRESULT hr = S_OK;
    ADDINDocumentObj* pDoc = 0x0;

//  m_pCommands->m_pDocuments.Lookup(pSEDocumentDispatch, pDoc);
    if ( m_pCommands->m_pDocuments.constFind( pSEDocumentDispatch ) !=
            m_pCommands->m_pDocuments.constEnd() )
        pDoc = m_pCommands->m_pDocuments.value( pSEDocumentDispatch );

    if ( pDoc )
    {
        QString strEnv( _com_util::ConvertBSTRToString( EnvironmentCatID ) );
        QString strSupportedEnv = _T( "{26618396-09d6-11d1-ba07-080036230602}" ); // part

        QString strLIP = _T( "{0DDABC90-125E-4cfe-9CB7-DC97FB74CCF4}" ); // sketch/layout in part
        strLIP = strLIP.toLower();

        strEnv = strEnv.toLower();
        strSupportedEnv = strSupportedEnv.toLower();

        if ( strEnv != strSupportedEnv ) //&& strEnv != strLIP ) // uncomment if wanted in sketch/layout in part
            *vbIsPageDisplayable = VARIANT_FALSE;
    }

    return hr;
}

HRESULT CCommands::CreateADDINDocument( LPDISPATCH pSEDocumentDispatch,
                                        BOOL bWithEvents,
                                        ADDINDocumentObj** ppADDINDocument )
{
    HRESULT hr = S_OK;
    ADDINDocumentObj* pDoc = NULL;
    BOOL bAlreadyThere = ( m_pDocuments.constFind( pSEDocumentDispatch ) != m_pDocuments.constEnd() );

    if ( bAlreadyThere == FALSE )
    {
        ADDINDocumentObj::CreateInstance( &pDoc );

        if ( pDoc )
        {
            pDoc->AddRef();
            pDoc->SetCommandsObject( this );
            pDoc->SetDocumentObject( pSEDocumentDispatch, bWithEvents );

            m_pDocuments[pSEDocumentDispatch] = pDoc;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
        pDoc = m_pDocuments.value( pSEDocumentDispatch );

    if ( ppADDINDocument )
    {
        *ppADDINDocument = pDoc;
        ( *ppADDINDocument )->AddRef();
    }

    return hr;
}

HRESULT CCommands::DestroyADDINDocument( LPDISPATCH pSEDocumentDispatch )
{
    HRESULT hr = S_OK;
    ADDINDocumentObj* pDoc = 0x0;

    BOOL bDocExists = ( m_pDocuments.constFind( pSEDocumentDispatch ) != m_pDocuments.constEnd() );

    if ( bDocExists )
    {
        m_pDocuments.remove( pSEDocumentDispatch );
        C_RELEASE( pDoc );
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

HRESULT CCommands::DestroyAllADDINDocuments()
{
    HRESULT hr = S_OK;
    CMapSEDocDispatchToMyDoc::Iterator it = m_pDocuments.begin();

    while ( it != m_pDocuments.end() )
    {
        ADDINDocumentObj* pDoc = it.value();
        C_RELEASE( pDoc );
        ++it;
    }

    m_pDocuments.clear();

    return hr;
}

ADDINDocumentObj* CCommands::GetDocument( LPDISPATCH pDocDispatch )
{
    ADDINDocumentObj* pDoc = NULL;

    if ( m_pDocuments.constFind( pDocDispatch ) !=
            m_pDocuments.constEnd() )
        pDoc = m_pDocuments.value( pDocDispatch );

    if ( pDoc )
        pDoc->AddRef();

    return pDoc;
}
