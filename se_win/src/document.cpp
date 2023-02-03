#include "stdafx.h"
#include "commands.h"
#include "document.h"
#include "resource.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

ADDINDocument::ADDINDocument()
{
    m_pDocument = NULL;
    m_pCommands = NULL;
    m_pDocumentEventsObj = NULL;
}

ADDINDocument::~ADDINDocument()
{
//  DestroyEdgeBar();
    UnadviseFromEvents();
    C_RELEASE( m_pDocumentEventsObj );
}

// This function AddRefs the Solid Edge document.
HRESULT ADDINDocument::SetDocumentObject( LPDISPATCH pDocumentDispatch, BOOL bAdviseEvents )
{
    ASSERT( pDocumentDispatch );

    HRESULT hr = S_OK;

    m_pDocument = pDocumentDispatch;

    if ( bAdviseEvents )
        hr = AdviseEvents();

    return hr;
}

IDispatchPtr ADDINDocument::GetDocument()
{
    return m_pDocument;
}

HRESULT ADDINDocument::AdviseEvents()
{
    ASSERT( m_pDocument != NULL );

    HRESULT hr = S_OK;

    if ( m_pDocumentEventsObj )
    {
        XDocumentEventsObj::CreateInstance( &m_pDocumentEventsObj );

        if ( m_pDocumentEventsObj )
            m_pDocumentEventsObj->AddRef();
        else
            hr = E_OUTOFMEMORY;
    }

    if ( m_pDocumentEventsObj )
    {
        VARIANT varResult;
        VariantInit( &varResult );
        DISPPARAMS disp = { NULL, NULL, 0, 0 };

        // Get the document events object and connect the event sink.
        hr = GetDocument()->Invoke( 0x47, IID_NULL, LOCALE_USER_DEFAULT,
                                    DISPATCH_PROPERTYGET, &disp, &varResult, NULL,
                                    NULL );

        if ( SUCCEEDED( hr ) )
        {
            LPUNKNOWN pDocumentEventProp = ( LPDISPATCH )( varResult.byref );
            hr = m_pDocumentEventsObj->Connect( pDocumentEventProp );

            C_RELEASE( pDocumentEventProp );
        }

        m_pDocumentEventsObj->m_pDocument = this;
    }

    return hr;
}

void ADDINDocument::SetCommandsObject( CCommands* pCommands )
{
    ASSERT( pCommands );
    m_pCommands = pCommands;
}

CCommands* ADDINDocument::GetCommandsObject()
{
    ASSERT( m_pCommands );
    return m_pCommands;
}

HRESULT ADDINDocument::UnadviseFromEvents()
{
    HRESULT hr = NOERROR;

    if ( NULL != m_pDocumentEventsObj )
    {
        // Get the document events object and disconnect the event sink.
        VARIANT varResult;
        VariantInit( &varResult );
        DISPPARAMS disp = { NULL, NULL, 0, 0 };

        hr = GetDocument()->Invoke( 0x47, IID_NULL, LOCALE_USER_DEFAULT,
                                    DISPATCH_PROPERTYGET, &disp, &varResult, NULL,
                                    NULL );

        if ( SUCCEEDED( hr ) )
        {
            LPUNKNOWN pDocumentEventProp = ( LPDISPATCH )( varResult.byref );
            hr = m_pDocumentEventsObj->Disconnect( pDocumentEventProp );

            C_RELEASE( pDocumentEventProp );
        }
    }

    return hr;
}

HRESULT ADDINDocument::XDocumentEvents::raw_BeforeClose()
{
    // Edge bar page is removed when I destroy the addin "document". Now that I
    // am using the new edge bar page event set, the remove page event may have
    // already removed the page.

    HRESULT hr = S_OK;

    if ( m_pDocument )
        hr = m_pDocument->GetCommandsObject()->DestroyADDINDocument( m_pDocument->GetDocument() );
    else
        hr = E_POINTER;

    return hr;
}

HRESULT ADDINDocument::XDocumentEvents::raw_BeforeSave()
{
    return S_OK;
}

HRESULT ADDINDocument::XDocumentEvents::raw_AfterSave()
{
    return S_OK;
}

HRESULT ADDINDocument::XDocumentEvents::raw_SelectSetChanged( LPDISPATCH pSelectSet )
{
    return S_OK;
}
