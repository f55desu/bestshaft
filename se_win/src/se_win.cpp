// se_win.cpp : Implementation of DLL Exports.

#include "stdafx.h"

#include "resource.h"
#include "../se_win_i.h"
#include "SEAddin.h"

CComModule _Module;

BEGIN_OBJECT_MAP( ObjectMap )
OBJECT_ENTRY( CLSID_SEAddIn, CSEAddIn )
END_OBJECT_MAP()

class CSE_WINApp
{
public:

    static void createInstance();
    static CSE_WINApp* getInstance();
    static void freeInstance();

    BOOL InitInstance();
    int ExitInstance();

    HINSTANCE m_hInstance;
private:
    CSE_WINApp();
    ~CSE_WINApp();
    static CSE_WINApp* m_appInstance;

};

CSE_WINApp* CSE_WINApp::m_appInstance = 0x0;

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID )
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        CSE_WINApp::getInstance()->m_hInstance = hInstance;
        CSE_WINApp::getInstance()->InitInstance();
    }

    return TRUE;
}

void CSE_WINApp::createInstance()
{
    m_appInstance = new CSE_WINApp();
}

CSE_WINApp* CSE_WINApp::getInstance()
{
    if ( !m_appInstance )
        createInstance();

    return m_appInstance;
}

BOOL CSE_WINApp::InitInstance()
{
    HRESULT hr = _Module.Init( ObjectMap, m_hInstance );

    if ( SUCCEEDED( hr ) )
        return TRUE;

    return FALSE;
}

STDAPI DllUnregisterServer();

int CSE_WINApp::ExitInstance()
{
    _Module.Term();
    return S_OK;
}

CSE_WINApp::CSE_WINApp()
{
    m_hInstance = 0x0;
    m_appInstance = 0x0;
}

CSE_WINApp::~CSE_WINApp()
{
    delete m_hInstance;
    m_hInstance = 0x0;

    delete m_appInstance;
    m_appInstance = 0x0;
}

STDAPI DllCanUnloadNow()
{
    return ( _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
#pragma warning(suppress : 6387)
{
    return _Module.GetClassObject( rclsid, riid, ppv );
}

STDAPI DllRegisterServer()
{
    return RegisterSEAddIn( CLSID_SEAddIn );
}

STDAPI DllUnregisterServer()
{
    return UnRegisterSEAddIn( CLSID_SEAddIn );
}

#ifdef _DEBUG
void GetLastErrorDescription( CComBSTR& bstr )
{
    CComPtr<IErrorInfo> pErrorInfo;

    if ( GetErrorInfo( 0, &pErrorInfo ) == S_OK )
        pErrorInfo->GetDescription( &bstr );
}
#endif //_DEBUG
