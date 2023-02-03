#include "../stdafx.h"
#include "../commands.h"
#include "../GeometryUtils.h"
#include "../Utils.h"
#include "../SolidEdgeExtensionImpl.h"

void SolidEdgeExtensionImpl::setEpsilon( double epsilon )
{
    m_logger.info( "New epsilon value is %f", epsilon );
    m_epsilon = epsilon;
}

void SolidEdgeExtensionImpl::setTolerance( double tolerance )
{
    m_logger.info( "New tolerance value is %f", tolerance );
    m_tolerance = tolerance;
}

QJSValue SolidEdgeExtensionImpl::SELECT_Object( double xPoint,
                                                double yPoint,
                                                double zPoint, TopologyPriority topologyPriority )
{
    try
    {
        QTime t;
        t.start();
        m_logger.info( "Selecting object in point %f, %f, %f", xPoint, yPoint, zPoint );
        QJSValue closestObjectID( QJSValue::NullValue );
        int searchResult = -1;

        if ( topologyPriority == TP_VERTEX || topologyPriority == TP_ALL )
            searchResult = selectVertex( xPoint, yPoint, zPoint );

        if ( searchResult == -1 )
            if ( topologyPriority == TP_EDGE || topologyPriority == TP_ALL )
                searchResult = selectEdge( xPoint, yPoint, zPoint );

        if ( searchResult == -1 )
            if ( topologyPriority == TP_FACE || topologyPriority == TP_ALL )
                searchResult = selectFace( xPoint, yPoint, zPoint );

        m_logger.info( "SELECT_Object time = %i", t.elapsed() );

        if ( searchResult == -1 )
        {
            m_logger.info( "SELECT_Object: there no objects at point (%f, %f, %f)", xPoint, yPoint, zPoint );
            return QJSValue::NullValue;
        }

        m_logger.info( "SELECT_Object: The object with id %i founded at point (%f, %f, %f)", searchResult, xPoint, yPoint,
                       zPoint );

        return QJSValue( searchResult );
    }
    catch ( const _com_error& e )
    {
        QSCRIPT_COM_ERROR( e );
    }
    catch ( ... )
    {
        QSCRIPT_ERROR( "Unknown exeption in the extension." );
    }

    return QJSValue::NullValue;
}

int SolidEdgeExtensionImpl::selectVertex( double xPoint, double yPoint, double zPoint )
{
    QVector3D point( xPoint, yPoint, zPoint );
    int vertexId = -1;

    QVector<IDispatchPtr> allVertices( getAllVertices() );

    if ( !allVertices.isEmpty() )
    {
        for ( int i = 0; i < allVertices.size(); i++ )
        {
            VertexPtr pVertex = allVertices.at( i );

            long coordinateCount = 3;
            SAFEARRAYBOUND sab_coordinates;
            sab_coordinates.lLbound = 0;
            sab_coordinates.cElements = coordinateCount;

            SAFEARRAY* sa_coordinates = SafeArrayCreate( VT_R8, 1, &sab_coordinates );
            HRESULT hr = pVertex->GetPointData( &sa_coordinates );
            double* v = new double[3];

            if ( SUCCEEDED( hr ) )
            {
                for ( long j = 0; j < 3; j++ )
                {
                    hr = SafeArrayGetElement( sa_coordinates, &j, &v[j] );

                    if ( !SUCCEEDED( hr ) )
                    {
                        QSHOW_ERROR( "Cannot get vertex coordinates" );
                        delete [] v;
                        return vertexId;
                    }
                }
            }

            QVector3D vCoordinates( v[0], v[1], v[2] );
            double distance = Utils::pointToPointDistance( vCoordinates, point );
            delete [] v;

            if ( distance <= m_epsilon )
            {
                vertexId = pVertex->GetID();
                break;
            }
        }
    }

    return vertexId;
}

int SolidEdgeExtensionImpl::selectEdge( double xPoint, double yPoint, double zPoint )
{
    QVector3D point( xPoint, yPoint, zPoint );
    int edgeId = -1;

    QVector<IDispatchPtr> allEdges( getAllEdges() );

    if ( !allEdges.isEmpty() )
    {
        for ( int i = 0; i < allEdges.size(); i++ )
        {
            EdgePtr pEdge = allEdges.at( i );
            QVector3D edgePoint;

            if ( !pEdge->IsClosed )
            {
                if ( !GetEdgeMidpoint( pEdge, edgePoint ) )
                    return edgeId;
            }
            else
            {
                if ( !GetPointOnEdge( pEdge, edgePoint ) )
                    return edgeId;
            }

            double distance = Utils::pointToPointDistance( edgePoint, point );

            if ( distance <= m_epsilon )
            {
                edgeId = pEdge->GetID();
                break;
            }
        }
    }

    return edgeId;
}

int SolidEdgeExtensionImpl::selectFace( double xPoint, double yPoint, double zPoint )
{
    QVector3D point( xPoint, yPoint, zPoint );
    int faceId = -1;

    QVector<IDispatchPtr> allFaces( getAllFaces() );

    if ( !allFaces.isEmpty() )
    {
        for ( int i = 0; i < allFaces.size(); i++ )
        {
            FacePtr pFace = allFaces.at( i );
            QVector3D facePoint;

            if ( !GetFaceMidPoint( pFace, facePoint ) )
                return faceId;

            double distance = Utils::pointToPointDistance( facePoint, point );

            if ( distance <= m_epsilon )
            {
                faceId = pFace->GetID();
                break;
            }
        }
    }

    return faceId;
}

QVector<IDispatchPtr> SolidEdgeExtensionImpl::getAllEdges() const
{
    m_logger.debug( "Getting all edges" );
    QVector<IDispatchPtr> allEdges;

    try
    {
        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr pModelsCollection = pPartDocument->Models;

        for ( long i = 1; i <= pModelsCollection->Count; i++ )
        {
            ModelPtr pModel = pModelsCollection->Item( i );

            BodyPtr pBody = pModel->Body;
            EdgesPtr pEdgesCollection = pBody->Edges[igQueryAll];
            long edgesCount = pEdgesCollection->Count;

            for ( long j = 1; j <= edgesCount; j++ )
                allEdges.push_back( pEdgesCollection->Item( j ) );
        }
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = QString( "COM error in the extension: %1" ).arg( QString::fromLocal8Bit( e.ErrorMessage() ) );
        m_logger.error( qPrintable( errorMessage ) );
        m_scriptContext->throwError( errorMessage );
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exeption in the extension." );
        m_scriptContext->throwError( "Unknown exeption in the extension." );
    }

    return allEdges;
}

QVector<IDispatchPtr> SolidEdgeExtensionImpl::getAllFaces() const
{
    m_logger.debug( "Getting all faces" );
    QVector<IDispatchPtr> allFaces;

    try
    {
        PartDocumentPtr pPartDocument =
            CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr pModelsCollection = pPartDocument->Models;

        for ( int i = 1; i <= pModelsCollection->Count; i++ )
        {
            ModelPtr pModel = pModelsCollection->Item( i );
            BodyPtr pBody = pModel->Body;
            FacesPtr pFacesCollection = pBody->Faces[igQueryAll];

            for ( int j = 1; j <= pFacesCollection->Count; j++ )
                allFaces.push_back( pFacesCollection->Item( j ) );
        }
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = QString( "COM error in the extension: %1" ).arg( QString::fromLocal8Bit( e.ErrorMessage() ) );
        m_logger.error( qPrintable( errorMessage ) );
        m_scriptContext->throwError( errorMessage );
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exeption in the extension." );
        m_scriptContext->throwError( "Unknown exeption in the extension." );
    }

    return allFaces;
}

QVector<IDispatchPtr> SolidEdgeExtensionImpl::getAllVertices() const
{
    m_logger.debug( "Getting all vertices" );
    QVector<IDispatchPtr> allVertices;

    try
    {
        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr pModelsCollection = pPartDocument->Models;

        for ( int i = 1; i <= pModelsCollection->Count; i++ )
        {
            ModelPtr pModel = pModelsCollection->Item( i );
            BodyPtr pBody = pModel->Body;
            VerticesPtr pVerticesCollection = pBody->Vertices;

            for ( int j = 1; j <= pVerticesCollection->Count; j++ )
                allVertices.push_back( pVerticesCollection->Item( j ) );
        }
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = QString( "COM error in the extension: %1" ).arg( QString::fromLocal8Bit( e.ErrorMessage() ) );
        m_logger.error( qPrintable( errorMessage ) );
        m_scriptContext->throwError( errorMessage );
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exeption in the extension." );
        m_scriptContext->throwError( "Unknown exeption in the extension." );
    }

    return allVertices;
}

QJSValue SolidEdgeExtensionImpl::SELECT_Object_Octree( double xPoint, double yPoint, double zPoint )
{
    m_logger.info( "Selecting object by octree in point %f, %f, %f", xPoint, yPoint, zPoint );

    try
    {
        if ( m_octree == 0x0 )
            BUILD_Octree();

        if ( m_octree == 0x0 )
        {
            m_logger.warn( "SELECT_Object_Octree: Octree was not builded. It is impossible to find an object." );
            return QJSValue::NullValue;
        }

        QTime t;
        t.start();

        int objectId = m_octree->findPrimitiveByPoint( QVector3D( xPoint, yPoint, zPoint ) );
        m_logger.info( "SELECT_Object_Octree time %i", t.elapsed() );

        if ( objectId == -1 )
        {
            m_logger.debug( "SELECT_Object_Octree: Point (%f, %f, %f) outside octree", xPoint, yPoint, zPoint );
            return QJSValue::NullValue;
        }

        if ( objectId == 0 )
        {
            m_logger.debug( "SELECT_Object_Octree: There no objects at point (%f, %f, %f)", xPoint, yPoint, zPoint );
            return QJSValue::NullValue;
        }

        QString objectName = m_topologyObjectsNames.value( objectId );
        m_logger.info( "SELECT_Object_Octree: The object with name %s founded at point (%f, %f, %f)", qPrintable( objectName ),
                       xPoint, yPoint,
                       zPoint );
        return objectId;
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = QString( "COM error in the extension: %1" ).arg( QString::fromLocal8Bit( e.ErrorMessage() ) );
        m_logger.error( qPrintable( errorMessage ) );
        m_scriptContext->throwError( errorMessage );
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exeption in the extension." );
        m_scriptContext->throwError( "Unknown exeption in the extension." );
    }

    return QJSValue::NullValue;
}

void SolidEdgeExtensionImpl::BUILD_Octree()
{
    QTime t;
    t.start();

    try
    {
        FillPrimitivesCoordinates();

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );
        BodyPtr body = model->GetBody();

        SAFEARRAYBOUND sabPoint;
        sabPoint.lLbound = 0;
        sabPoint.cElements = 3;

        SAFEARRAY* saMinPoint = SafeArrayCreate( VT_R8, 1, &sabPoint );
        SAFEARRAY* saMaxPoint = SafeArrayCreate( VT_R8, 1, &sabPoint );

        if ( saMinPoint == 0x0 || saMaxPoint == 0x0 )
        {
            m_logger.warn( "Building octree: can not create safe arrays" );
            return;
        }

        HRESULT hr = body->GetRange( &saMinPoint, &saMaxPoint );

        if ( !SUCCEEDED( hr ) )
        {
            m_logger.warn( "Building octree: can not get bound box of body" );
            return;
        }

        double minPoint[3];
        double maxPoint[3];

        for ( long i = 0; i < 3; i++ )
        {
            SafeArrayGetElement( saMinPoint, &i, &minPoint[i] );
            SafeArrayGetElement( saMaxPoint, &i, &maxPoint[i] );
        }

        SafeArrayDestroy( saMinPoint );
        SafeArrayDestroy( saMaxPoint );

        QVector3D minPointCoordinates( minPoint[0], minPoint[1], minPoint[2] );
        QVector3D maxPointCoordinates( maxPoint[0], maxPoint[1], maxPoint[2] );

        m_octree = new Octant( minPointCoordinates, maxPointCoordinates );

        if ( m_octree->checkForEntities( m_topologyObjectCoordinates ) > 1 )
            m_octree->createSubOctants();

        m_logger.info( "BUILD_Octree time %i", t.elapsed() );

        m_logger.info( "BUILD_Octree: count of subOctants: %i", m_octree->getSubOctantsCount() );
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( qPrintable( QString( "COM error in extension: %1" )
                                    .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ) ) );
        return;
    }
    catch ( ... )
    {
        m_logger.error( "Building octree: unknown exception" );
        return;
    }
}
