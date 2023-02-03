/* Commands.h : header file
 * This class is used to handle generic application events and command events
 * fired specifically to this addin. Trivial implementations of the application
 * events are provided.
*/

#ifndef COMMANDS_H
#define COMMANDS_H

#include "stdafx.h"
#include "../se_win_i.h"

class ADDINDocument;
typedef CComObject<ADDINDocument> ADDINDocumentObj;
typedef QMap<LPDISPATCH, CComObject<ADDINDocument>*> CMapSEDocDispatchToMyDoc;

class CCommands :
    public CComObjectRoot,
    public CComCoClass<CCommands, &CLSID_Commands>
{
protected:
    static ApplicationPtr m_pApplication;
    static ISEAddInExPtr m_pSEAddIn;

    CMapSEDocDispatchToMyDoc m_pDocuments;

public:
    CCommands();
    ~CCommands();

    HRESULT SetApplicationObject( LPDISPATCH pApplicationDispatch, BOOL bWithEvents = TRUE );
    static ApplicationPtr GetApplicationPtr();
    HRESULT UnadviseFromEvents();

    HRESULT SetAddInObject( AddIn* pSolidEdgeAddIn, BOOL bWithEvents = TRUE );
    static ISEAddInPtr GetAddIn();

    HRESULT CreateADDINDocument( LPDISPATCH pSEDocumentDispatch,
                                 BOOL bWithEvents = TRUE,
                                 ADDINDocumentObj** ppADDINDocument = NULL );

    HRESULT DestroyADDINDocument( LPDISPATCH pSEDOcumentDispatch );
    HRESULT DestroyAllADDINDocuments();
    ADDINDocumentObj* GetDocument( LPDISPATCH pSEDocumentDispatch );

    BEGIN_COM_MAP( CCommands )
    END_COM_MAP()
    DECLARE_NOT_AGGREGATABLE( CCommands )

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

        CCommands* m_pCommands;

    protected:
        DWORD m_dwAdvise;
    };

    // This object handles events fired by the AddIn object
    class XAddInEvents : public XEventHandler<ISEAddInEvents,
        &__uuidof( ISEAddInEvents ), &LIBID_se_winLib,
        XAddInEvents, &CLSID_SE_WINAddInEvents>
    {
    public:
        STDMETHOD( raw_OnCommand )( long nCmdID );
        STDMETHOD( raw_OnCommandHelp )( long hFrameWnd, long uHelpCommand, long nCmdID );
        STDMETHOD( raw_OnCommandUpdateUI )( long nCmdID,
                                            long* lCmdFlags,
                                            BSTR* MenuItemText,
                                            long* nIDBitmap );
    };
    typedef CComObject<XAddInEvents> XAddInEventsObj;
    XAddInEventsObj* m_pAddInEventsObj;

    // This object handles events fired by the AddInEx object
    class XAddInEventsEx : public XEventHandler<ISEAddInEventsEx,
        &__uuidof( ISEAddInEventsEx ), &LIBID_se_winLib,
        XAddInEventsEx, &CLSID_SE_WINAddInEventsEx>
    {
    public:
        STDMETHOD( raw_OnCommand )( long nCmdID );
        STDMETHOD( raw_OnCommandHelp )( long hFrameWnd, long uHelpCommand, long nCmdID );
        STDMETHOD( raw_OnCommandUpdateUI )( long nCmdID, long* lCmdFlags, BSTR* MenuItemText, long* nIDBitmap );
        STDMETHOD( raw_OnCommandOnLineHelp )( long uHelpCommand, long nCmdID, BSTR* HelpURL );
    };
    typedef CComObject<XAddInEventsEx> XAddInEventsExObj;
    XAddInEventsExObj* m_pAddInEventsExObj;

    // This object handles events fired by the Application object
    class XApplicationEvents : public XEventHandler<ISEApplicationEvents,
        &__uuidof( ISEApplicationEvents ), &LIBID_se_winLib,
        XApplicationEvents, &CLSID_SE_WINApplicationEvents>
    {
    public:
        STDMETHOD( raw_AfterActiveDocumentChange )( LPDISPATCH theDocument );
        STDMETHOD( raw_AfterCommandRun )( long theCommandID );
        STDMETHOD( raw_AfterDocumentOpen )( LPDISPATCH theDocument );
        STDMETHOD( raw_AfterDocumentPrint )( LPDISPATCH theDocument,
                                             long hDC,
                                             double* ModelToDC,
                                             long* Rect );
        STDMETHOD( raw_AfterDocumentSave )( LPDISPATCH theDocument );
        STDMETHOD( raw_AfterEnvironmentActivate )( LPDISPATCH theEnvironment );
        STDMETHOD( raw_AfterNewDocumentOpen )( LPDISPATCH theDocument );
        STDMETHOD( raw_AfterNewWindow )( LPDISPATCH theWindow );
        STDMETHOD( raw_AfterWindowActivate )( LPDISPATCH theWindow );
        STDMETHOD( raw_BeforeCommandRun )( long theCommandID );
        STDMETHOD( raw_BeforeDocumentClose )( LPDISPATCH theDocument );
        STDMETHOD( raw_BeforeDocumentPrint )( LPDISPATCH theDocument,
                                              long hDC,
                                              double* ModelToDC,
                                              long* Rect );
        STDMETHOD( raw_BeforeEnvironmentDeactivate )( LPDISPATCH theEnvironment );
        STDMETHOD( raw_BeforeWindowDeactivate )( LPDISPATCH theWindow );
        STDMETHOD( raw_BeforeQuit )();
        STDMETHOD( raw_BeforeDocumentSave )( LPDISPATCH theDocument );
    };
    typedef CComObject<XApplicationEvents> XApplicationEventsObj;
    XApplicationEventsObj* m_pApplicationEventsObj;

    // This object handles edge bar events fired by Solid Edge to the addin object.
    class XAddInEdgeBarEvents : public XEventHandler<ISEAddInEdgeBarEvents,
        &__uuidof( ISEAddInEdgeBarEvents ), &LIBID_se_winLib,
        XAddInEdgeBarEvents, &CLSID_SE_WINAddInEdgeBarEvents>
    {
    public:
        STDMETHOD( raw_AddPage )( LPDISPATCH theDocument );
        STDMETHOD( raw_RemovePage )( LPDISPATCH theDocument );
        STDMETHOD( raw_IsPageDisplayable )( IDispatch* theDocument,
                                            BSTR EnvironmentCatID,
                                            VARIANT_BOOL* vbIsPageDisplayable );
    };
    typedef CComObject<XAddInEdgeBarEvents> XAddInEdgeBarEventsObj;
    XAddInEdgeBarEventsObj* m_pAddInEdgeBarEventsObj;

public:

    friend class XAddInEvents;
    friend class XAddInEventsEx;
};

#endif // COMMANDS_H
