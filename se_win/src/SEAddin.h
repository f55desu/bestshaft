// SEAddin.h : Declaration of the CSEAddin

#ifndef SEADDIN_H
#define SEADDIN_H

#include "resource.h"
#include "commands.h"
#include "command.h"

using namespace SolidEdgeFramework;

#define COM_TYPELIBINTERFACE_ENTRY(x)\
{&__uuidof(x), \
  offsetofclass(x, _ComMapClass), \
  _ATL_SIMPLEMAPENTRY},

DEFINE_GUID( CLSID_SEAddIn,
             0x6D4144EA, 0x2FC2, 0x11D3, 0x92, 0x76, 0x00, 0xC0, 0x4F, 0x79, 0xBE, 0x98 );


class CSEAddIn :
    public ISolidEdgeAddIn,
    public CComObjectRoot,
    public ISupportErrorInfo,
    public CComCoClass<CSEAddIn, &CLSID_SEAddIn>
{
public:
    CSEAddIn();
    ~CSEAddIn();

    DECLARE_REGISTRY_RESOURCEID( IDR_SE_WIN )

    BEGIN_COM_MAP( CSEAddIn )
    COM_TYPELIBINTERFACE_ENTRY( ISolidEdgeAddIn )
    COM_INTERFACE_ENTRY( ISupportErrorInfo )
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP( CSEAddIn )
    END_CONNECTION_POINT_MAP()


    // ISupportsErrorInfo
    STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

    // ISolidEdgeAddin
    STDMETHOD( raw_OnConnection )( THIS_ IDispatch* pAppDispatch,
                                   SeConnectMode ConnectMode,
                                   SolidEdgeFramework::AddIn* pUnkAddIn );
    STDMETHOD( raw_OnConnectToEnvironment )( BSTR EnvironmentCatid,
                                             LPDISPATCH pEnvironment,
                                             VARIANT_BOOL bFirstTime );
    STDMETHOD( raw_OnDisconnection )( THIS_ SeDisconnectMode DisconnectMode );

public:

protected:
    CCommandsObj* m_pCommands;
};

#endif //SEADDIN_H
