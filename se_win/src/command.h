#ifndef COMMAND_H
#define COMMAND_H

#include "stdafx.h"
#include "commands.h"
#include "../se_win_i.h"

// Use the following typedef when creating an instance of CCommands.
typedef CComObject<CCommands> CCommandsObj;

// Define a base class for an individual command.
class CCommand :
    public IUnknown,
    public CComObjectRoot,
    public CComCoClass<CCommand, &CLSID_SE_WINCommand>
{
protected:
    CCommands* m_pCommands;

    ISECommandPtr m_pSECommand;
    ISEMouseExPtr m_pSEMouse;

public:
    CCommand();
    virtual ~CCommand();
    LPUNKNOWN GetMyUnknown();

    void SetCommandsObject( CCommands* pCommands );
    CCommands* GetCommandsObject();

    virtual HRESULT CreateCommand( SolidEdgeConstants::seCmdFlag CommandType );
    ISECommandPtr GetCommand();
    virtual void UnadviseFromCommandEvents();
    virtual void ReleaseInterfaces();

    BEGIN_COM_MAP( CCommand )
    COM_INTERFACE_ENTRY( IUnknown )
    END_COM_MAP()
    DECLARE_NOT_AGGREGATABLE( CCommand )

protected:
    template <class IEvents, const IID* piidEvents, const GUID* plibid,
              class XEvents, const CLSID* pClsidEvents>
    class XEventHandler :
        public CComDualImpl<IEvents, piidEvents, plibid>,
        public CComObjectRoot,
        public CComCoClass<XEvents, pClsidEvents>
    {
    public:
        BEGIN_COM_MAP( XEvents )
        COM_INTERFACE_ENTRY_IID( *piidEvents, IEvents )
        END_COM_MAP()
        DECLARE_NOT_AGGREGATABLE( XEvents )
        HRESULT Connect( IUnknown* pUnk )
        {
            HRESULT hr = S_OK;
            VERIFY( SUCCEEDED( hr = AtlAdvise( pUnk, this, *piidEvents, &m_dwAdvise ) ) );
            return hr;
        }
        HRESULT Disconnect( IUnknown* pUnk )
        {
            HRESULT hr = S_OK;
            VERIFY( SUCCEEDED( hr = AtlUnadvise( pUnk, *piidEvents, m_dwAdvise ) ) );
            return hr;
        }

        CCommand* m_pCommand;
    protected:
        DWORD m_dwAdvise;
    };

    class XCommandEvents :
        public XEventHandler<ISECommandEvents, &__uuidof( ISECommandEvents ),
        &LIBID_se_winLib, XCommandEvents, &CLSID_CommandEvents>
    {
    public:
        STDMETHOD( raw_Activate )( void );
        STDMETHOD( raw_Deactivate )( void );
        STDMETHOD( raw_Terminate )( void );
        STDMETHOD( raw_Idle )( long lCount, VARIANT_BOOL* pbMore );
        STDMETHOD( raw_KeyDown )( short* KeyCode, short Shift );
        STDMETHOD( raw_KeyPress )( short* KeyAscii );
        STDMETHOD( raw_KeyUp )( short* KeyCode, short Shift );
    };
    typedef CComObject<XCommandEvents> XCommandEventsObj;
    XCommandEventsObj* m_pCommandEventsObj;

    class XMouseEvents : public XEventHandler<ISEMouseEvents,
        &__uuidof( ISEMouseEvents ), &LIBID_se_winLib,
        XMouseEvents, &CLSID_MouseEvents>
    {
    public:
        STDMETHOD( raw_MouseDown )( short sButton,
                                    short sShift,
                                    double dX,
                                    double dY,
                                    double dZ,
                                    LPDISPATCH pWindowDispatch,
                                    long lKeyPointType,
                                    LPDISPATCH pGraphicDispatch );

        STDMETHOD( raw_MouseUp )( short sButton,
                                  short sShift,
                                  double dX,
                                  double dY,
                                  double dZ,
                                  LPDISPATCH pWindowDispatch,
                                  long lKeyPointType,
                                  LPDISPATCH pGraphicDispatch );

        STDMETHOD( raw_MouseMove )( short sButton,
                                    short sShift,
                                    double dX,
                                    double dY,
                                    double dZ,
                                    LPDISPATCH pWindowDispatch,
                                    long lKeyPointType,
                                    LPDISPATCH pGraphicDispatch );

        STDMETHOD( raw_MouseClick )( short sButton,
                                     short sShift,
                                     double dX,
                                     double dY,
                                     double dZ,
                                     LPDISPATCH pWindowDispatch,
                                     long lKeyPointType,
                                     LPDISPATCH pGraphicDispatch );

        STDMETHOD( raw_MouseDblClick )( short sButton,
                                        short sShift,
                                        double dX,
                                        double dY,
                                        double dZ,
                                        LPDISPATCH pWindowDispatch,
                                        long lKeyPointType,
                                        LPDISPATCH pGraphicDispatch );

        STDMETHOD( raw_MouseDrag )( short sButton,
                                    short sShift,
                                    double dX,
                                    double dY,
                                    double dZ,
                                    LPDISPATCH pWindowDispatch,
                                    short DragState,
                                    long lKeyPointType,
                                    LPDISPATCH pGraphicDispatch );
    };
    typedef CComObject<XMouseEvents> XMouseEventsObj;
    XMouseEventsObj* m_pMouseEventsObj;

    class XLocateFilterEvents : public XEventHandler<ISELocateFilterEvents,
        &__uuidof( ISELocateFilterEvents ), &LIBID_se_winLib,
        XLocateFilterEvents, &CLSID_LocateFilterEvents>
    {
    public:
        STDMETHOD( raw_Filter )( LPDISPATCH pGraphicDispatch, VARIANT_BOOL* vbValid );

    };
    typedef CComObject<XLocateFilterEvents> XLocateFilterEventsObj;
    XLocateFilterEventsObj* m_pLocateFilterEventsObj;

    class XWindowEvents : public XEventHandler<ISECommandWindowEvents,
        &__uuidof( ISECommandWindowEvents ), &LIBID_se_winLib,
        XWindowEvents, &CLSID_CommandWindowEvents>
    {
    public:
        STDMETHOD( raw_WindowProc )( LPDISPATCH pUnkDoc,
                                     LPDISPATCH pUnkView,
                                     UINT nMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     LRESULT* lResult );
    };
    typedef CComObject<XWindowEvents> XWindowEventsObj;
    XWindowEventsObj* m_pWindowEventsObj;

    STDMETHOD( Activate )();
    STDMETHOD( Deactivate )();
    STDMETHOD( Terminate )();
    STDMETHOD( Idle )( long lCount, VARIANT_BOOL* pbMore );
    STDMETHOD( KeyDown )( short* KeyCode, short Shift );
    STDMETHOD( KeyPress )( short* KeyAscii );
    STDMETHOD( KeyUp )( short* KeyCode, short Shift );

    STDMETHOD( MouseDown )( short sButton,
                            short sShift,
                            double dX,
                            double dY,
                            double dZ,
                            LPDISPATCH pWindowDispatch,
                            long lKeyPointType,
                            LPDISPATCH pGraphicDispatch );
    STDMETHOD( MouseUp )( short sButton,
                          short sShift,
                          double dX,
                          double dY,
                          double dZ,
                          LPDISPATCH pWindowDispatch,
                          long lKeyPointType,
                          LPDISPATCH pGraphicDispatch );
    STDMETHOD( MouseMove )( short sButton,
                            short sShift,
                            double dX,
                            double dY,
                            double dZ,
                            LPDISPATCH pWindowDispatch,
                            long lKeyPointType,
                            LPDISPATCH pGraphicDispatch );
    STDMETHOD( MouseClick )( short sButton,
                             short sShift,
                             double dX,
                             double dY,
                             double dZ,
                             LPDISPATCH pWindowDispatch,
                             long lKeyPointType,
                             LPDISPATCH pGraphicDispatch );
    STDMETHOD( MouseDblClick )( short sButton,
                                short sShift,
                                double dX,
                                double dY,
                                double dZ,
                                LPDISPATCH pWindowDispatch,
                                long lKeyPointType,
                                LPDISPATCH pGraphicDispatch );
    STDMETHOD( MouseDrag )( short sButton,
                            short sShift,
                            double dX,
                            double dY,
                            double dZ,
                            LPDISPATCH pWindowDispatch,
                            short DragState,
                            long lKeyPointType,
                            LPDISPATCH pGraphicDispatch );
    STDMETHOD( WindowProc )( LPDISPATCH pDocDispatch,
                             LPDISPATCH pViewDispatch,
                             UINT nMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             LRESULT* lResult );
    STDMETHOD( Filter )( LPDISPATCH pGraphicDispatch,
                         VARIANT_BOOL* vbValid );

    friend class XCommandEvents;
    friend class XMouseEvents;
    friend class XWindowEvents;
    friend class XLocateFilterEvents;

    typedef CComObject<CCommand> CCommandObj;
    typedef CComAggObject<CCommand> CAggCommandObj;
};

#endif // COMMAND_H
