#ifndef UTIL_H
#define UTIL_H

using namespace SolidEdgeFramework;
using namespace SolidEdgePart;
using namespace SolidEdgeDraft;
using namespace SolidEdgeAssembly;

HRESULT GetSolidEdgeVersion( DWORD& dwMajor, DWORD& dwMinor, ApplicationPtr& pEdge );

HRESULT GetDocType( LPDISPATCH pDocDispatch,
                    DocumentTypeConstants& nDocType );

HRESULT GetLocatedGraphicType( LPDISPATCH pGraphicDispatch,
                               int& nObjectType );

HRESULT GetLocatedGraphicTypeFromReference( LPDISPATCH pGraphicDispatch,
                                            int& nObjectType,
                                            LPUNKNOWN* pObjectUnk = NULL );

HRESULT GetDocPtr( _bstr_t& strFilename, SolidEdgeDocumentPtr& pSolidEdgeDocument, DocumentTypeConstants& nDocType );

HRESULT GetDocPtr( _bstr_t& strFilename, PartDocumentPtr& pSolidEdgeDocument );

HRESULT GetDocPtr( _bstr_t& strFilename, DraftDocumentPtr& pSolidEdgeDocument );

HRESULT GetDocPtr( _bstr_t& strFilename,
                   AssemblyDocumentPtr& pAssemblyDoc,
                   PartDocumentPtr& pPartDoc,
                   SheetMetalDocumentPtr& pSheetMetalDoc,
                   DraftDocumentPtr& pDraftDoc,
                   WeldmentDocumentPtr& pWeldmentDoc,
                   DocumentTypeConstants& nDocType );

HRESULT GetAddinStorage( LPDISPATCH pDocDispatch,
                         long grfMode,
                         _bstr_t StorageName,
                         IStoragePtr& pStorage
                       );

HRESULT ComGetProperty( const LPDISPATCH pDisp, LPOLESTR szProperty, VARTYPE propType, void* property );
HRESULT ComPutProperty( const LPDISPATCH pDisp, LPOLESTR szProperty, VARIANT_BOOL vBool );

#endif
