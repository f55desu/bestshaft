#include "stdafx.h"
#include "command.h"

CCommand::CCommand()
{
    m_pCommands = NULL;

    m_pCommandEventsObj = NULL;
    m_pMouseEventsObj = NULL;
    m_pWindowEventsObj = NULL;
    m_pLocateFilterEventsObj = NULL;
}

CCommand::~CCommand()
{
    UnadviseFromCommandEvents();
    ReleaseInterfaces();
    C_RELEASE( m_pCommandEventsObj );
    C_RELEASE( m_pMouseEventsObj );
    C_RELEASE( m_pWindowEventsObj );
    C_RELEASE( m_pLocateFilterEventsObj );
}

LPUNKNOWN CCommand::GetMyUnknown()
{
    return GetUnknown();
}

void CCommand::SetCommandsObject( CCommands* pCommands )
{
    ASSERT( pCommands );
    m_pCommands = pCommands;
}

CCommands* CCommand::GetCommandsObject()
{
    ASSERT( m_pCommands );
    return m_pCommands;
}

HRESULT CCommand::CreateCommand( SolidEdgeConstants::seCmdFlag CommandType )
{
    HRESULT hr = S_OK;

    m_pSECommand = m_pCommands->GetApplicationPtr()->CreateCommand( CommandType );

    XCommandEventsObj::CreateInstance( &m_pCommandEventsObj );

    if ( m_pCommandEventsObj )
    {
        m_pCommandEventsObj->AddRef();
        hr = m_pCommandEventsObj->Connect( m_pSECommand );
        m_pCommandEventsObj->m_pCommand = this;
    }
    else
        hr = E_OUTOFMEMORY;

    if ( SUCCEEDED( hr ) )
    {
        if ( CommandType == SolidEdgeConstants::seNoDeactivate )
        {
            m_pSEMouse = m_pSECommand->GetMouse();

            XMouseEventsObj::CreateInstance( &m_pMouseEventsObj );

            if ( m_pMouseEventsObj )
            {
                m_pMouseEventsObj->AddRef();
                hr = m_pMouseEventsObj->Connect( m_pSEMouse );
                m_pMouseEventsObj->m_pCommand = this;
            }
            else
                hr = E_OUTOFMEMORY;

            if ( SUCCEEDED( hr ) )
            {
                XWindowEventsObj::CreateInstance( &m_pWindowEventsObj );

                if ( m_pWindowEventsObj )
                {
                    m_pWindowEventsObj->AddRef();
                    hr = m_pWindowEventsObj->Connect( m_pSECommand->GetWindow() );
                    m_pWindowEventsObj->m_pCommand = this;
                }
                else
                    hr = E_OUTOFMEMORY;
            }

            XLocateFilterEventsObj::CreateInstance( &m_pLocateFilterEventsObj );

            if ( m_pLocateFilterEventsObj )
            {
                m_pLocateFilterEventsObj->AddRef();
                hr = m_pLocateFilterEventsObj->Connect( m_pSEMouse );
                m_pLocateFilterEventsObj->m_pCommand = this;
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

ISECommandPtr CCommand::GetCommand()
{
    return m_pSECommand;
}

void CCommand::UnadviseFromCommandEvents()
{
    if ( m_pMouseEventsObj && m_pSEMouse )
        m_pMouseEventsObj->Disconnect( m_pSEMouse );

    if ( m_pWindowEventsObj && m_pSECommand )
    {
        IUnknownPtr pWindow = m_pSECommand->GetWindow();

        if ( pWindow )
            m_pWindowEventsObj->Disconnect( pWindow );
    }

    if ( m_pLocateFilterEventsObj && m_pSEMouse )
        m_pLocateFilterEventsObj->Disconnect( m_pSEMouse );

    if ( m_pCommandEventsObj && m_pSECommand )
        m_pCommandEventsObj->Disconnect( m_pSECommand );
}

void CCommand::ReleaseInterfaces()
{
    m_pSECommand = NULL;
    m_pSEMouse = NULL;
}

HRESULT CCommand::XCommandEvents::raw_Activate()
{
    HRESULT hr = S_OK;
    hr = m_pCommand->Activate();

    return hr;
}

HRESULT CCommand::XCommandEvents::raw_Deactivate()
{
    HRESULT hr = S_OK;
    hr = m_pCommand->Deactivate();

    return hr;
}

HRESULT CCommand::XCommandEvents::raw_Terminate()
{
    HRESULT hr = S_OK;

    m_pCommand->UnadviseFromCommandEvents();
    hr = m_pCommand->Terminate();
    m_pCommand->ReleaseInterfaces();
    LPUNKNOWN pMyUnknown = NULL;

    pMyUnknown = m_pCommand->GetMyUnknown();

    if ( pMyUnknown )
        pMyUnknown->Release();

    return hr;
}

HRESULT CCommand::XCommandEvents::raw_Idle( long lCount, VARIANT_BOOL* pbMore )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->Idle( lCount, pbMore );

    return hr;
}
HRESULT CCommand::XCommandEvents::raw_KeyDown( short* KeyCode, short Shift )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->KeyDown( KeyCode, Shift );

    return hr;
}
HRESULT CCommand::XCommandEvents::raw_KeyPress( short* KeyAscii )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->KeyPress( KeyAscii );

    return hr;
}
HRESULT CCommand::XCommandEvents::raw_KeyUp( short* KeyCode, short Shift )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->KeyUp( KeyCode, Shift );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseDown( short sButton,
                                               short sShift,
                                               double dX,
                                               double dY,
                                               double dZ,
                                               LPDISPATCH pWindowDispatch,
                                               long lKeyPointType,
                                               LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseDown( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                                lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseUp( short sButton,
                                             short sShift,
                                             double dX,
                                             double dY,
                                             double dZ,
                                             LPDISPATCH pWindowDispatch,
                                             long lKeyPointType,
                                             LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseUp( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                              lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseMove( short sButton,
                                               short sShift,
                                               double dX,
                                               double dY,
                                               double dZ,
                                               LPDISPATCH pWindowDispatch,
                                               long lKeyPointType,
                                               LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseMove( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                                lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseClick( short sButton,
                                                short sShift,
                                                double dX,
                                                double dY,
                                                double dZ,
                                                LPDISPATCH pWindowDispatch,
                                                long lKeyPointType,
                                                LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseClick( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                                 lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseDblClick( short sButton,
                                                   short sShift,
                                                   double dX,
                                                   double dY,
                                                   double dZ,
                                                   LPDISPATCH pWindowDispatch,
                                                   long lKeyPointType,
                                                   LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseDblClick( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                                    lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XMouseEvents::raw_MouseDrag( short sButton,
                                               short sShift,
                                               double dX,
                                               double dY,
                                               double dZ,
                                               LPDISPATCH pWindowDispatch,
                                               short DragState,
                                               long lKeyPointType,
                                               LPDISPATCH pGraphicDispatch )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->MouseDrag( sButton, sShift, dX, dY, dZ, pWindowDispatch,
                                DragState, lKeyPointType, pGraphicDispatch );

    return hr;
}

HRESULT CCommand::XWindowEvents::raw_WindowProc( LPDISPATCH pUnkDoc,
                                                 LPDISPATCH pUnkView,
                                                 UINT nMsg,
                                                 WPARAM wParam,
                                                 LPARAM lParam,
                                                 LRESULT* lResult )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->WindowProc( pUnkDoc, pUnkView, nMsg, wParam, lParam, lResult );

    return hr;
}

HRESULT CCommand::XLocateFilterEvents::raw_Filter( LPDISPATCH pGraphicDispatch,
                                                   VARIANT_BOOL* vbValid )
{
    HRESULT hr = S_OK;
    hr = m_pCommand->Filter( pGraphicDispatch, vbValid );

    return hr;
}

HRESULT CCommand::Activate()
{
    return S_OK;
}

HRESULT CCommand::Deactivate()
{
    return S_OK;
}

HRESULT CCommand::Terminate()
{
    return S_OK;
}

HRESULT CCommand::Idle( long lCount, VARIANT_BOOL* pbMore )
{
    *pbMore = VARIANT_FALSE;
    return S_OK;
}

HRESULT CCommand::KeyDown( short* KeyCode, short Shift )
{
    return S_OK;
}

HRESULT CCommand::KeyPress( short* KeyAscii )
{
    return S_OK;
}

HRESULT CCommand::KeyUp( short* KeyCode, short Shift )
{
    return S_OK;
}

HRESULT CCommand::MouseDown( short sButton,
                             short sShift,
                             double dX,
                             double dY,
                             double dZ,
                             LPDISPATCH pWindowDispatch,
                             long lKeyPointType,
                             LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::MouseUp( short sButton,
                           short sShift,
                           double dX,
                           double dY,
                           double dZ,
                           LPDISPATCH pWindowDispatch,
                           long lKeyPointType,
                           LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::MouseMove( short sButton,
                             short sShift,
                             double dX,
                             double dY,
                             double dZ,
                             LPDISPATCH pWindowDispatch,
                             long lKeyPointType,
                             LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::MouseClick( short sButton,
                              short sShift,
                              double dX,
                              double dY,
                              double dZ,
                              LPDISPATCH pWindowDispatch,
                              long lKeyPointType,
                              LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::MouseDblClick( short sButton,
                                 short sShift,
                                 double dX,
                                 double dY,
                                 double dZ,
                                 LPDISPATCH pWindowDispatch,
                                 long lKeyPointType,
                                 LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::MouseDrag( short sButton,
                             short sShift,
                             double dX,
                             double dY,
                             double dZ,
                             LPDISPATCH pWindowDispatch,
                             short DragState,
                             long lKeyPointType,
                             LPDISPATCH pGraphicDispatch )
{
    return S_OK;
}

HRESULT CCommand::WindowProc( LPDISPATCH pDocDispatch,
                              LPDISPATCH pViewDispatch,
                              UINT nMsg,
                              WPARAM wParam,
                              LPARAM lParam,
                              LRESULT* lResult )
{
    return S_OK;
}

HRESULT CCommand::Filter( LPDISPATCH pGraphicDispatch, VARIANT_BOOL* vbValid )
{
    return S_OK;
}
