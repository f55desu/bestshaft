#include "Point3D.h"
#include "Utils.h"

#include "stdafx.h"

bool Utils::getVertexCoordinates( const VertexPtr& vertex, double* vertexCoordinates )
{
    long coordinateCount = 3;
    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = coordinateCount;

    SAFEARRAY* saCoordinates = NULL; /*::SafeArrayCreate(VT_R8, 1, &sabCoordinates);*/
    HRESULT hr = vertex->GetPointData( &saCoordinates );

    if ( SUCCEEDED( hr ) )
    {
        for ( long j = 0; j < 3; j++ )
        {
            double val = 0.0;
            hr = ::SafeArrayGetElement( saCoordinates, &j, &val );
            vertexCoordinates[j] = val;

            if ( !SUCCEEDED( hr ) )
            {
                SafeArrayDestroy( saCoordinates );
                return false;
            }
        }
    }

    SafeArrayDestroy( saCoordinates );
    return true;
}

bool Utils::getPointOnEdgeUseByParameter( const EdgeUsePtr& edgeUse, double parameter, Point3D& pointCoordinates )
{
    EdgePtr edge = edgeUse->Edge;
    return Utils::getPointOnEdgeByParameter( edge, parameter, pointCoordinates );
}

bool Utils::getPointOnEdgeByParameter( const EdgePtr& edge, double parameter, Point3D& point )
{
    SAFEARRAYBOUND sabParameters;
    sabParameters.lLbound = 0;
    sabParameters.cElements = 1;
    SAFEARRAY* saParameter = ::SafeArrayCreate( VT_R8, 1, &sabParameters );
    long index = 0;
    ::SafeArrayPutElement( saParameter, &index, &parameter );
    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = 3;
    SAFEARRAY* saCoordinates = ::SafeArrayCreate( VT_R8, 1, &sabCoordinates );
    HRESULT hr = edge->GetPointAtParam( 1, &saParameter, &saCoordinates );
    ::SafeArrayDestroy( saParameter );
    Q_ASSERT( SUCCEEDED( hr ) );
    Utils::getCoordinatesFromSafeArray( saCoordinates, point );
    ::SafeArrayDestroy( saCoordinates );
    return true;
}

Point3D Utils::getPointOnFaceByParameter( const FacePtr& face, double* parameterUV )
{
    SAFEARRAYBOUND sabParameters;
    sabParameters.lLbound = 0;
    sabParameters.cElements = 2;
    SAFEARRAY* saParameter = ::SafeArrayCreate( VT_R8, 1, &sabParameters );
    long index = 0;
    ::SafeArrayPutElement( saParameter, &index, ( void* )( &parameterUV[index] ) );
    ++index;
    ::SafeArrayPutElement( saParameter, &index, ( void* )( &parameterUV[index] ) );
    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = 3;
    SAFEARRAY* saCoordinates = ::SafeArrayCreate( VT_R8, 1, &sabCoordinates );
    HRESULT hr = face->GetPointAtParam( 1, &saParameter, &saCoordinates );
    ::SafeArrayDestroy( saParameter );
    Q_ASSERT( SUCCEEDED( hr ) );
    Point3D point;
    Utils::getCoordinatesFromSafeArray( saCoordinates, point );
    ::SafeArrayDestroy( saCoordinates );
    return point;
}

double Utils::planeEquation( const Point3D& point, const Point3D& basePoint, const Point3D& normalVector )
{
    return ( normalVector.x * ( point.x - basePoint.x ) +
             normalVector.y * ( point.y - basePoint.y ) +
             normalVector.z * ( point.z - basePoint.z ) );
}

bool Utils::getRootByChord( const EdgePtr& edge, double minT, double maxT, const Point3D& basePoint,
                            const Point3D& normalVector, double& result )
{
    double eps = 0.0001;
    double a = minT;
    double b = maxT;

    Point3D minTCoordinates, maxTCoordinates;
    Utils::getPointOnEdgeByParameter( edge, a, minTCoordinates );
    Utils::getPointOnEdgeByParameter( edge, b, maxTCoordinates );

    double fa = planeEquation( minTCoordinates, basePoint, normalVector );

    if ( Utils::checkDifference( fa, 0.0, e_absolute ) )
    {
        result = a;
        return true;
    }

    double fb = planeEquation( maxTCoordinates, basePoint, normalVector );

    if ( Utils::checkDifference( fb, 0.0, e_absolute ) )
    {
        result = b;
        return true;
    }

    if ( fa * fb >= 0 )
        return false;

    while ( fabs( b - a ) > eps )
    {
        Utils::getPointOnEdgeByParameter( edge, a, minTCoordinates );
        Utils::getPointOnEdgeByParameter( edge, b, maxTCoordinates );

        fa = planeEquation( minTCoordinates, basePoint, normalVector );
        fb = planeEquation( maxTCoordinates, basePoint, normalVector );

        a = b - ( b - a ) * fb / ( fb - fa );
        b = a + ( a - b ) * fa / ( fa - fb );
    }

    result = b;
    return true;
}

void Utils::writeVectorToFile( const QString& filename, const QVector3D& startPoint, const QVector3D& endPoint )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );
    ts << QString( "v %1 %2 %3" ).arg( startPoint.x() ).arg( startPoint.y() ).arg( startPoint.z() ) << endl;
    ts << QString( "v %1 %2 %3" ).arg( endPoint.x() + startPoint.x() ).arg( endPoint.y() + startPoint.y() )
       .arg( endPoint.z() + startPoint.z() ) << endl;
    ts.flush();

    ts << QString( "g %1" ).arg( filename ) << endl;
    ts << "l 1 2" << endl;
    ts.flush();

    outFile.close();
}

void Utils::writeVectorToFile( const QString& filename, const Point3D& startPoint, const Point3D& endPoint )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );
    ts << QString( "v %1 %2 %3" ).arg( startPoint.x ).arg( startPoint.y ).arg( startPoint.z ) << endl;
    ts << QString( "v %1 %2 %3" ).arg( endPoint.x + startPoint.x ).arg( endPoint.y + startPoint.y )
       .arg( endPoint.z + startPoint.z ) << endl;

    ts << QString( "g %1" ).arg( filename ) << endl;
    ts << "l 1 2" << endl;

    outFile.close();
}

void Utils::writeVectorsToFile( const QString& filename, const QVector3D& startPoint,
                                const QList<QVector3D>& endPoints )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    foreach ( QVector3D point, endPoints )
    {
        ts << QString( "v %1 %2 %3" ).arg( startPoint.x() ).arg( startPoint.y() ).arg( startPoint.z() ) << endl;
        ts << QString( "v %1 %2 %3" ).arg( point.x() + startPoint.x() ).arg( point.y() + startPoint.y() )
           .arg( point.z() + startPoint.z() ) << endl;
        ts.flush();
    }

    for ( int i = 0; i < endPoints.count(); i++ )
    {
        ts << QString( "g %1%2" ).arg( filename ).arg( i ) << endl;
        ts << QString( "l %1 %2" ).arg( i * 2 + 1 ).arg( i * 2 + 2 ) << endl;
        ts.flush();
    }

    outFile.close();
}

void Utils::writePointToFile( const QString& filename, const Point3D& point )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    ts << QString( "v %1 %2 %3" ).arg( point.x ).arg( point.y ).arg( point.z ) << endl;
    ts << QString( "p 1" ) << endl;
    ts.flush();

    outFile.close();
}

void Utils::writePointToFile( const QString& filename, const QVector3D& point )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    ts << QString( "v %1 %2 %3" ).arg( point.x() ).arg( point.y() ).arg( point.z() ) << endl;
    ts << QString( "p 1" ) << endl;
    ts.flush();

    outFile.close();
}

void Utils::writePointsToFile( const QString& filename, const QList<QVector3D>& points )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    foreach ( QVector3D point, points )
    {
        ts << QString( "v %1 %2 %3" ).arg( point.x() ).arg( point.y() ).arg( point.z() ) << endl;
        ts.flush();
    }

    for ( int i = 1; i <= points.count(); i++ )
        ts << QString( "p %1" ).arg( i ) << endl;

    ts.flush();

    outFile.close();
}

bool Utils::getCoordinatesFromSafeArray( SAFEARRAY* safeArray, QVector3D& resultVector )
{
    Q_ASSERT( safeArray );
    long index = 0;
    double coordinateX = 0.0;
    double coordinateY = 0.0;
    double coordinateZ = 0.0;
    HRESULT hr = ::SafeArrayGetElement( safeArray, & index, &coordinateX );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, & index, &coordinateY );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, & index, &coordinateZ );
    Q_ASSERT( SUCCEEDED( hr ) );
    resultVector = QVector3D( coordinateX, coordinateY, coordinateZ );
    return true;
}

bool Utils::getCoordinatesFromSafeArray( SAFEARRAY* safeArray, double* coordinates )
{
    Q_ASSERT( safeArray );
    long index = 0;
    HRESULT hr = ::SafeArrayGetElement( safeArray, &index, &coordinates[0] );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, &index, &coordinates[1] );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, &index, &coordinates[2] );
    Q_ASSERT( SUCCEEDED( hr ) );
    return true;
}

bool Utils::getCoordinatesFromSafeArray( SAFEARRAY* safeArray, Point3D& point )
{
    Q_ASSERT( safeArray );
    long index = 0;
    double coordinateX = 0.0;
    double coordinateY = 0.0;
    double coordinateZ = 0.0;
    HRESULT hr = ::SafeArrayGetElement( safeArray, &index, &coordinateX );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, &index, &coordinateY );
    Q_ASSERT( SUCCEEDED( hr ) );
    ++index;
    hr = ::SafeArrayGetElement( safeArray, &index, &coordinateZ );
    Q_ASSERT( SUCCEEDED( hr ) );
    point.x = coordinateX * 1.e3;
    point.y = coordinateY * 1.e3;
    point.z = coordinateZ * 1.e3;
    return true;
}

SAFEARRAY* Utils::createSafeArrayForCoordinates( const QVector3D& coordinates )
{
    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = 3;
    SAFEARRAY* saCoordinates = ::SafeArrayCreate( VT_R8, 1, &sabCoordinates );
    long indexOfElement = 0;
    double x = coordinates.x();
    double y = coordinates.y();
    double z = coordinates.z();
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &x );
    ++indexOfElement;
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &y );
    ++indexOfElement;
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &z );
    return saCoordinates;
}

SAFEARRAY* Utils::createSafeArrayForPoint( const Point3D& point )
{
    SAFEARRAYBOUND sabCoordinates;
    sabCoordinates.lLbound = 0;
    sabCoordinates.cElements = 3;
    SAFEARRAY* saCoordinates = ::SafeArrayCreate( VT_R8, 1, &sabCoordinates );
    long indexOfElement = 0;
    double x = point.x * 1.e-3;
    double y = point.y * 1.e-3;
    double z = point.z * 1.e-3;
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &x );
    ++indexOfElement;
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &y );
    ++indexOfElement;
    ::SafeArrayPutElement( saCoordinates, &indexOfElement, &z );
    return saCoordinates;
}

SAFEARRAY* Utils::createSafeArrayForParam( double param )
{
    SAFEARRAYBOUND sabParam;
    sabParam.lLbound = 0;
    sabParam.cElements = 3;
    SAFEARRAY* saParams = ::SafeArrayCreate( VT_R8, 1, &sabParam );
    long indexOfElement = 0;
    ::SafeArrayPutElement( saParams, &indexOfElement, &param );

    return saParams;
}

double Utils::pointToPointDistance( const QVector3D& a, const QVector3D& b )
{
    return sqrt( ( a.x() - b.x() ) * ( a.x() - b.x() ) +
                 ( a.y() - b.y() ) * ( a.y() - b.y() ) +
                 ( a.z() - b.z() ) * ( a.z() - b.z() ) );
}

QVector3D Utils::getMinPointOnClosedEdge( const EdgePtr& edge )
{
    const double goldenSectionRatio = ( 1 + sqrt( 5.0 ) ) / 2;
    double eps = 0.000001;

    double minParam, maxParam;
    edge->GetParamExtents( &minParam, &maxParam );
    double a = minParam;
    double b = maxParam;

    do
    {
        double t1 = b - ( b - a ) / goldenSectionRatio;
        double t2 = a + ( b - a ) / goldenSectionRatio;

        SAFEARRAY* saT1Param = Utils::createSafeArrayForParam( t1 );
        SAFEARRAY* saT2Param = Utils::createSafeArrayForParam( t2 );

        SAFEARRAY* saT1PointCoordinates = NULL;
        SAFEARRAY* saT2PointCoordinates = NULL;

        edge->GetPointAtParam( 1, &saT1Param, &saT1PointCoordinates );
        edge->GetPointAtParam( 1, &saT2Param, &saT2PointCoordinates );

        QVector3D t1Point, t2Point;
        Utils::getCoordinatesFromSafeArray( saT1PointCoordinates, t1Point );
        Utils::getCoordinatesFromSafeArray( saT2PointCoordinates, t2Point );

        ::SafeArrayDestroy( saT1Param );
        ::SafeArrayDestroy( saT2Param );
        ::SafeArrayDestroy( saT1PointCoordinates );
        ::SafeArrayDestroy( saT2PointCoordinates );

        if ( Utils::isFirstQVector3DLess( t1Point, t2Point ) == true )
        {
            b = t2;
            t2 = t1;
            t1 = b - ( b - a ) / goldenSectionRatio;
        }
        else
        {
            a = t1;
            t1 = t2;
            t2 = a + ( b - a ) / goldenSectionRatio;
        }
    } while ( fabs( b - a ) > eps );

    double t = ( a + b ) / 2;
    SAFEARRAY* saParam = Utils::createSafeArrayForParam( t );
    SAFEARRAY* saPointCoordinates = NULL;
    edge->GetPointAtParam( 1, &saParam, &saPointCoordinates );
    QVector3D minPoint;

    QVector3D basePoint( 0.0, 0.1, 1.0 );
    SAFEARRAY* saBasePoint = Utils::createSafeArrayForCoordinates( basePoint );
    SAFEARRAY* sa1 = NULL;
    SAFEARRAY* sa2 = NULL;
    SAFEARRAY* sa3 = NULL;
    SAFEARRAY* saParam1 = NULL;
    edge->GetParamAtPoint( 1, &saBasePoint, &sa1, &sa2, &saParam1, &sa3 );
    long index = 0;
    double val = 0.0;
    ::SafeArrayGetElement( saParam1, &index, &val );

    Utils::getCoordinatesFromSafeArray( saPointCoordinates, minPoint );
    ::SafeArrayDestroy( saParam );
    ::SafeArrayDestroy( saPointCoordinates );

    return minPoint;
}

bool Utils::isFirstQVector3DLess( const QVector3D& first, const QVector3D& second )
{
    double eps = 0.001;

    if ( fabs( first.x() - second.x() ) < eps ) // is equals
    {
        if ( fabs( first.y() - second.y() ) < eps )
            return ( first.z() < second.z() );
        else
            return ( first.y() < second.y() );
    }
    else
        return ( first.x() < second.x() );
}

IDispatchPtr Utils::getEntityByID( const PartDocumentPtr& document, int id )
{
    ModelsPtr modelsCollection = document->Models;
    ModelPtr model = modelsCollection->Item( 1 );
    BodyPtr body = model->GetBody();
    return body->GetEntityByID( id );
}

bool Utils::checkDifference( double a, double b, double e )
{
    return ::fabs( a - b ) < e;
}

/*double Utils::GoldenSection(const FacePtr& face, double* uv, double* x, double a, double b, int xyzIndex)
{
  double x1, x2, last;

  *x = x1 = b + ( b - a ) * tau;
  double y1 = Utils::getPointOnFaceByParameter(face, uv)[xyzIndex];
  *x = x2 = a + ( a - b ) * tau;
  double y2 = Utils::getPointOnFaceByParameter(face, uv)[xyzIndex];

  do
  {
    if (y2 < y1)//minimum
    {
      a = x1;
      x1 = x2;
      y1 = y2;
      *x = x2 = a + (a - b) * tau;
      last = y2 = Utils::getPointOnFaceByParameter(face, uv)[xyzIndex];
    }
    else
    {
      b = x2;
      x2 = x1;
      y2 = y1;
      *x = x1 = b + (b - a) * tau;
      last = y1 = Utils::getPointOnFaceByParameter(face, uv)[xyzIndex];
    }
  }
  while((b - a) > e_absolute);

  *x = (b + a) * 0.5;

  return last;
}*/

bool Utils::isEdgeALine( const EdgePtr& edge )
{
    if ( edge->IsClosed )
        return false;

    double limits[2];
    edge->GetParamExtents( &limits[0], &limits[1] );
    double lengthAtParam = DBL_MAX;
    edge->GetLengthAtParam( limits[0], limits[1], &lengthAtParam );

    VertexPtr startVertex, endVertex;
    startVertex = edge->GetStartVertex();
    endVertex = edge->GetEndVertex();

    double startVertexCoordinates[3], endVertexCoordinates[3];
    Utils::getVertexCoordinates( startVertex, startVertexCoordinates );
    Utils::getVertexCoordinates( endVertex, endVertexCoordinates );

    Point3D startPoint( startVertexCoordinates );
    Point3D endPoint( endVertexCoordinates );

    double lenghtAtPoints = startPoint.DistanceTo( endPoint );

    double e = lengthAtParam * 1.e-2 * e_percent;
    return ( Utils::checkDifference( lenghtAtPoints, lengthAtParam, e ) );
}

void Utils::writeFaceToFile( const QString& filename, const FacePtr& face )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    long facetsCount = 0;
    SAFEARRAY* saFacetCoordinates = NULL;
    QList<QVector3D> facetVertices;
    QStringList facets;

    HRESULT hr = face->GetFacetData( 0.0001, &facetsCount, &saFacetCoordinates );

    if ( SUCCEEDED( hr ) )
    {
        ts << QString( "o face%1\n" ).arg( face->GetTag() );

        for ( int facetNumber = 0; facetNumber < facetsCount; facetNumber++ )
        {
            QString facet = "f ";

            for ( long pointNumber = 0; pointNumber < 3; pointNumber++ )
            {
                double points[3] = {0.0};

                for ( long coordinateNumber = 0; coordinateNumber < 3; coordinateNumber++ )
                {
                    long coordinateIndex = facetNumber * 9 + pointNumber * 3 + coordinateNumber;
                    hr = SafeArrayGetElement( saFacetCoordinates, &coordinateIndex, &points[coordinateNumber] );

                    if ( !SUCCEEDED( hr ) )
                        break;
                }

                QVector3D facetVertex( points[0] * 1.e3, points[1] * 1.e3, points[2] * 1.e3 );
                int i = 0;

                for ( i; i < facetVertices.count(); i++ )
                {
                    if ( facetVertices[i] == facetVertex )
                        break;
                }

                if ( i == facetVertices.count() ) // vertex not found, add new
                {
                    facetVertices.append( facetVertex );
                    ts << QString( "v %1 %2 %3\n" ).arg( facetVertex.x() ).arg( facetVertex.y() ).arg( facetVertex.z() );
                }

                facet.append( QString( "%1 " ).arg( i + 1 ) ); // in .obj numbering starts with 1 instead of 0
            }

            facet.append( "\n" );
            facets.append( facet );
        }

        foreach ( QString facet, facets )
            ts << facet;
    }

    outFile.close();
}

void Utils::writeFaceTopology( const QString& filename, const FacePtr& face )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    int cnt = 1;
    LoopsPtr loops = face->Loops;

    for ( int i = 1, it = loops->Count; i <= it; i++ )
    {
        LoopPtr loop = loops->Item( i );
        EdgeUsesPtr edgeUses = loop->EdgeUses;

        for ( int j = 1, jt = edgeUses->Count; j <= jt; j++ )
        {
            EdgeUsePtr edgeUse = edgeUses->Item( j );
            EdgePtr edge = edgeUse->Edge;

            double limits[2];
            HRESULT hr = edge->GetParamExtents( &limits[0], &limits[1] );
            Q_ASSERT( SUCCEEDED( hr ) );

            SAFEARRAYBOUND sabParams;
            sabParams.lLbound = 0;
            sabParams.cElements = 1;
            SAFEARRAY* saParams = ::SafeArrayCreate( VT_R8, 1, &sabParams );
            Q_ASSERT( saParams != 0x0 );
            long index = 0;
            ::SafeArrayPutElement( saParams, &index, &limits[0] );

            SAFEARRAY* saCoordinates = NULL;
            hr = edge->GetPointAtParam( 1, &saParams, &saCoordinates );
            Q_ASSERT( SUCCEEDED( hr ) );
            Point3D startPoint;
            Utils::getCoordinatesFromSafeArray( saCoordinates, startPoint );
            ::SafeArrayDestroy( saCoordinates );

            ts << QString( "o edge%1\n" ).arg( edge->GetTag() );
            ts << QString( "v %1 %2 %3\n" ).arg( startPoint.x ).arg( startPoint.y ).arg( startPoint.z );
            ts << QString( "f %1 %1 %1\n" ).arg( cnt ).arg( cnt ).arg( cnt );

            SAFEARRAY* saTangent = 0x0;
            hr = edge->GetTangent( 1, &saParams, &saTangent );
            Q_ASSERT( SUCCEEDED( hr ) );
            ::SafeArrayDestroy( saParams );
            Vector3D tangent;
            Utils::getCoordinatesFromSafeArray( saTangent, tangent );

            int orientation = ( edgeUse->GetIsOpposedToEdge() == VARIANT_FALSE ? 1 : -1 );
            int reversed = ( edge->GetIsParamReversed() == VARIANT_FALSE ? 1 : -1 );

            tangent *= ( double )( orientation * reversed );

            double scale;
            hr = edge->GetLengthAtParam( limits[0], limits[1], &scale );
            Q_ASSERT( SUCCEEDED( hr ) );
            scale *= 5.e-1;

            ts << QString( "v %1 %2 %3" ).arg( startPoint.x + tangent.x * scale ).arg( startPoint.y + tangent.y * scale )
               .arg( startPoint.z + tangent.z * scale ) << endl;

            ts << QString( "l %1 %2" ).arg( cnt++ ).arg( cnt++ ) << endl;
        }
    }
}

void Utils::writePointsToFile( const QString& filename, const Points3DCollection& intersectionColl )
{
    QFile outFile( QString( "%1.obj" ).arg( filename ) );
    outFile.open( QFile::WriteOnly | QFile::Text );
    QTextStream ts( &outFile );

    foreach ( Point3D point, intersectionColl )
        ts << QString( "v %1 %2 %3\n" ).arg( point.x ).arg( point.y ).arg( point.z );

    int i = 0;

    foreach ( Point3D point, intersectionColl )
        ts << QString( "p %1" ).arg( ++i ) << endl;

    outFile.close();
}

void Utils::getFaceFirstPartialDerivative( const FacePtr face, const double* uv, Point3D& du, Point3D& dv )
{
    SAFEARRAYBOUND sabParam;
    sabParam.lLbound = 0;
    sabParam.cElements = 2;
    SAFEARRAY* saParams = ::SafeArrayCreate( VT_R8, 1, &sabParam );

    long i = 0;
    double _uv[2], h = 1.e-6;

    _uv[0] = uv[0] + h;
    _uv[1] = uv[1];
    i = 0;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );
    i++;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );

    SAFEARRAY* saCoordinates = 0x0;
    HRESULT hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
    Q_ASSERT( SUCCEEDED( hr ) );

    Point3D p[2];
    Utils::getCoordinatesFromSafeArray( saCoordinates, p[0] );

    _uv[0] = uv[0] - h;
    i = 0;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );

    hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
    Q_ASSERT( SUCCEEDED( hr ) );

    Utils::getCoordinatesFromSafeArray( saCoordinates, p[1] );

    double h2 = h * 2.;
    du.x = ( p[0].x - p[1].x ) / h2;
    du.y = ( p[0].y - p[1].y ) / h2;
    du.z = ( p[0].z - p[1].z ) / h2;

    _uv[0] = uv[0];
    _uv[1] = uv[1] + h;
    i = 0;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );
    i++;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );

    hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
    Q_ASSERT( SUCCEEDED( hr ) );

    Utils::getCoordinatesFromSafeArray( saCoordinates, p[0] );

    _uv[1] = uv[1] - h;
    i = 1;
    ::SafeArrayPutElement( saParams, &i, &_uv[i] );

    hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
    Q_ASSERT( SUCCEEDED( hr ) );

    Utils::getCoordinatesFromSafeArray( saCoordinates, p[1] );

    dv.x = ( p[0].x - p[1].x ) / h2;
    dv.y = ( p[0].y - p[1].y ) / h2;
    dv.z = ( p[0].z - p[1].z ) / h2;

    ::SafeArrayDestroy( saParams );
    ::SafeArrayDestroy( saCoordinates );
}
