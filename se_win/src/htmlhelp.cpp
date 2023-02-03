#include "stdafx.h"
#include "resource.h"

// Note: HtmlHelp crashes SolidEdge with Internet Explorer versions 2 or 3.
//       In order to prevent the crash, we check the version of IE here and return
//       E_FAIL if the version is not valid for html help.

static HRESULT ValidateRegisteredInternetExplorerVersion()
{
	HRESULT hr = E_FAIL; // If an acceptable version of Internet Explorer is not registered then return an error.

	CString strKeyPath;

	TCHAR szValue[MAX_PATH] = { '\0' };

	LONG lStatus = ERROR_SUCCESS;

	HKEY hKey = NULL;

	DWORD dwSize = MAX_PATH;
	DWORD dwType = 0;

	int nChars = 0;
	int iChar = 0;
	int nItems = 0;
	int nVerVal1 = 0;
	int nVerVal2 = 0;
	int nVerVal3 = 0;
	int nVerVal4 = 0;

	// If an acceptable version of Internet Explorer is not registered then return an error.
	strKeyPath = L"SOFTWARE\\Microsoft\\Internet Explorer";
	lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyPath, 0, KEY_QUERY_VALUE, &hKey );
	if( (ERROR_SUCCESS == lStatus) && (NULL != hKey) )
	{
		lStatus = RegQueryValueEx(hKey, _T("Version"), 0, &dwType, (BYTE *)szValue, &dwSize);
		if( (ERROR_SUCCESS == lStatus) && (REG_SZ == dwType) )
		{
			nChars = (int)_tcslen (szValue);
			for( iChar = 0;  iChar < nChars;  iChar ++ )
			{
				if('.' == szValue[iChar])
				{
					szValue[iChar] = ' ';
				}
			}

			nItems = _stscanf_s(szValue, "%d %d %d %d", &nVerVal1, &nVerVal2, &nVerVal3, &nVerVal4);
			if( 4 == nItems )
			{
				// Note that 4.72.2106.8 is the minimum or earliest version which we will accept.
				if( 4 < nVerVal1 )
				{
					hr = NOERROR; // An acceptable version of Internet Explorer is registered so return success.
				}
				else if( 4 == nVerVal1 )
				{
					if( 72 < nVerVal2 )
					{
						hr = NOERROR; // An acceptable version of Internet Explorer is registered so return success.
					}
					else if( 72 == nVerVal2 )
					{
						if( 2106 < nVerVal3 )
						{
							hr = NOERROR; // An acceptable version of Internet Explorer is registered so return success.
						}
						else if( 2106 == nVerVal3 )
						{
							if( 8 < nVerVal4 )
							{
								hr = NOERROR; // An acceptable version of Internet Explorer is registered so return success.
							}
							else if( 8 == nVerVal4 )
							{
								hr = NOERROR; // An acceptable version of Internet Explorer is registered so return success.
							}
						}
					}                    
				}
			} // End if read 4 items.
		} // End if query succeeded.

		RegCloseKey(hKey);

	} // End if open succeeded

	//wrapup:;
	return hr;
} // End ValidateRegisteredInternetExplorerVersion()

HWND WINAPI AddinHtmlHelpA ( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData )
{
	HRESULT hr = NOERROR;
  HWND    hwndHelp = NULL;

  // An acceptable version of Internet Explorer must be registered to launch htmlhelp.
  hr = ValidateRegisteredInternetExplorerVersion();
  if( NOERROR == hr )
  {
    hwndHelp = ::HtmlHelpA( hwndCaller, pszFile, uCommand, dwData );
  }
  else
  {
    AfxMessageBox( IDS_E_BAD_INTERNET_EXPLORER_VERSION );
  }

	return hwndHelp;
}

HWND WINAPI AddinHtmlHelpW (
    HWND hwndCaller,
    LPCWSTR pszFile,
    UINT uCommand,
    DWORD_PTR dwData
    )
{
	HRESULT hr = NOERROR;
  HWND hwndHelp = NULL;

  // An acceptable version of Internet Explorer must be registered to launch htmlhelp.
  hr = ValidateRegisteredInternetExplorerVersion();
  if( NOERROR == hr )
  {
    hwndHelp = ::HtmlHelpW( hwndCaller, pszFile, uCommand, dwData );
  }
  else
  {
    AfxMessageBox( IDS_E_BAD_INTERNET_EXPLORER_VERSION );
  }

	return hwndHelp;
}
