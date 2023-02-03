#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "stdafx.h"
#include "commands.h"
#include "command.h"
#include "../se_win_i.h"

class ADDINDocument :
    public CComObjectRoot,
    public CComCoClass<ADDINDocument, &CLSID_ADDINDocument>
{
protected:
    IDispatchPtr  m_pDocument;
    CCommands* m_pCommands; // Don't Release()!

public:
    ADDINDocument();
    ~ADDINDocument();

    HRESULT SetDocumentObject( LPDISPATCH pDocumentDispatch, BOOL bWithEvents = TRUE );
    IDispatchPtr GetDocument();

    HRESULT UnadviseFromEvents();
    HRESULT AdviseEvents();

    void SetCommandsObject( CCommands* pCommands );
    CCommands* GetCommandsObject();

    BEGIN_COM_MAP( ADDINDocument )
    END_COM_MAP()
    DECLARE_NOT_AGGREGATABLE( ADDINDocument )


protected:
    template <class IEvents, const IID* piidEvents, const GUID* plibid,
              class XEvents, const CLSID* pClsidEvents>
    class XEventHandler :
        public CComDualImpl<IEvents, piidEvents, plibid>,
        public CComObjectRoot,
        public CComCoClass<XEvents, pClsidEvents>
    {
    public:
        XEventHandler()
        {
            m_pDocument = NULL;
        }
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

        ADDINDocument* m_pDocument;

    protected:
        DWORD m_dwAdvise;
    };

    class XDocumentEvents : public XEventHandler<ISEDocumentEvents,
        &__uuidof( ISEDocumentEvents ), &LIBID_se_winLib,
        XDocumentEvents, &CLSID_ADDINDocumentEvents>
    {
    public:
        STDMETHOD( raw_BeforeClose )();
        STDMETHOD( raw_BeforeSave )();
        STDMETHOD( raw_AfterSave )();
        STDMETHOD( raw_SelectSetChanged )( LPDISPATCH pSelectSet );
    };
    typedef CComObject<XDocumentEvents> XDocumentEventsObj;
    XDocumentEventsObj* m_pDocumentEventsObj;
public:
    friend class XDocumentEvents;
};

#endif // DOCUMENT_H
