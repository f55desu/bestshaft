#include "../stdafx.h"
#include "../commands.h"
#include "../SolidEdgeExtensionImpl.h"
#include "../GeometryUtils.h"
#include "../Utils.h"
#include "../Point3D.h"

using namespace std;

QJSValue SolidEdgeExtensionImpl::SET_Names_For_Topology( TopologyPriority topologyPriority )
{
    QTime t;
    t.start();
    m_logger.info( "Setting names for topology entities" );

    m_topologyObjectsNames.clear();
    m_topologyObjectCoordinates.clear();

    try
    {
        if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QSHOW_ERROR( "There no document or active document is not part document" );
            return QJSValue( false );
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );
        BodyPtr body = model->GetBody();

        if ( topologyPriority == TP_VERTEX || topologyPriority == TP_ALL )
        {
            VerticesPtr vertices = body->Vertices;

            if ( vertices->Count != 0 )
                setNamesForVertices( vertices );
        }

        if ( topologyPriority == TP_EDGE || topologyPriority == TP_ALL )
        {
            EdgesPtr edges = body->GetEdges( igQueryAll );

            if ( edges->Count != 0 )
                setNamesForEdges( edges );
        }

        if ( topologyPriority == TP_FACE || topologyPriority == TP_ALL )
        {
            FacesPtr faces = body->GetFaces( igQueryAll );

            if ( faces->Count != 0 )
                setNamesForFaces( faces );
        }

        m_logger.info( "Setting names for topology entities time = %i", t.elapsed() );
    }
    catch ( ... )
    {
        m_logger.error( "Setting names for topology entities: unknown exception" );
        return QJSValue( false );
    }

    return QJSValue( true );
}

bool SolidEdgeExtensionImpl::GetCoordinatesFromVertex( const VertexPtr& vertex, QVector3D& coordinates )
{
    long coordinateCount = 3;
    SAFEARRAYBOUND sab_coordinates;
    sab_coordinates.lLbound = 0;
    sab_coordinates.cElements = coordinateCount;

    SAFEARRAY* sa_coordinates = SafeArrayCreate( VT_R8, 1, &sab_coordinates );
    HRESULT hr = vertex->GetPointData( &sa_coordinates );
    double* vertexCoordinates = new double[3];

    if ( SUCCEEDED( hr ) )
    {
        for ( long i = 0; i < 3; i++ )
        {
            hr = SafeArrayGetElement( sa_coordinates, &i, &vertexCoordinates[i] );

            if ( !SUCCEEDED( hr ) )
                break;
        }
    }

    SafeArrayDestroy( sa_coordinates );

    if ( !SUCCEEDED( hr ) )
    {
        m_logger.warn( "Can not get coordinates from vertex %i", vertex->GetID() );
        return false;
    }

    coordinates = QVector3D( vertexCoordinates[0], vertexCoordinates[1], vertexCoordinates[2] );
    return true;
}

bool coordinateLessThen( const QVector3D& first, const QVector3D& second )
{
    const double tolerance = 0.00000001;

    if ( abs( first.x() - second.x() ) > tolerance )
        return first.x() < second.x();
    else if ( abs( first.y() - second.y() ) > tolerance )
        return first.y() < second.y();
    else
        return first.z() < second.z();
}

bool coordinateLessThenPair( const QPair<int, QVector3D>& first, const QPair<int, QVector3D>& second )
{
    QVector3D firstVertex = first.second;
    QVector3D secondVertex = second.second;

    return coordinateLessThen( firstVertex, secondVertex );
}

void SolidEdgeExtensionImpl::setNamesForVertices( const VerticesPtr& vertices )
{
    try
    {
        QList<QPair<int, QVector3D> > verticesCoordinates;

        for ( int i = 1; i <= vertices->Count; i++ )
        {
            VertexPtr vertex = vertices->Item( i );
            int vertexID = vertex->GetID();

            if ( !m_topologyObjectsNames.value( vertexID ).isEmpty() )
                continue;

            QVector3D vertexCoordinate;

            if ( !GetCoordinatesFromVertex( vertex, vertexCoordinate ) )
            {
                m_logger.error( "Setting names for topology entities: Cannot get vertex coordinates" );
                return;
            }

            verticesCoordinates.append( qMakePair( vertexID, vertexCoordinate ) );

            QString vertexInfo = QString( "Vertex id = %1, x = %2, y = %3, z = %4" )
                                 .arg( vertexID ).arg( vertexCoordinate.x() ).arg( vertexCoordinate.y() ).arg( vertexCoordinate.z() );
            m_logger.debug( vertexInfo.toStdString().c_str() );
        }

        qSort( verticesCoordinates.begin(), verticesCoordinates.end(), coordinateLessThenPair );

        for ( int i = 0; i < verticesCoordinates.size(); i++ )
        {
            QPair<int, QVector3D> vertexCoordinates = verticesCoordinates[i];
            QString vertexName = QString( "v%1" ).arg( i + 1 );
            m_topologyObjectsNames.insert( vertexCoordinates.first, vertexName );
            m_topologyObjectCoordinates.insert( vertexCoordinates.first, vertexCoordinates.second );
        }
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( QString( "COM error in extension: %s" )
                        .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exception while naming vertices" );
    }
}

// returns coordinates of point on given edge, that have minimal x, y, z coordinates
bool SolidEdgeExtensionImpl::GetPointOnEdge( const EdgePtr& edge, QVector3D& point )
{
    try
    {
        long strokeCount = edge->GetStrokeCount( m_tolerance );

        SAFEARRAYBOUND sabPoints;
        sabPoints.lLbound = 0;
        sabPoints.cElements = strokeCount * 3;

        SAFEARRAY* saPoints = SafeArrayCreate( VT_R8, 1, &sabPoints );

        if ( !saPoints )
        {
            m_logger.warn( "getPointOnEdge: can not create safearray" );
            return false;
        }

        SAFEARRAY* saParams = NULL;

        HRESULT hr = edge->GetStrokeData( m_tolerance, &strokeCount, &saPoints, &saParams );

        if ( !SUCCEEDED( hr ) )
        {
            SafeArrayDestroy( saPoints );
            m_logger.warn( "getPointOnEdge: can not get strokedata for edge %i", edge->GetID() );
            return false;
        }

        QList<QVector3D> pointsCoordinates;

        for ( long i = 0; i < strokeCount - 1; i++ )
        {
            double* coordinates = new double[3];

            long coordinateNumber = i * 3;
            hr = SafeArrayGetElement( saPoints, &coordinateNumber, &coordinates[0] );
            coordinateNumber++;
            hr = SafeArrayGetElement( saPoints, &coordinateNumber, &coordinates[1] );
            coordinateNumber++;
            hr = SafeArrayGetElement( saPoints, &coordinateNumber, &coordinates[2] );

            QVector3D newPoint( coordinates[0], coordinates[1], coordinates[2] );
            pointsCoordinates.append( newPoint );
        }

        SafeArrayDestroy( saPoints );

        qSort( pointsCoordinates.begin(), pointsCoordinates.end(), coordinateLessThen );

        point = pointsCoordinates[0];
        return true;
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( QString( "getPointOnEdge: COM error in extension: %1" )
                        .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
        return false;
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exception while getting edge coordinates" );
        return false;
    }
}

bool SolidEdgeExtensionImpl::GetEdgeMidpoint( const EdgePtr& edge, QVector3D& edgeMidpoint )
{
    try
    {
        VertexPtr startVertex = edge->GetStartVertex();
        QVector3D startVector;
        GetCoordinatesFromVertex( startVertex, startVector );

        VertexPtr endVertex = edge->GetEndVertex();
        QVector3D endVector;
        GetCoordinatesFromVertex( endVertex, endVector );

        int edgeID = edge->GetID();

        m_logger.debug( "Edge id = %i, sx = %f, sy = %f, sz = %f, ex = %f, ey = %f, ez = %f",
                        edgeID, startVector.x(), startVector.y(), startVector.z(), endVector.x(), endVector.y(), endVector.z() );

        double minParam = 0.0;
        double maxParam = 0.0;
        HRESULT hr = edge->GetParamExtents( &minParam, &maxParam );

        if ( !SUCCEEDED( hr ) )
        {
            m_logger.warn( "Can not get min and max params" );
            return false;
        }

        double midParam = ( minParam + maxParam ) / 2;
        const long numOfParams = 1;
        SAFEARRAYBOUND sabMidParam;
        sabMidParam.lLbound = 0;
        sabMidParam.cElements = numOfParams;
        SAFEARRAY* saMidParam = SafeArrayCreate( VT_R8, 1, &sabMidParam );

        if ( !saMidParam )
        {
            m_logger.warn( "Can not create safeArray while naming edges" );
            return false;
        }

        long indexOfElement = 0;
        SafeArrayPutElement( saMidParam, &indexOfElement, &midParam );

        SAFEARRAYBOUND sabMidPoint;
        sabMidPoint.lLbound = 0;
        sabMidPoint.cElements = 3;
        SAFEARRAY* saMidPointCoordinates = SafeArrayCreate( VT_R8, 1, &sabMidPoint );

        if ( !saMidPointCoordinates )
        {
            m_logger.warn( "Can not create safeArray while naming edges" );
            SafeArrayDestroy( saMidParam );
            return false;
        }

        hr = edge->GetPointAtParam( numOfParams, &saMidParam, &saMidPointCoordinates );
        SafeArrayDestroy( saMidParam );

        if ( !SUCCEEDED( hr ) )
        {
            m_logger.warn( "Can not get point at param" );
            SafeArrayDestroy( saMidPointCoordinates );
            return false;
        }

        double midPointXCoordinate = 0.0;
        double midPointYCoordinate = 0.0;
        double midPointZCoordinate = 0.0;

        indexOfElement = 0;
        SafeArrayGetElement( saMidPointCoordinates, &indexOfElement, &midPointXCoordinate );
        indexOfElement++;
        SafeArrayGetElement( saMidPointCoordinates, &indexOfElement, &midPointYCoordinate );
        indexOfElement++;
        SafeArrayGetElement( saMidPointCoordinates, &indexOfElement, &midPointZCoordinate );

        SafeArrayDestroy( saMidPointCoordinates );

        edgeMidpoint = QVector3D( midPointXCoordinate, midPointYCoordinate, midPointZCoordinate );

        QString edgeInfo = QString( "Edge id = %1, minParam = %2, maxParam = %3, midPoint coordinates: %4, %5, %6" )
                           .arg( edgeID ).arg( minParam ).arg( maxParam ).arg( midPointXCoordinate ).arg( midPointYCoordinate ).arg(
                               midPointZCoordinate );
        m_logger.debug( edgeInfo.toStdString().c_str() );

        return true;
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( QString( "COM error in extension: %1" )
                        .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
        return false;
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exception while naming edges" );
        return false;
    }
}

bool SolidEdgeExtensionImpl::GetFaceMidPoint( const FacePtr& face, QVector3D& faceMidpoint )
{
    return false;
}

void SolidEdgeExtensionImpl::setNamesForEdges( const EdgesPtr& edges )
{
    try
    {
        QList<QPair<int, QVector3D> > edgesCoordinates;

        for ( long i = 1; i <= edges->Count; i++ )
        {
            EdgePtr edge = edges->Item( i );
            int edgeID = edge->GetID();

            if ( !m_topologyObjectsNames.value( edgeID ).isEmpty() )
                continue;

            if ( edge->IsClosed )
            {
                QVector3D pointOnEdge;

                if ( !GetPointOnEdge( edge, pointOnEdge ) )
                {
                    m_logger.warn( "Can not get point on Edge" );
                    return;
                }

                m_logger.debug( "Edge id = %i is closed: x = %f, y = %f, z = %f",
                                edgeID, pointOnEdge.x(), pointOnEdge.y(), pointOnEdge.z() );
                edgesCoordinates.append( qMakePair( edgeID, pointOnEdge ) );
                continue;
            }

            QVector3D midPoint;

            if ( !GetEdgeMidpoint( edge, midPoint ) )
                return;

            edgesCoordinates.append( qMakePair( edgeID, midPoint ) );
        }

        qSort( edgesCoordinates.begin(), edgesCoordinates.end(), coordinateLessThenPair );

        for ( int i = 0; i < edgesCoordinates.size(); i++ )
        {
            QPair<int, QVector3D> edgeCoordinates = edgesCoordinates[i];
            QString edgeName = QString( "e%1" ).arg( i + 1 );
            m_topologyObjectsNames.insert( edgeCoordinates.first, edgeName );
            m_topologyObjectCoordinates.insert( edgeCoordinates.first, edgeCoordinates.second );
        }
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( QString( "COM error in extension: %1" )
                        .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
        return;
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exception while naming edges" );
    }
}

QVector3D getMinPoint( const QList<QVector3D>& points )
{
    QList<QVector3D> minXPoints;
    double minX = points.at( 0 ).x();

    foreach ( QVector3D point, points )
    {
        if ( point.x() < minX )
        {
            minXPoints.clear();
            minXPoints.append( point );
            minX = point.x();
        }
        else if ( point.x() == minX )
            minXPoints.append( point );
    }

    if ( minXPoints.size() == 1 )
        return minXPoints.at( 0 );

    QList<QVector3D> minYPoints;
    double minY = minXPoints.at( 0 ).y();

    foreach ( QVector3D point, minXPoints )
    {
        if ( point.y() < minY )
        {
            minYPoints.clear();
            minYPoints.append( point );
            minY = point.y();
        }
        else if ( point.y() == minY )
            minYPoints.append( point );
    }

    if ( minYPoints.size() == 1 )
        return minYPoints.at( 0 );

    QVector3D minZPoint;
    double minZ = minYPoints.at( 0 ).z();

    foreach ( QVector3D point, minYPoints )
    {
        if ( point.z() < minZ )
        {
            minZPoint = point;
            minZ = point.z();
        }
    }

    return minZPoint;
}

Point3D getMinPoint( const EdgePtr& edge, const QVector3D& pointCoordinates, const QVector3D& normalVectorCoordinates )
{
    double tolerance = 0.0001;
    double minParam = 0.0;
    double maxParam = 0.0;
    edge->GetParamExtents( &minParam, &maxParam );

    double a = maxParam - ( maxParam - minParam ) / 1.618;
    double b = minParam + ( maxParam - minParam ) / 1.618;

    SAFEARRAYBOUND sabParams;
    sabParams.lLbound = 0;
    sabParams.cElements = 2;

    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = 6;

    while ( abs( b - a ) < tolerance )
    {
        Point3D aCoordinates, bCoordinates;
        Utils::getPointOnEdgeByParameter( edge, a, aCoordinates );
        Utils::getPointOnEdgeByParameter( edge, b, bCoordinates );

        double af = normalVectorCoordinates.x() * ( aCoordinates.x - pointCoordinates.x() ) +
                    normalVectorCoordinates.y() * ( aCoordinates.y - pointCoordinates.y() ) +
                    normalVectorCoordinates.z() * ( aCoordinates.z - pointCoordinates.z() );

        double bf = normalVectorCoordinates.x() * ( bCoordinates.x - pointCoordinates.x() ) +
                    normalVectorCoordinates.y() * ( bCoordinates.y - pointCoordinates.y() ) +
                    normalVectorCoordinates.z() * ( bCoordinates.z - pointCoordinates.z() );

        if ( af > bf )
        {
            minParam = a;
            a = b;
            b = minParam + ( maxParam - minParam ) / 1.618;
        }
        else
        {
            maxParam = b;
            b = a;
            a = maxParam - ( maxParam - minParam ) / 1.618;
        }
    }

    double resultParam = ( minParam + maxParam ) / 2;
    Point3D intersectCoordinate;
    Utils::getPointOnEdgeByParameter( edge, resultParam, intersectCoordinate );
    return intersectCoordinate;
}

QList<Point3D> getIntersectionPointsCoordinates( const FacePtr& face, const Point3D& basePointCoordinates,
                                                 const Point3D& normalVectorCoordinates )
{
    QList<Point3D> intersectionPointsCoordinates;
    EdgesPtr edges = face->GetEdges();

    for ( int i = 1; i <= edges->Count; i++ )
    {
        QList<Point3D> intersectionPoints;
        EdgePtr edge = edges->Item( i );
        double minParam, maxParam;
        edge->GetParamExtents( &minParam, &maxParam );

        double intersectParam = 0.0;

        if ( Utils::getRootByChord( edge, minParam, maxParam, basePointCoordinates, normalVectorCoordinates, intersectParam ) )
        {
            Point3D intersectionPoint;
            Utils::getPointOnEdgeByParameter( edge, intersectParam, intersectionPoint );

            if ( intersectionPoint != basePointCoordinates )
                intersectionPoints.append( intersectionPoint );
        }

        if ( intersectionPoints.count() == 0 )
        {
            double halfParam = ( minParam + maxParam ) / 2;

            if ( Utils::getRootByChord( edge, minParam, halfParam, basePointCoordinates, normalVectorCoordinates, intersectParam ) )
            {
                Point3D intersectionPoint;
                Utils::getPointOnEdgeByParameter( edge, intersectParam, intersectionPoint );

                if ( intersectionPoint != basePointCoordinates )
                    intersectionPoints.append( intersectionPoint );
            }

            if ( Utils::getRootByChord( edge, halfParam, maxParam, basePointCoordinates, normalVectorCoordinates, intersectParam ) )
            {
                Point3D intersectionPoint;
                Utils::getPointOnEdgeByParameter( edge, intersectParam, intersectionPoint );

                if ( intersectionPoint != basePointCoordinates )
                    intersectionPoints.append( intersectionPoint );
            }
        }

        intersectionPointsCoordinates.append( intersectionPoints );
    }

    return intersectionPointsCoordinates;
}

Point3D& SolidEdgeExtensionImpl::GetClosedToBasePoint( const FacePtr& face,
                                                       const QVector<Point3D>& intersectionPointsCollection,
                                                       Point3D& closedPoint )
{
    return closedPoint;
}

void SolidEdgeExtensionImpl::setNamesForFaces( const FacesPtr& faces )
{
    try
    {
        QList<QPair<int, QVector3D> > facesCoordinates;

        for ( int i = 1; i <= faces->Count; i++ )
        {
            FacePtr face = faces->Item( i );
            int faceID = face->GetID();

            if ( !m_topologyObjectsNames.value( faceID ).isEmpty() )
                continue;

            QVector3D midPoint;

            if ( !GetFaceMidPoint( face, midPoint ) )
                return;

            //      facesCoordinates.append(qMakePair(faceID, midPoint));
        }

        /*qSort(facesCoordinates.begin(), facesCoordinates.end(), coordinateLessThenPair);

        for (int i = 0; i < facesCoordinates.size(); i++)
        {
          QPair<int, QVector3D> faceCoordinates = facesCoordinates[i];
          QString faceName = QString("f%1").arg(i + 1);
          m_topologyObjectsNames.insert(faceCoordinates.first, faceName);
          m_topologyObjectCoordinates.insert(faceCoordinates.first, faceCoordinates.second);
        }*/
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( qPrintable( QString( "COM error in extension: %1" )
                                    .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ) ) );
        return;
    }
    catch ( ... )
    {
        m_logger.error( "Unknown exception while naming faces" );
        return;
    }
}
