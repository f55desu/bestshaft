#include "stdafx.h"
#include "util.h"

HRESULT GetSolidEdgeVersion( DWORD& dwMajor, DWORD& dwMinor, ApplicationPtr& pEdge )
{
    HRESULT hr = S_OK;

    if ( pEdge )
    {
        _bstr_t strVer = pEdge->Version;

        QString version = QString( _com_util::ConvertBSTRToString( strVer ) );

        if ( !version.isEmpty() )
        {
            QString majorVersion = "";
            QString minorVersion = "";
            QStringList versions = version.split( ".", QString::SkipEmptyParts );

            if ( versions.size() > 0 )
            {
                majorVersion = versions.at( 0 );

                if ( versions.size() > 1 )
                    minorVersion = versions.at( 1 );
            }

            if ( !majorVersion.isEmpty() )
                dwMajor = majorVersion.toInt();

            if ( !minorVersion.isEmpty() )
                dwMinor = minorVersion.toInt();
        }
    }
    else
        hr = E_POINTER;

    return hr;
}

// This routine returns the document type given the dispatch of a Solid Edge document.
// Returns E_INVALIDARG if pDocDispatch is NULL. Otherwise, returns the result from the Invoke
// method on that dispatch.
HRESULT GetDocType( LPDISPATCH pDocDispatch, DocumentTypeConstants& nDocType )
{
    HRESULT hr = NOERROR;

    nDocType = igUnknownDocument;

    try
    {
        if ( NULL != pDocDispatch )
        {
            DISPID rgDispId = 0;

            OLECHAR* Names[1] = {L"Type"};

            hr = pDocDispatch->GetIDsOfNames( IID_NULL,
                                              Names,
                                              1,
                                              LOCALE_USER_DEFAULT,
                                              &rgDispId );

            if ( SUCCEEDED( hr ) )
            {
                VARIANT varResult;
                VariantInit( &varResult );
                V_VT( &varResult ) = VT_I4;

                DISPPARAMS disp = { NULL, NULL, 0, 0 };

                // Get the document type property.
                hr = pDocDispatch->Invoke( rgDispId,
                                           IID_NULL,
                                           LOCALE_USER_DEFAULT,
                                           DISPATCH_PROPERTYGET,
                                           &disp,
                                           &varResult,
                                           NULL,
                                           NULL );

                if ( SUCCEEDED( hr ) )
                    nDocType = ( DocumentTypeConstants )( V_I4( &varResult ) );
                else
                    _com_issue_errorex( hr, pDocDispatch, __uuidof( pDocDispatch ) );
            }

        }
        else
            hr = E_INVALIDARG;
    }
    catch ( _com_error& e )
    {
        hr = e.Error();
    }

    return hr;
}

// This routine will return the type of located element given the dispatch interface of the element.
// If the type could not be determined, zero is returned (no element has a type value of zero)
// Returns E_INVALIDARG if pGraphicDispatch is NULL. Otherwise, returns the code from the Invoke
// method on the input dispatch interface.

// Note: The returned integer representing the object type should match one of the enumerated types
//       from either the SolidEdgeFramework::ObjectType or the SolidEdgeGeometry::GNTTypePropertyConstants
//       enumerations. Luckily, those two enumerated sets are defined such that there is no value
//       in common between the two. Which one of the two is expected to be returned from this routine
//       depends on the context in which the call is made.

HRESULT GetLocatedGraphicType( LPDISPATCH pGraphicDispatch, int& nObjectType )
{
    HRESULT hr = NOERROR;

    try
    {
        nObjectType = 0;

        if ( pGraphicDispatch )
        {
            DISPID rgDispId = 0;

            OLECHAR* Names[1] = {L"Type"};

            hr = pGraphicDispatch->GetIDsOfNames( IID_NULL,
                                                  Names,
                                                  1,
                                                  LOCALE_USER_DEFAULT,
                                                  &rgDispId );

            if ( SUCCEEDED( hr ) )
            {
                VARIANT varResult;
                VariantInit( &varResult );
                V_VT( &varResult ) = VT_I4;

                DISPPARAMS disp = { NULL, NULL, 0, 0 };

                hr = pGraphicDispatch->Invoke( rgDispId,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_PROPERTYGET,
                                               &disp,
                                               &varResult,
                                               NULL,
                                               NULL );

                if ( SUCCEEDED( hr ) )
                    nObjectType = V_I4( &varResult );
            }
        }
        else
            hr = E_INVALIDARG;
    }
    catch ( _com_error& e )
    {
        hr = e.Error();
    }

    return hr;
}
// In certain cases , the returned object is a Reference to the actual object that
// was located. This routine will return the type of located element behind the Reference object.
// If the type could not be determined, zero is returned (no element has a type value of zero).
// Returns E_INVALIDARG if the graphic dispatch is NULL or S_FALSE if the input graphic dispatch is
// not that of a Reference object. In the latter case, nObjectType will be the type of the input
// graphic object. Otherwise, returns the code from the Invoke method on the input
// dispatch interface. In addition, if pObjectDispatch is not NULL, the dispatch of
// the object behind the Reference object is also returned. And yes, if returned, its up to the
// caller to Release the interface once it is no longer needed.

// Note: Some known cases where a Reference object is returned:
//
//       Locates in the Assembly environment.
//       Locates of objects in a drawing view when located from the Draft file
//       containing the drawing view.

HRESULT GetLocatedGraphicTypeFromReference( LPDISPATCH pGraphicDispatch,
                                            int& nObjectType,
                                            LPUNKNOWN* pObjectUnknown )
{
    HRESULT hr = NOERROR;

    try
    {
        hr = GetLocatedGraphicType( pGraphicDispatch, nObjectType );

        if ( SUCCEEDED( hr ) )
        {
            // Assumed context of this call is from a locate performed in the Assembly environment.
            // Hence, the returned type should be enumerated in SolidEdgeFramework::ObjectType and
            // should be an igReference.

            if ( nObjectType == igReference )
            {
                // Obtain the object behind the reference and get its type.

                ReferencePtr pRef = pGraphicDispatch;

                IDispatchPtr pObject = pRef->GetObject();

                if ( NULL != pObject )
                {
                    DISPID rgDispId = 0;

                    OLECHAR* Names[1] = {L"Type"};

                    // Assume nothing! Get the id to pass to Invoke in the most robust way rather than assume
                    // the id will never change.

                    hr = pObject->GetIDsOfNames( IID_NULL,
                                                 Names,
                                                 1,
                                                 LOCALE_USER_DEFAULT,
                                                 &rgDispId );

                    if ( SUCCEEDED( hr ) )
                    {
                        VARIANT varResult;
                        VariantInit( &varResult );
                        V_VT( &varResult ) = VT_I4;

                        DISPPARAMS disp = { NULL, NULL, 0, 0 };

                        hr = pObject->Invoke( rgDispId,
                                              IID_NULL,
                                              LOCALE_USER_DEFAULT,
                                              DISPATCH_PROPERTYGET,
                                              &disp,
                                              &varResult,
                                              NULL,
                                              NULL );

                        if ( SUCCEEDED( hr ) )
                            nObjectType = V_I4( &varResult );
                    }

                    if ( NULL != pObjectUnknown )
                    {
                        // Return the dispatch pointer of the object.

                        pObject->QueryInterface( IID_IUnknown, ( LPVOID* )pObjectUnknown );
                    }
                }
            }
            else
            {
                if ( NULL != pObjectUnknown )
                {
                    // Return the dispatch pointer of the object.

                    pGraphicDispatch->QueryInterface( IID_IUnknown, ( LPVOID* )pObjectUnknown );
                }

                hr = S_FALSE;
            }
        }
    }
    catch ( _com_error& e )
    {
        hr = e.Error();
    }

    return hr;
}

HRESULT GetDocPtr( _bstr_t& strFilename, PartDocumentPtr& pPartDocument )
{
    HRESULT hr = S_OK;

    SolidEdgeDocumentPtr pDocument;

    DocumentTypeConstants nDocType = igUnknownDocument;

    hr = GetDocPtr( strFilename, pDocument, nDocType );

    pPartDocument = pDocument; // will be NULL if the pDocument is not a part document.

    return hr;
}

HRESULT GetDocPtr( _bstr_t& strFilename, DraftDocumentPtr& pDraftDocument )
{
    HRESULT hr = S_OK;

    SolidEdgeDocumentPtr pDocument;

    DocumentTypeConstants nDocType = igUnknownDocument;

    hr = GetDocPtr( strFilename, pDocument, nDocType );

    pDraftDocument = pDocument; // will be NULL if the pDocument is not a part document.

    return hr;
}

HRESULT GetDocPtr( _bstr_t& strFilename, SolidEdgeDocumentPtr& pSolidEdgeDocument, DocumentTypeConstants& nDocType )
{
    HRESULT hr = S_OK;

#if _MSC_VER < 1200   // 1200 or greater and compiler generates smart pointers everywhere
    IDispatch* pDocDispatch = NULL;
#else
    IDispatchPtr pDocDispatch;
#endif

    try
    {
        bool bGetName = false;

        // See if a name was passed in. If so, get the docs collection and attempt to get the
        // document using the name as the index.
        if ( 0 != strFilename.length() )
        {
            DocumentsPtr pDocs = GetApplicationPtr()->GetDocuments();

            if ( NULL != pDocs )
            {
                _variant_t vName( strFilename );

                pDocDispatch = pDocs->Item( vName );
            }
            else
                hr = E_UNEXPECTED;
        }
        else
        {
            pDocDispatch = GetApplicationPtr()->GetActiveDocument();
            bGetName = true;
        }

        pSolidEdgeDocument = pDocDispatch;

        if ( pSolidEdgeDocument )
        {
            nDocType = pSolidEdgeDocument->Type;

            if ( bGetName )
                strFilename = pSolidEdgeDocument->GetFullName();
        }
    }
    catch ( _com_error& e )
    {
        // Probably not open in solid edge. No big deal.
        hr = e.Error();
    }

    return hr;
}

// Most common cause of failure for this routine: file is not open in the current session of Solid Edge.
// S_FALSE if unknown document type (only handles assembly, part, sheetmetal and draft files).
// If strFilename is empty, this routine will grab the active document.

HRESULT GetDocPtr( _bstr_t& strFilename,
                   AssemblyDocumentPtr& pAssemblyDoc,
                   PartDocumentPtr& pPartDoc,
                   SheetMetalDocumentPtr& pSheetMetalDoc,
                   DraftDocumentPtr& pDraftDoc,
                   WeldmentDocumentPtr& pWeldmentDoc,
                   DocumentTypeConstants& nDocType )
{
    HRESULT hr = NOERROR;

    nDocType = igUnknownDocument;

    pAssemblyDoc = NULL;
    pPartDoc = NULL;
    pSheetMetalDoc = NULL;
    pDraftDoc = NULL;

    BOOL bGetName = FALSE;

    // Note: The Microsoft compiler's generate different signatures for the interfaces
    // generated from the typelibs depending on the compiler. Before VC6.0 (compiler ver 1200),
    // returned pure dispatch pointers were not smart pointers. That forces me to write conditionally
    // compiled code in order to avoid reference leaks that occur when assigning such an output
    // dispatch pointer to a smart pointer variable. The leak is due to the fact that the
    // dispatch pointer is returned on the stack and is lost once the assignment to the smart
    // pointer is done.

#if _MSC_VER < 1200   // 1200 or greater and compiler generates smart pointers everywhere
    IDispatch* pDocDispatch = NULL;
#else
    IDispatchPtr pDocDispatch;
#endif

    try
    {
        // See if a name was passed in. If so, get the docs collection and attempt to get the
        // document using the name as the index.
        if ( 0 != strFilename.length() )
        {
            DocumentsPtr pDocs = GetApplicationPtr()->GetDocuments();

            if ( NULL != pDocs )
            {
                _variant_t vName( strFilename );

                pDocDispatch = pDocs->Item( vName );
            }
            else
                hr = E_UNEXPECTED;
        }
        else
        {
            pDocDispatch = GetApplicationPtr()->GetActiveDocument();
            bGetName = TRUE;
        }

        if ( SUCCEEDED( hr ) && NULL != pDocDispatch )
        {
            if ( pDocDispatch )
            {
                VARIANT varResult;
                VariantInit( &varResult );
                V_VT( &varResult ) = VT_I4;

                DISPPARAMS disp = { NULL, NULL, 0, 0 };

                hr = pDocDispatch->Invoke( 0x46, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &disp, &varResult, NULL, NULL );

                if ( SUCCEEDED( hr ) )
                    nDocType = ( DocumentTypeConstants )( V_I4( &varResult ) );
                else
                    _com_issue_errorex( hr, pDocDispatch, __uuidof( pDocDispatch ) );
            }

            switch ( nDocType )
            {
                case igAssemblyDocument:
                {
                    pAssemblyDoc = pDocDispatch;

                    if ( NULL == pAssemblyDoc )
                        hr = E_UNEXPECTED;
                    else if ( bGetName )
                        strFilename = pAssemblyDoc->GetFullName();

                    break;
                }

                case igPartDocument:
                {
                    pPartDoc = pDocDispatch;

                    if ( NULL == pPartDoc )
                        hr = E_UNEXPECTED;
                    else if ( bGetName )
                        strFilename = pPartDoc->GetFullName();

                    break;
                }

                case igSheetMetalDocument:
                {
                    pSheetMetalDoc = pDocDispatch;

                    if ( NULL == pSheetMetalDoc )
                        hr = E_UNEXPECTED;
                    else if ( bGetName )
                        strFilename = pSheetMetalDoc->GetFullName();

                    break;
                }

                case igDraftDocument:
                {
                    pDraftDoc = pDocDispatch;

                    if ( NULL == pDraftDoc )
                        hr = E_UNEXPECTED;
                    else if ( bGetName )
                        strFilename = pDraftDoc->GetFullName();

                    break;
                }

                case igWeldmentDocument:
                {
                    pWeldmentDoc = pDocDispatch;

                    if ( NULL == pWeldmentDoc )
                        hr = E_UNEXPECTED;
                    else if ( bGetName )
                        strFilename = pWeldmentDoc->GetFullName();

                    break;
                }

                case igUnknownDocument:
                {
                    hr = S_FALSE;
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }
    catch ( _com_error& e )
    {
        // Probably not open in solid edge. No big deal.
        hr = e.Error();
    }

#if _MSC_VER < 1200
    C_RELEASE( pDocDispatch );
#endif

    return hr;
}

IUnknownPtr GetAddInStorage ( LPDISPATCH pDocDispatch, BSTR Name, long grfMode )
{
    ASSERT( pDocDispatch );

    IUnknownPtr pUnkStorage;

    SolidEdgeDocumentPtr pSolidEdgeDocument = pDocDispatch;

    if ( pSolidEdgeDocument )
        IUnknownPtr pUnk = pSolidEdgeDocument->GetAddInsStorage( Name, grfMode );

    return pUnkStorage;

    // This is what I did before the advent of SolidEdgeDocument API. I have since changed
    // the signature of the function from IUnknown* to IUnknownPtr.

    //IUnknown * _result;
    //_com_dispatch_method(pDocDispatch, 0x49, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&_result,
    // L"\x0008\x0003", Name, grfMode);
    //return _result;
}

HRESULT GetAddinStorage( LPDISPATCH pDocDispatch,
                         long grfMode,
                         _bstr_t StorageName,
                         IStoragePtr& pStorage
                       )
{
    ASSERT( pDocDispatch );

    HRESULT hr = NOERROR;

    try
    {
        IUnknownPtr pUnkStorage = GetAddInStorage( pDocDispatch, ( BSTR )StorageName, grfMode );

        if ( pUnkStorage )
            pStorage = pUnkStorage;
    }
    catch ( _com_error& e )
    {
        hr = e.Error();
    }

    return hr;
}

// https://community.plm.automation.siemens.com/t5/Solid-Edge-Developer-Forum/Features/m-p/21857/highlight/true#M5006
HRESULT ComGetProperty( const LPDISPATCH pDisp, LPOLESTR szProperty, VARTYPE propType, void* property )
{
    try
    {
        if ( pDisp == NULL )
            return E_POINTER;

        DISPID  rgDispId = 0;
        HRESULT hr = pDisp->GetIDsOfNames( IID_NULL, &szProperty, 1, LOCALE_USER_DEFAULT, &rgDispId );

        if ( SUCCEEDED( hr ) )
            hr = _com_dispatch_propget( pDisp, rgDispId, propType, property );

        return hr;
    }
    catch ( _com_error& e )
    {
        HRESULT hr = e.Error();
        return hr;
    }
}


HRESULT ComPutProperty( const LPDISPATCH pDisp, LPOLESTR szProperty, VARIANT_BOOL vBool )
{
    DISPID  rgDispId = 0;
    HRESULT hr = pDisp->GetIDsOfNames( IID_NULL, &szProperty, 1, LOCALE_USER_DEFAULT, &rgDispId );

    if ( SUCCEEDED( hr ) )
        hr = _com_dispatch_method( pDisp, rgDispId, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, L"\x000b", vBool );

    return hr;
}
