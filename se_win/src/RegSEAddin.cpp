#include "stdafx.h"

#define TYPELIB_UID "{81512E3A-54CE-47C2-B857-0389A518CC37}"

STDAPI RegisterSEAddIn( const CLSID& AddInCLSID )
{
    QString appVersion = "se_win.Solid Edge Addin.1";
    QString appVersionIndependentProgID = "se_win.Solid Edge Addin";
    QString addInClassName = "Solid Edge Addin Class";

    QSettings regOrganizer( "HKEY_CURRENT_USER\\SOFTWARE", QSettings::NativeFormat );

    regOrganizer.beginGroup( "Classes" );

    regOrganizer.beginGroup( "CLSID" );
    HRESULT hr = S_OK;
    LPOLESTR lpClsidString;
    hr = StringFromCLSID( AddInCLSID, &lpClsidString );

    QString clsidStr = QString::fromWCharArray( ( wchar_t* )lpClsidString );
    regOrganizer.beginGroup( clsidStr );
    regOrganizer.setValue( ".", addInClassName );
    regOrganizer.setValue( "409", "Paratran" );
    regOrganizer.setValue( "AutoConnect", 1 );

    regOrganizer.beginGroup( "Environment Categories" );
    regOrganizer.beginGroup( "{C484ED57-DBB6-4A83-BEDB-C08600AF07BF}" );
    regOrganizer.setValue( ".", "" );
    regOrganizer.endGroup();
    regOrganizer.beginGroup( "{26618396-09D6-11d1-BA07-080036230602}" );
    regOrganizer.setValue( ".", "" );
    regOrganizer.endGroup();
    regOrganizer.beginGroup( "{D9B0BB85-3A6C-4086-A0BB-88A1AAD57A58}" );
    regOrganizer.setValue( ".", "" );
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "Implemented Categories" );
    regOrganizer.beginGroup( "{26B1D2D1-2B03-11D2-B589-080036E8B802}" );
    regOrganizer.setValue( ".", "" );
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "InprocServer32" );
    TCHAR dllPath[MAX_PATH];
    HMODULE hm = NULL;
    ::GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         ( LPCSTR ) &RegisterSEAddIn, &hm );
    ::GetModuleFileName( hm, dllPath, MAX_PATH );
    regOrganizer.setValue( ".", QString( dllPath ) );
    regOrganizer.setValue( "ThreadingModel", "Apartment" );
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "ProgID" );
    regOrganizer.setValue( ".", appVersion );
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "Programmable" );
    regOrganizer.setValue( ".", "" );
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "Summary" );
    regOrganizer.setValue( "409", "INOBITEC Solid Edge AddIn" );
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "VersionIndependentProgID" );
    regOrganizer.setValue( ".", appVersionIndependentProgID );
    regOrganizer.endGroup();

    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "se_win.SEAddin.1" );
    regOrganizer.setValue( ".", addInClassName );
    regOrganizer.beginGroup( "CLSID" );
    regOrganizer.setValue( ".", clsidStr );
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "se_win.SEAddin" );
    regOrganizer.setValue( ".", addInClassName );
    regOrganizer.beginGroup( "CurVer" );
    regOrganizer.setValue( ".", appVersion );
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "TypeLib" );
    regOrganizer.beginGroup( TYPELIB_UID );
    regOrganizer.beginGroup( "1.0" );
    regOrganizer.setValue( ".", "se_win 1.0 Type Library" );
    regOrganizer.beginGroup( "0" );
    regOrganizer.beginGroup( "win64" );
    regOrganizer.setValue( ".", QString( dllPath ) );
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( "Wow6432Node" );
    regOrganizer.beginGroup( "TypeLib" );
    regOrganizer.beginGroup( TYPELIB_UID );
    regOrganizer.beginGroup( "1.0" );
    regOrganizer.setValue( ".", "se_win 1.0 Type Library" );
    regOrganizer.beginGroup( "0" );
    regOrganizer.beginGroup( "win64" );
    regOrganizer.setValue( ".", QString( dllPath ) );
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.endGroup();

    regOrganizer.beginGroup( "Wow6432Node" );
    regOrganizer.beginGroup( "Classes" );
    regOrganizer.beginGroup( "TypeLib" );
    regOrganizer.beginGroup( TYPELIB_UID );
    regOrganizer.beginGroup( "1.0" );
    regOrganizer.setValue( ".", "se_win 1.0 Type Library" );
    regOrganizer.beginGroup( "0" );
    regOrganizer.beginGroup( "win64" );
    regOrganizer.setValue( ".", QString( dllPath ) );
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.sync();

    QSettings::Status status = regOrganizer.status();

    return ( status == QSettings::NoError ) ? S_OK : S_FALSE;
}

STDAPI UnRegisterSEAddIn( const CLSID& AddInCLSID )
{
    QString appVersion = "se_win.Solid Edge Addin.1";
    QString appVersionIndependentProgID = "se_win.Solid Edge Addin";
    QSettings regOrganizer( "HKEY_CURRENT_USER\\SOFTWARE\\Classes", QSettings::NativeFormat );

    regOrganizer.beginGroup( "CLSID" );
    HRESULT hr = S_OK;
    LPOLESTR lpClsidString;
    hr = StringFromCLSID( AddInCLSID, &lpClsidString );

    QString clsidStr = QString::fromWCharArray( ( wchar_t* )lpClsidString );
    regOrganizer.beginGroup( clsidStr );
    regOrganizer.remove( "" );
    regOrganizer.endGroup();
    regOrganizer.endGroup();

    regOrganizer.beginGroup( appVersion );
    regOrganizer.remove( "" );
    regOrganizer.endGroup();

    regOrganizer.beginGroup( appVersionIndependentProgID );
    regOrganizer.remove( "" );
    regOrganizer.endGroup();

    regOrganizer.sync();

    QSettings::Status status = regOrganizer.status();

    return ( status == QSettings::NoError ) ? S_OK : S_FALSE;
}
