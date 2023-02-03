#include "stdafx.h"
#include "commands.h"
#include "TopWindow.h"

#include "SolidEdgeExtensionImpl.h"

SolidEdgeExtensionImpl SolidEdgeExtensionImpl::m_instance;
HHOOK SolidEdgeExtensionImpl::m_hHook;

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
    DWORD ProcID;
    ::GetWindowThreadProcessId( hwnd, &ProcID );

    if ( ::GetCurrentProcessId() == ProcID )
    {
        hwnd = ::GetAncestor( hwnd, GA_ROOTOWNER );
        *( ( LPDWORD )lParam ) = ( DWORD )hwnd;
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK SolidEdgeExtensionImpl::QtFilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp )
    {
        // don't process deferred-deletes while in a modal loop
        if ( BaseExtension::GetModalState() )
            qApp->sendPostedEvents();
        else
            qApp->sendPostedEvents( 0, -1 );
    }

    return ::CallNextHookEx( m_hHook, nCode, wParam, lParam );
}

SolidEdgeExtensionImpl::SolidEdgeExtensionImpl() :
    BaseExtension(),
    m_registered( 0 ),
    m_firstSolidFlag( false ),
    m_octree( 0x0 )
{
    m_epsilon = 1.0;
    m_tolerance = 0.01;

    m_topologyObjectsNames.clear();
}

SolidEdgeExtensionImpl::~SolidEdgeExtensionImpl()
{
}

SolidEdgeExtensionImpl& SolidEdgeExtensionImpl::Instance()
{
    return m_instance;
}

void SolidEdgeExtensionImpl::Initialize()
{
    BaseExtension::Initialize();

    if ( m_registered == 0 )
        m_registered = 1;
}

void SolidEdgeExtensionImpl::Terminate()
{
    if ( m_topWindow )
    {
        delete m_topWindow;
        m_topWindow = 0x0;
    }

    BaseExtension::TermQt();

    if ( m_hHook )
    {
        ::UnhookWindowsHookEx( m_hHook );
        m_hHook = 0x0;
    }
}

void SolidEdgeExtensionImpl::InitQt()
{
    if ( m_hHook == 0x0 )
    {
        m_hHook = ::SetWindowsHookEx( WH_GETMESSAGE,
                                      QtFilterProc,
                                      0,
                                      GetCurrentThreadId() );
    }

    BaseExtension::InitQt();
}

QWidget* SolidEdgeExtensionImpl::GetTopWindow()
{
    if ( m_topWindow )
        return m_topWindow;

    HWND appHwnd = ( HWND )CCommands::GetApplicationPtr()->hWnd;
    Q_ASSERT( appHwnd );

    //For windows system Qt has HWND mapped to WId
    return m_topWindow = new TopWindow( appHwnd );
}

QJSValue SolidEdgeExtensionImpl::CheckPostprocess()
{
    m_logger.info( "Checking postprocess" );

    if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
            CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
    {
        DocumentsPtr documentsCollection = CCommands::GetApplicationPtr()->Documents;

        if ( !documentsCollection )
            QSCRIPT_ERROR( "CheckPostprocess: Cannot get documents collection." );

        PartDocumentPtr newPartDocument = documentsCollection->Add( "SolidEdge.PartDocument" );

        if ( !newPartDocument )
            QSCRIPT_ERROR( "CheckPostprocess: Cannot create part document." );
    }

    CheckFirstSolidInPart();

    if ( m_firstSolidFlag )
        QSCRIPT_ERROR( "Model has already exist." );
    else
    {
        PartDocumentPtr currentPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( currentPartDocument )
            currentPartDocument->ModelingMode = seModelingModeOrdered;

        return QJSValue();
    }
}

void SolidEdgeExtensionImpl::CheckFirstSolidInPart()
{
    m_logger.info( "Check first solid in part" );
    PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;

    if ( !partDocument )
        return;

    ModelsPtr modelsCollection = partDocument->Models;

    if ( !modelsCollection )
        return;

    ModelPtr model = modelsCollection->Item( 1 );

    if ( !model )
        return;

    BodyPtr body = model->Body;
    m_firstSolidFlag = ( body != 0x0 );
}


QJSValue SolidEdgeExtensionImpl::CheckContextIsValid()
{
    m_logger.info( "Checking context is valid" );

    if ( CCommands::GetApplicationPtr()->ActiveDocument != 0x0 )
        return QJSValue();
    else
        QSCRIPT_ERROR( "Work part doesn't exist. Create new or open existing part model." );
}

QString SolidEdgeExtensionImpl::Test()
{
    m_logger.info( "Test" );
    setTolerance( 0.000001 );
//  SET_Names_For_Topology();
    DumpTopologyToObj();
//  dumpAll();

//  dumpTangents();

    return "";
}

void SolidEdgeExtensionImpl::FillPrimitivesCoordinates()
{
    QTime t;
    t.start();
    m_logger.info( "Filling hash of primitives coordinates" );

    m_topologyObjectCoordinates.clear();

    try
    {
        if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QSHOW_ERROR( "There no document or active document is not part document" );
            return;
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );
        BodyPtr body = model->GetBody();

        VerticesPtr vertices = body->Vertices;

        if ( vertices->Count != 0 )
        {
            for ( int i = 1; i < vertices->Count; i++ )
            {
                QVector3D vertexCordinates;
                VertexPtr vertex = vertices->Item( i );

                if ( GetCoordinatesFromVertex( vertex, vertexCordinates ) )
                    m_topologyObjectCoordinates.insert( vertex->GetID(), vertexCordinates );
                else
                    return;
            }
        }

        EdgesPtr edges = body->GetEdges( igQueryAll );

        if ( edges->Count != 0 )
        {
            for ( int i = 1; i < edges->Count; i++ )
            {
                QVector3D edgeCoordinates;
                EdgePtr edge = edges->Item( i );
                bool result = false;

                if ( edge->IsClosed )
                    result = GetPointOnEdge( edge, edgeCoordinates );
                else
                    result = GetEdgeMidpoint( edge, edgeCoordinates );

                if ( result == true )
                    m_topologyObjectCoordinates.insert( edge->GetID(), edgeCoordinates );
                else
                    return;
            }
        }

        FacesPtr faces = body->GetFaces( igQueryAll );

        if ( faces->Count != 0 )
        {
            for ( int i = 1; i < faces->Count; i++ )
            {
                QVector3D faceCordinates;
                FacePtr face = faces->Item( i );

                if ( GetFaceMidPoint( face, faceCordinates ) )
                    m_topologyObjectCoordinates.insert( face->GetID(), faceCordinates );
                else
                    return;
            }
        }

        m_logger.info( "Filling hash of primitives coordinates time = %i", t.elapsed() );
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( qPrintable( QString( "COM error in extension: %1" )
                                    .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ) ) );
        return;
    }
    catch ( ... )
    {
        m_logger.error( "Filling hash of primitives coordinates: unknown exception" );
        return;
    }
}
