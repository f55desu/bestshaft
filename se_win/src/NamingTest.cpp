#include "stdafx.h"
#include "SolidEdgeExtensionImpl.h"
#include "Utils.h"
#include "Point3D.h"
#include "commands.h"

double Point3D::m_tolerance = e_point_absolute;

int Evaluate_t_Snap( const EdgePtr& edge, double* X, double* limits, Point3D& point, bool );
Point3D& edgeObjective( const EdgePtr& edge, const double* t, Point3D& point );
int GoldenSection( const EdgePtr& edge, const double* product, double* x, double a, double b,
                   Point3D& lastPoint, double& lastValue, double e ); // pass SE object here
int GoldenSectionMax( const EdgePtr& edge, const double* product, double* x, double a, double b,
                      Point3D& lastPoint, double& lastValue, double e ); // pass SE object here
int GoldenSection( const FacePtr& face, const double* uv, const double* product, double* x, double a, double b,
                   double& lastValue, Point3D& surfacePoint, double e );

enum ErrorCode
{
    NoError = 0,
    CommonPointNotFound,
    EdgeCutPlaneIntersectEmpty,
    ClosedIntersectionPointNotFound
};

struct EdgeInfo
{
public:
    inline EdgeInfo()
    {
        edge = 0x0,
        limits[0] = std::numeric_limits<double>::max(),
                    limits[1] = std::numeric_limits<double>::max(),
                                tParamBasePoint = std::numeric_limits<double>::max(),
                                minmax = -2;
    }
    Point3D vertexes[2];
    EdgePtr edge;
    double limits[2];
    double tParamBasePoint;
    int minmax;
    Point3D onEdgePoint;
};

struct LoopBasePointInfo
{
public:
    inline LoopBasePointInfo()
    {
        index[0] = 0,
                   index[1] = 1,
                              cutPlane[0] = 0.0,
                                            cutPlane[1] = 0.0,
                                                          cutPlane[2] = 0.0,
                                                                        cutPlane[3] = 0.0;
    }

    void SwapEdgesIndex()
    {
        int tmp = index[0];
        index[0] = index[1];
        index[1] = tmp;
    }

    int FindBasePoint()
    {
        if ( adjEdges[0].edge == adjEdges[1].edge )
        {
            if ( adjEdges[0].minmax == -2 )
            {
                point = adjEdges[0].vertexes[0];
                adjEdges[0].minmax = -1;
                adjEdges[0].tParamBasePoint = adjEdges[0].limits[0];
            }
            else
                point = adjEdges[0].onEdgePoint;

            return NoError;
        }

        int i, j;

        for ( i = 0; i < 2; i++ )
        {
            if ( adjEdges[i].minmax == -1 ) //find edge base point lying on
            {
                if ( adjEdges[i].onEdgePoint < point )
                {
                    point = adjEdges[i].onEdgePoint;

                    if ( i == 1 )
                        SwapEdgesIndex();
                }
            }
        }

        if ( point )
            return NoError;

        for ( i = 0; i < 2; i++ )
            for ( j = 0; j < 2; j++ )
                if ( adjEdges[0].vertexes[i].IsClosedTo2( adjEdges[1].vertexes[j] ) &&
                        //            adjEdges[0].vertexes[i] == adjEdges[1].vertexes[j] &&
                        adjEdges[0].vertexes[i] < point )
                {
                    point = adjEdges[0].vertexes[i];
                    adjEdges[0].tParamBasePoint = adjEdges[0].limits[i];
                    adjEdges[0].minmax = i;
                    adjEdges[1].tParamBasePoint = adjEdges[1].limits[j];
                    adjEdges[1].minmax = j;
                }

        if ( point )
            return NoError;

        return CommonPointNotFound;
    }

    bool CalcCutPlane()
    {
        int n;
        double direction[2] = {1.};

        if ( adjEdges[index[0]].minmax == -1 )
            n = 1;
        else
        {
            n = 2;
            direction[1] = ( ( adjEdges[0].minmax != adjEdges[1].minmax ) ? -1. : 1. );
        }

        Point3D derivatives[4] = { Point3D( 0. ), Point3D( 0. ), Point3D( 0. ), Point3D( 0. ) };

        for ( int i = 0, j = 0; i < n; i++, j += 2 )
        {
            Point3D p;
            Utils::getPointOnEdgeByParameter( adjEdges[index[i]].edge, adjEdges[index[i]].tParamBasePoint, p );

            SAFEARRAYBOUND sabParam;
            sabParam.lLbound = 0;
            sabParam.cElements = 1;
            SAFEARRAY* saParam = ::SafeArrayCreate( VT_R8, 1, &sabParam );
            Q_ASSERT( saParam );

            SAFEARRAY* firstDerivative = NULL;
            SAFEARRAY* secondDerivative = NULL;
            SAFEARRAY* thirdDerivative = NULL;
            HRESULT hr = adjEdges[index[i]].edge->GetDerivatives( 1, &saParam, &firstDerivative, &secondDerivative,
                                                                  &thirdDerivative );
            Q_ASSERT( SUCCEEDED( hr ) );

            Utils::getCoordinatesFromSafeArray( firstDerivative, derivatives[j] );
            Utils::getCoordinatesFromSafeArray( secondDerivative, derivatives[j + 1] );

            ::SafeArrayDestroy( saParam );
            ::SafeArrayDestroy( firstDerivative );
            ::SafeArrayDestroy( secondDerivative );
            ::SafeArrayDestroy( thirdDerivative );

            derivatives[j] *= direction[i];
            derivatives[j].Norm();
        }

        Point3D normal = derivatives[0] - derivatives[2];

        if ( normal.Length() > e_absolute )
        {
            cutPlane[0] = normal[0];
            cutPlane[1] = normal[1];
            cutPlane[2] = normal[2];
            cutPlane[3] = point * normal * -1.;
        }
        else
        {
            if ( derivatives[1].Length() > e_absolute && derivatives[3].Length() > e_absolute )
            {
                cutPlane[0] = derivatives[1].x;
                cutPlane[1] = derivatives[1].y;
                cutPlane[2] = derivatives[1].z;
                cutPlane[3] = point * normal * -1.;
            }
            else
                return false;
        }

        return true;
    }

    void LoadEdgeInfo( int edgeIndex, const EdgePtr& edge )
    {
        Q_ASSERT( edgeIndex >= 0 && edgeIndex < 2 );

        adjEdges[edgeIndex].edge = edge;

        HRESULT hr = adjEdges[edgeIndex].edge->GetParamExtents( &adjEdges[edgeIndex].limits[0],
                                                                &adjEdges[edgeIndex].limits[1] );
        Q_ASSERT( SUCCEEDED( hr ) );

        if ( !Utils::isEdgeALine( edge ) )
        {
            int result = Evaluate_t_Snap( adjEdges[edgeIndex].edge, &adjEdges[edgeIndex].tParamBasePoint,
                                          adjEdges[edgeIndex].limits, adjEdges[edgeIndex].onEdgePoint, false );
            adjEdges[edgeIndex].minmax = ( result == MINIMUM_EXISTS ? -1 : -2 );
        }

        Utils::getPointOnEdgeByParameter( adjEdges[edgeIndex].edge, adjEdges[edgeIndex].limits[0],
                                          adjEdges[edgeIndex].vertexes[0] );
        Utils::getPointOnEdgeByParameter( adjEdges[edgeIndex].edge, adjEdges[edgeIndex].limits[1],
                                          adjEdges[edgeIndex].vertexes[1] );
    }

    void MoveNextEdge()
    {
        adjEdges[0] = adjEdges[1];
        point.Reset();
        index[0] = 0;
        index[1] = 1;
    }

    Point3D point;
    EdgeInfo adjEdges[2];
    double cutPlane[4];
    int index[2];
};

void GetClosestUVPointOnSurface( const FacePtr& face, const Point3D& point, double* uv )
{
    const double tol = 1.e-6;
    double _uv[2];

    SAFEARRAY* saPoints = Utils::createSafeArrayForPoint( point );
    SAFEARRAY* saParams = 0x0;
    SAFEARRAY* saGuessParam = 0x0;
    SAFEARRAY* saMaxDeviation = 0x0;
    SAFEARRAY* saFlags = 0x0;
    face->GetParamAtPoint( 1, &saPoints, &saGuessParam, &saMaxDeviation, &saParams, &saFlags );

    for ( long i = 0; i < 4; i++ )
        ::SafeArrayGetElement( saParams, &i, &_uv[i] );

    ::SafeArrayDestroy( saPoints );
    ::SafeArrayDestroy( saGuessParam );
    ::SafeArrayDestroy( saMaxDeviation );
    ::SafeArrayDestroy( saParams );
    ::SafeArrayDestroy( saFlags );

    if ( Utils::checkDifference( _uv[0], uv[0], tol ) && Utils::checkDifference( _uv[1], uv[1], tol ) )
    {
        uv[0] = _uv[0];
        uv[1] = _uv[1];
    }
    else
    {
        double limits[4];
        SAFEARRAY* minParams = NULL;
        SAFEARRAY* maxParams = NULL;
        HRESULT hr = face->GetParamRange( &minParams, &maxParams );
        Q_ASSERT( SUCCEEDED( hr ) );
        long param = 0;
        ::SafeArrayGetElement( minParams, &param, &limits[0] );
        ::SafeArrayGetElement( maxParams, &param, &limits[1] );
        ++param;
        ::SafeArrayGetElement( minParams, &param, &limits[2] );
        ::SafeArrayGetElement( maxParams, &param, &limits[3] );
        ::SafeArrayDestroy( minParams );
        ::SafeArrayDestroy( maxParams );

        if ( ( limits[0] < _uv[0] || Utils::checkDifference( limits[0], _uv[0], tol ) ) &&
                ( limits[1] > _uv[0] || Utils::checkDifference( limits[1], _uv[0], tol ) ) &&
                ( limits[2] < _uv[1] || Utils::checkDifference( limits[2], _uv[1], tol ) ) &&
                ( limits[3] > _uv[1] || Utils::checkDifference( limits[3], _uv[1], tol ) ) )
        {
            uv[0] = _uv[0];
            uv[1] = _uv[1];
        }
    }
}

int EvaluateLoopBasePoint( const LoopPtr& loop, LoopBasePointInfo& loopBasePointInfo )
{
    int code = 0;

    EdgesPtr edges = loop->GetEdges();

    LoopBasePointInfo currentLoopBasePointInfo;
    currentLoopBasePointInfo.LoadEdgeInfo( 0, edges->Item( 1 ) );

    EdgeInfo firstEdgeInfo = currentLoopBasePointInfo.adjEdges[0];

    for ( long i = 2; i <= edges->Count; i++ )
    {
        EdgePtr edge = edges->Item( i );
        currentLoopBasePointInfo.LoadEdgeInfo( 1, edge );

        if ( code = currentLoopBasePointInfo.FindBasePoint() )
            return code;

        if ( currentLoopBasePointInfo.point < loopBasePointInfo.point && currentLoopBasePointInfo.CalcCutPlane() )
            loopBasePointInfo = currentLoopBasePointInfo;

        currentLoopBasePointInfo.MoveNextEdge();
    }

    //Check first and last edges of loop
    currentLoopBasePointInfo.adjEdges[1] = firstEdgeInfo;

    if ( code = currentLoopBasePointInfo.FindBasePoint() )
        return code;

    if ( currentLoopBasePointInfo.point < loopBasePointInfo.point && currentLoopBasePointInfo.CalcCutPlane() )
        loopBasePointInfo = currentLoopBasePointInfo;

    return code;
}

bool TraverseTopology( const FacePtr& face, const EdgePtr& edge, int& orientation )
{
    LoopsPtr loops = face->Loops;

    for ( int i = 1, it = loops->Count; i <= it; i++ )
    {
        LoopPtr loop = loops->Item( i );
        EdgeUsesPtr edgeUses = loop->EdgeUses;

        for ( int j = 1, jt = edgeUses->Count; j <= jt; j++ )
        {
            EdgeUsePtr edgeUse = edgeUses->Item( j );
            EdgePtr _edge = edgeUse->Edge;

            if ( _edge->GetTag() == edge->GetTag() )
            {
                Q_ASSERT( _edge->GetID() == edge->GetID() );
                orientation = ( edgeUse->GetIsOpposedToEdge() == VARIANT_FALSE ? 1 : -1 ) *
                              ( _edge->GetIsParamReversed() == VARIANT_FALSE ? 1 : -1 );
                return true;
            }
        }
    }

    return true;
}

void ExtractEdgeOrientation( const FacePtr& face, const EdgePtr& edge, int& orientation )
{
    TraverseTopology( face, edge, orientation );
}

void GoldenSectionAllRoots( const EdgePtr& edge, const double* product, double a, double b,
                            const Point3D& intersectPointA, const Point3D& intersectPointB,
                            Points3DCollection& intersectionPointsCollection, double xTolerance, double yTolerance )
{
    Point3D rootPoint;
    double t, lastObjectiveValue;
    int result = GoldenSection( edge, product, &t, a, b, rootPoint, lastObjectiveValue, xTolerance );

    double realDistanceTolerance = 4. * yTolerance * yTolerance;

    if ( lastObjectiveValue > realDistanceTolerance )
        return;

    if ( rootPoint.IsClosedTo( intersectPointA, yTolerance ) ||
            rootPoint.IsClosedTo( intersectPointB, yTolerance ) )
    {
        Point3D _rootPoint( rootPoint );
        result = GoldenSectionMax( edge, product, &t, a, b, rootPoint, lastObjectiveValue, xTolerance );

        if ( result == MAXIMUM_EXISTS )
        {
            GoldenSectionAllRoots( edge, product, a, t, intersectPointA, rootPoint, intersectionPointsCollection,
                                   xTolerance, yTolerance );
            GoldenSectionAllRoots( edge, product, t, b, rootPoint, intersectPointB, intersectionPointsCollection,
                                   xTolerance, yTolerance );
        }
        else
        {
            Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                                               t = intersectionPointsCollection.end();

            for ( ; it != t; ++it )
                if ( it->IsClosedTo( _rootPoint, yTolerance ) )
                    break;

            if ( it == t )
                intersectionPointsCollection.push_back( _rootPoint );
        }

        return;
    }

    if ( result == MINIMUM_EXISTS )
    {
        intersectionPointsCollection.push_back( rootPoint );
        GoldenSectionAllRoots( edge, product, a, t, intersectPointA, rootPoint, intersectionPointsCollection,
                               xTolerance, yTolerance );
        GoldenSectionAllRoots( edge, product, t, b, rootPoint, intersectPointB, intersectionPointsCollection,
                               xTolerance, yTolerance );
    }
    else
    {
        if ( result == MINIMUM_ON_BOUNDARY /*&&
              !rootPoint.IsClosedTo(intersectPointA) &&
              !rootPoint.IsClosedTo(intersectPointB)*/ )
        {
            Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                                               t = intersectionPointsCollection.end();

            for ( ; it != t; ++it )
                if ( it->IsClosedTo( rootPoint, yTolerance ) )
                    break;

            if ( it == t )
                intersectionPointsCollection.push_back( rootPoint );
        }
    }
}

void EdgeCutPlaneIntersect( const EdgePtr& edge, Points3DCollection& intersectionPointsCollection,
                            const LoopBasePointInfo& loopBasePointInfo )
{
    int edgeId = edge->GetID();

    bool basePointOnEdge = loopBasePointInfo.adjEdges[loopBasePointInfo.index[0]].minmax == -1;

    if ( Utils::isEdgeALine( edge ) && !basePointOnEdge &&
            ( edgeId == loopBasePointInfo.adjEdges[0].edge->GetID() || edgeId == loopBasePointInfo.adjEdges[1].edge->GetID() ) )
        return;

    bool oneEdge = basePointOnEdge && loopBasePointInfo.adjEdges[1].edge->GetID() == -1 &&
                   edgeId == loopBasePointInfo.adjEdges[0].edge->GetID();

    double limits[2];
    double curveLength = 0.0;

    edge->GetParamExtents( &limits[0], &limits[1] );
    edge->GetLengthAtParam( limits[0], limits[1], &curveLength );

    Point3D normal( loopBasePointInfo.cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {loopBasePointInfo.cutPlane[0] / q, loopBasePointInfo.cutPlane[1] / q,
                         loopBasePointInfo.cutPlane[2] / q, loopBasePointInfo.cutPlane[3] / q
                        };
    double bound[3] = {limits[0],
                       oneEdge ? loopBasePointInfo.adjEdges[loopBasePointInfo.index[0]].tParamBasePoint : limits[1], limits[1]
                      };

    for ( int i = 0, n = oneEdge ? 2 : 1; i < n; i++ )
    {
        GoldenSectionAllRoots( edge, product, bound[i], bound[i + 1],
                               loopBasePointInfo.point, loopBasePointInfo.point,
                               intersectionPointsCollection, ( limits[1] - limits[0] ) * 1.e-2 * e_percent,
                               curveLength * 1.e-2 * e_percent );
    }
}

void EdgeCutPlaneIntersect( const EdgePtr& edge, Points3DCollection& intersectionPointsCollection,
                            const double* cutPlane )
{
    /*long vertex_count;
    SAFEARRAY* saPoints = NULL;
    SAFEARRAY* saParams = NULL;
    HRESULT hr = edge->GetStrokeData(0.001, &vertex_count, &saPoints, &saParams);
    Q_ASSERT(SUCCEEDED(hr));*/
    double curveLength = 0.;
    /*for (long i = 0, j = 0, k = 3, t = vertex_count - 1; i < t; i++)
    {
      double xi, xii, yi, yii, zi, zii;
      ::SafeArrayGetElement(saPoints, &j, &xi);
      ::SafeArrayGetElement(saPoints, &k, &xii);
      j++;k++;
      ::SafeArrayGetElement(saPoints, &j, &yi);
      ::SafeArrayGetElement(saPoints, &k, &yii);
      j++;k++;
      ::SafeArrayGetElement(saPoints, &j, &zi);
      ::SafeArrayGetElement(saPoints, &k, &zii);
      j++;k++;
      curveLength += Point3D(xi, yi, zi).DistanceTo(Point3D(xii, yii, zii));
    }
    ::SafeArrayDestroy(saPoints);
    ::SafeArrayDestroy(saParams);

    curveLength *= 1.e3;*/

    double limits[2];
    HRESULT hr = edge->GetParamExtents( &limits[0], &limits[1] );
    Q_ASSERT( SUCCEEDED( hr ) );

    hr = edge->GetLengthAtParam( limits[0], limits[1], &curveLength );
    Q_ASSERT( SUCCEEDED( hr ) );
    curveLength *= 1.e3;

    Vector3D normal( cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {cutPlane[0] / q, cutPlane[1] / q, cutPlane[2] / q, cutPlane[3] / q};

    Point3D point[2];
    GoldenSectionAllRoots( edge, product, limits[0], limits[1], point[0], point[1],
                           intersectionPointsCollection, ( limits[1] - limits[0] ) * 1.e-2 * e_percent,
                           curveLength * 1.e-2 * e_percent );
}

Point3D faceObjective( const FacePtr& face, double* uv )
{
    SAFEARRAYBOUND sabParameters;
    sabParameters.lLbound = 0;
    sabParameters.cElements = 2;

    SAFEARRAY* saParameters = ::SafeArrayCreate( VT_R8, 1, &sabParameters );
    long index = 0;
    ::SafeArrayPutElement( saParameters, &index, &uv[index] );
    ++index;
    ::SafeArrayPutElement( saParameters, &index, &uv[index] );

    SAFEARRAY* saCoordinates = NULL;
    HRESULT hr = face->GetPointAtParam( 1, &saParameters, &saCoordinates );
    ::SafeArrayDestroy( saParameters );

    if ( !SUCCEEDED( hr ) )
    {
        ::SafeArrayDestroy( saCoordinates );
        return Point3D();
    }

    Point3D point;
    Utils::getCoordinatesFromSafeArray( saCoordinates, point );
    ::SafeArrayDestroy( saCoordinates );
    return point;
}

double intersectObjective( const FacePtr& face, double* uv, const Point3D& middlePoint,
                           const Point3D& lineNormal, Point3D& lastPoint )
{
    Point3D surfacePoint( Utils::getPointOnFaceByParameter( face, uv ) );
    lastPoint = surfacePoint;
    return ( ( surfacePoint - middlePoint ) ^ lineNormal ).Length() / lineNormal.Length();
}

void EvaluateFacePlaneIntersectionPoint( const FacePtr& face, const double* cutPlane, Point3D& surfacePoint )
{
    Vector3D normal( cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {cutPlane[0] / q, cutPlane[1] / q, cutPlane[2] / q, cutPlane[3] / q};

    double limits[4];
    SAFEARRAY* minParams = NULL;
    SAFEARRAY* maxParams = NULL;
    HRESULT hr = face->GetParamRange( &minParams, &maxParams );
    Q_ASSERT( SUCCEEDED( hr ) );
    long param = 0;
    ::SafeArrayGetElement( minParams, &param, &limits[0] );
    ::SafeArrayGetElement( maxParams, &param, &limits[1] );
    ++param;
    ::SafeArrayGetElement( minParams, &param, &limits[2] );
    ::SafeArrayGetElement( maxParams, &param, &limits[3] );
    ::SafeArrayDestroy( minParams );
    ::SafeArrayDestroy( maxParams );

    double uv[2] = {( limits[0] + limits[1] ) / 2., ( limits[2] + limits[3] ) / 2.};

    for ( int i = 0, j = 0; i < 2; i++, j += 2 )
    {
        double lastObjectiveValue;
        int result = GoldenSection( face, uv, product, &uv[i], limits[j], limits[j + 1],
                                    lastObjectiveValue, surfacePoint, ( limits[j + 1] - limits[j] ) * 1.e-2 * e_percent );

        if ( result == MINIMUM_EXISTS )
            break;

        uv[i] = ( limits[j] + limits[j + 1] ) / 2.;
    }
}

double FacePlaneIntersectObjective( const FacePtr& face, const double* uv, const double* product,
                                    Point3D& surfacePoint )
{
    SAFEARRAYBOUND sabParams;
    sabParams.lLbound = 0;
    sabParams.cElements = 2;
    SAFEARRAY* saParams = ::SafeArrayCreate( VT_R8, 1, &sabParams );
    Q_ASSERT( saParams != 0x0 );
    long index = 0;
    ::SafeArrayPutElement( saParams, &index, ( void* )( &( uv[index] ) ) );
    index++;
    ::SafeArrayPutElement( saParams, &index, ( void* )( &( uv[index] ) ) );
    SAFEARRAY* saPoint = 0x0;
    HRESULT hr = face->GetPointAtParam( 1, &saParams, &saPoint );
    Q_ASSERT( SUCCEEDED( hr ) );
    Utils::getCoordinatesFromSafeArray( saPoint, surfacePoint );
    ::SafeArrayDestroy( saParams );
    ::SafeArrayDestroy( saPoint );
    double r = surfacePoint[0] * product[0] + surfacePoint[1] * product[1] + surfacePoint[2] * product[2] + product[3];
    return r * r;
}

Point3D edgeObjective( const EdgePtr& edge, double parameter, Point3D& point )
{
    SAFEARRAYBOUND sabParameters;
    sabParameters.lLbound = 0;
    sabParameters.cElements = 1;
    SAFEARRAY* saParameter = ::SafeArrayCreate( VT_R8, 1, &sabParameters );
    long index = 0;
    ::SafeArrayPutElement( saParameter, &index, &parameter );

    SAFEARRAY* saCoordinates = NULL;
    HRESULT hr = edge->GetPointAtParam( 1, &saParameter, &saCoordinates );
    Q_ASSERT( SUCCEEDED( hr ) );
    Utils::getCoordinatesFromSafeArray( saCoordinates, point );
    ::SafeArrayDestroy( saParameter );
    ::SafeArrayDestroy( saCoordinates );
    return point;
}

double intersectObjective( const EdgePtr& edge, const double* product, const double* t, Point3D& point )
{
    Utils::getPointOnEdgeByParameter( edge, *t, point );
    double r = point[0] * product[0] + point[1] * product[1] + point[2] * product[2] + product[3];
    return r * r;
}

int GoldenSection( const FacePtr& face, const double* uv, const double* product, double* x, double a, double b,
                   double& lastValue, Point3D& surfacePoint, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( Utils::checkDifference( *x, inia, e ) || Utils::checkDifference( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int GoldenSection( const EdgePtr& edge, const double* product, double* x, double a, double b,
                   Point3D& lastPoint, double& lastValue, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;
    //e = (b - a) * 1.e-2 * e;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( edge, product, x, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( edge, product, x, lastPoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = intersectObjective( edge, product, x, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = intersectObjective( edge, product, x, lastPoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( Utils::checkDifference( *x, inia, e ) || Utils::checkDifference( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int GoldenSectionMax( const EdgePtr& edge, const double* product, double* x, double a, double b,
                      Point3D& lastPoint, double& lastValue, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;
    //e = (b - a) * 1.e-2 * e;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( edge, product, x, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( edge, product, x, lastPoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 > y1 )//maximum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = intersectObjective( edge, product, x, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = intersectObjective( edge, product, x, lastPoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( Utils::checkDifference( *x, inia, e ) || Utils::checkDifference( *x, inib, e ) )
        return MAXIMUM_ON_BOUNDARY;

    return MAXIMUM_EXISTS;
}

int GoldenSection( const EdgePtr& edge, double* t, double a, double b, int xyzIndex, Point3D& point,
                   double rootTolerance, double boundaryTolerance )
{
    double x1, x2, inia = a, inib = b, deriv = 0.0;

    *t = x1 = b + ( b - a ) * tau;
    Utils::getPointOnEdgeByParameter( edge, *t, point );
    double y1 = point[xyzIndex];
    *t = x2 = a + ( a - b ) * tau;
    Utils::getPointOnEdgeByParameter( edge, *t, point );
    double y2 = point[xyzIndex];

    do
    {
        deriv += ::fabs( y2 - y1 );

        if ( y2 < y1 ) //minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *t = x2 = a + ( a - b ) * tau;
            Utils::getPointOnEdgeByParameter( edge, *t, point );
            y2 = point[xyzIndex];
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *t = x1 = b + ( b - a ) * tau;
            Utils::getPointOnEdgeByParameter( edge, *t, point );
            y1 = point[xyzIndex];
        }
    } while ( ( b - a ) > rootTolerance );

    *t = ( b + a ) * 0.5;

    // Check curve is on plane so edgeObjective() doesn't change
    if ( deriv < e_absolute )
        return CURVE_ON_PLANE;

    // Check root is on boundaries
    if ( Utils::checkDifference( *t, inia, boundaryTolerance ) || Utils::checkDifference( *t, inib, boundaryTolerance ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

double GoldenSection( const FacePtr& face, double* uv, double* x, double a, double b, int xyzIndex )
{
    double x1, x2, last;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = Utils::getPointOnFaceByParameter( face, uv )[xyzIndex];
    *x = x2 = a + ( a - b ) * tau;
    double y2 = Utils::getPointOnFaceByParameter( face, uv )[xyzIndex];

    do
    {
        if ( y2 < y1 ) //minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            last = y2 = Utils::getPointOnFaceByParameter( face, uv )[xyzIndex];
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            last = y1 = Utils::getPointOnFaceByParameter( face, uv )[xyzIndex];
        }
    } while ( ( b - a ) > e_absolute );

    *x = ( b + a ) * 0.5;

    return last;
}

int GoldenSection( const FacePtr& face, double* X, double* x, double a, double b, const Point3D& middlePoint,
                   const Point3D& lineNormal, Point3D& lastPoint, double& last, bool min )
{
    double x1, x2, e = ( b - a ) * 1.e-2 * e_percent, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );

    int iterLimit = 100;

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( min ? y2 < y1 : y2 > y1 )
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            last = y2 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            last = y1 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
        }
    } while ( ( b - a ) > e && iterLimit-- > 0 );

    *x = ( b + a ) * 0.5;

    // Check line intersect surface so intersectObjective() is variable
    if ( integr < 1.e-6 )
        return NO_EXTREMUM_EXISTS;

    // Check root is on boundaries
    if ( Utils::checkDifference( *x, inia, e ) || Utils::checkDifference( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int Evaluate_t_Snap( const EdgePtr& edge, double* X, double* limits, Point3D& point, bool )
{
    // TODO: find all minimums
    int result;

    for ( int xyzEnum = 0; xyzEnum < 3; xyzEnum++ )
    {
        result = GoldenSection( edge, X, limits[0], limits[1], xyzEnum, point,
                                ( limits[1] - limits[0] ) * 1.e-2 * root_tolerance_percent,
                                ( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent );

        // Check curve is on plane so edgeObjective() doesn't change
        if ( result == CURVE_ON_PLANE )
            continue;

        break;
    }

    return result;
}

void EvaluateUVSnap( const FacePtr& face, double* uv, double* limits, bool periodic )
{
    uv[0] = ( limits[0] + limits[1] ) * 0.5;
    uv[1] = ( limits[2] + limits[3] ) * 0.5;

    for ( int xyzEnum = 0; xyzEnum < 3; xyzEnum++ )
    {
        double first;

        for ( size_t i = 0, j = 0; i < 2; i++, j += 2 )
            first = GoldenSection( face, uv, &uv[i], limits[j], limits[j + 1], xyzEnum );

        for ( ;; )
        {
            double last;

            for ( int i = 0, j = 0; i < 2; i++, j += 2 )
                last = GoldenSection( face, uv, &uv[i], limits[j], limits[j + 1], xyzEnum );

            if ( Utils::checkDifference( first, last, e_absolute ) )
                break;

            first = last;
        }

        //Check root on boundaries
        if ( Utils::checkDifference( uv[0], limits[0], e_absolute ) && Utils::checkDifference( uv[1], limits[2], e_absolute ) ||
                Utils::checkDifference( uv[0], limits[1], e_absolute ) && Utils::checkDifference( uv[1], limits[3], e_absolute ) )
        {
            if ( periodic )
                break;
        }
        else
            break;
    }
}

void FindClosedIntersectionPoint( const Point3D& basePoint, const Points3DCollection& intersectionPointsCollection,
                                  Points3DCollection::const_iterator& closedPoint )
{
    // TODO: REMAKE THIS APROACH. FIND INTERSECT CURVE ROUGHLY AND THAN TAKE ITS END POINT
    double rmin = DBL_MAX;

    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            t = intersectionPointsCollection.end(); i != t; ++i )
    {
        double r = basePoint.DistanceTo( *i );

        if ( r < rmin )
        {
            closedPoint = i;
            rmin = r;
        }
    }
}

void FindClosedIntersectionPoint( const Point3D& basePoint, Vector3D dir,
                                  const Points3DCollection& intersectionPointsCollection,
                                  Points3DCollection::const_iterator& closedPoint )
{
    closedPoint = intersectionPointsCollection.end();
    double rmin = std::numeric_limits<double>::max();

    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            t = intersectionPointsCollection.end(); i != t; ++i )
    {
        Vector3D v = ( *i - basePoint );
        v.Norm();

        if ( !v.IsClosedTo2( dir ) )
            continue;

        double r = basePoint.DistanceTo( *i );

        if ( r < rmin )
        {
            closedPoint = i;
            rmin = r;
        }
    }
}

void FindAllLineFaceIntersections( const FacePtr& face, const Point3D& middlePoint,
                                   const Point3D& lineNormal, Points3DCollection& intersectionPointsCollection,
                                   double* limits, double* toplimits, double* etop, bool* periodic )
{
    double X[2] = {( limits[0] + limits[1] ) * 0.5, ( limits[2] + limits[3] ) * 0.5};

    Point3D intersectionPoint;
    double first;
    int result;

    for ( int i = 0, j = 0; i < 2; i++, j += 2 )
    {
        result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], middlePoint, lineNormal, intersectionPoint,
                                first, true );

        if ( result == NO_EXTREMUM_EXISTS )
            return;
    }

    double last;

    for ( ;; )
    {
        for ( int i = 0, j = 0; i < 2; i++, j += 2 )
        {
            result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], middlePoint, lineNormal, intersectionPoint,
                                    last, true );

            if ( result == NO_EXTREMUM_EXISTS )
                return;
        }

        if ( Utils::checkDifference( first, last, e_absolute ) )
        {
            //      if (last > 5.e-2) //tolerance of intersection (distance between point on surface and intersection line)
            //TODO it really depends on first derivation of U (or V)
            //so the tolerance must be calculated dynamically
            //        return;

            break;
        }

        first = last;
    }

    double e[2] = {( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent,
                   ( limits[3] - limits[2] ) * 1.e-2 * boundary_tolerance_percent
                  };

    if ( ( ( Utils::checkDifference( X[0], limits[0], e[0] ) || Utils::checkDifference( X[0], limits[1], e[0] ) ) &&
            !( periodic[0] && ( Utils::checkDifference( X[0], toplimits[0], etop[0] ) ||
                                Utils::checkDifference( X[0], toplimits[1], etop[0] ) ) ) ) &&
            ( Utils::checkDifference( X[1], limits[2], e[1] ) || Utils::checkDifference( X[1], limits[3], e[1] ) ) &&
            !( periodic[1] && ( Utils::checkDifference( X[1], toplimits[2], etop[1] ) ||
                                Utils::checkDifference( X[1], toplimits[3], etop[1] ) ) ) )
    {
        X[0] = ( limits[0] + limits[1] ) * 0.5;
        X[1] = ( limits[2] + limits[3] ) * 0.5;

        for ( int i = 0, j = 0; i < 2; i++, j += 2 )
        {
            result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], middlePoint, lineNormal, intersectionPoint,
                                    first, false );

            if ( result == NO_EXTREMUM_EXISTS )
                return;
        }

        double last;

        for ( ;; )
        {
            for ( int i = 0, j = 0; i < 2; i++, j += 2 )
            {
                result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], middlePoint, lineNormal,
                                        intersectionPoint, last, false );

                if ( result == NO_EXTREMUM_EXISTS )
                    return;
            }

            if ( Utils::checkDifference( first, last, e_absolute ) )
                break;

            first = last;
        }

        if ( ( Utils::checkDifference( X[0], limits[0], e[0] ) || Utils::checkDifference( X[0], limits[1], e[0] ) ) &&
                ( Utils::checkDifference( X[1], limits[2], e[1] ) || Utils::checkDifference( X[1], limits[3], e[1] ) ) )
            return;
    }
    else
    {
        if ( qFind( intersectionPointsCollection.begin(), intersectionPointsCollection.end(), intersectionPoint ) ==
                intersectionPointsCollection.end() && !( last > 5.e-2 ) )
            intersectionPointsCollection.push_back( intersectionPoint );

        if ( ( periodic[0] && ( Utils::checkDifference( X[0], toplimits[0], etop[0] ) ||
                                Utils::checkDifference( X[0], toplimits[1], etop[0] ) ) ) ||
                ( periodic[1] && ( Utils::checkDifference( X[1], toplimits[2], etop[1] ) ||
                                   Utils::checkDifference( X[1], toplimits[3], etop[1] ) ) ) )
            return;
    }

    double roi1[4] = { limits[0], X[0], limits[2], X[1] };
    double roi2[4] = { X[0], limits[1], X[1], limits[3] };
    double roi3[4] = { limits[0], X[0], X[1], limits[3] };
    double roi4[4] = { X[0], limits[1], limits[2], X[1] };

    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi1, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi2, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi3, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi4, toplimits, etop,
                                  periodic );
}

bool FindLineFaceIntersect( const FacePtr& face, const Point3D& lineNormal, const Point3D& middlePoint,
                            Point3D& intersectionPoint )
{
    SAFEARRAY* periodicityU = NULL;
    SAFEARRAY* periodicityV = NULL;
    long endSingularityU = 0.0;
    SAFEARRAY* singularityU = NULL;
    long endSingularityV = 0.0;
    SAFEARRAY* singularityV = NULL;
    HRESULT hr = face->GetParamAnomaly( &periodicityU, &periodicityV, &endSingularityU, &singularityU, &endSingularityV,
                                        &singularityV );
    Q_ASSERT( SUCCEEDED( hr ) );

    long elementNum = 0;
    double periodU = 0.0, periodV = 0.0;

    ::SafeArrayGetElement( periodicityU, &elementNum, &periodU );
    ::SafeArrayGetElement( periodicityV, &elementNum, &periodV );

    ::SafeArrayDestroy( periodicityU );
    ::SafeArrayDestroy( periodicityV );
    ::SafeArrayDestroy( singularityU );
    ::SafeArrayDestroy( singularityV );

    bool periodic[2] = { !Utils::checkDifference( periodU, 0.0, e_absolute ), !Utils::checkDifference( periodV, 0.0, e_absolute ) };

    double limits[4];
    SAFEARRAY* minParams = NULL;
    SAFEARRAY* maxParams = NULL;

    hr = face->GetParamRange( &minParams, &maxParams );
    Q_ASSERT( SUCCEEDED( hr ) );

    long param = 0;
    ::SafeArrayGetElement( minParams, &param, &limits[0] );
    ::SafeArrayGetElement( maxParams, &param, &limits[1] );
    ++param;
    ::SafeArrayGetElement( minParams, &param, &limits[2] );
    ::SafeArrayGetElement( maxParams, &param, &limits[3] );

    ::SafeArrayDestroy( minParams );
    ::SafeArrayDestroy( maxParams );

    Points3DCollection intersectionPointsCollection;
    double e[2] = {( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent,
                   ( limits[3] - limits[2] ) * 1.e-2 * boundary_tolerance_percent
                  };
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, limits, limits, e,
                                  periodic );

    if ( !intersectionPointsCollection.empty() )
    {
        Points3DCollection::const_iterator closestPoint;
        FindClosedIntersectionPoint( middlePoint, intersectionPointsCollection, closestPoint );
        intersectionPoint = *closestPoint;
        return true;
    }
    else
        return false;
}

void EvaluateEdgeSnapPoint( const EdgePtr& edge, Point3D& point )
{
    SAFEARRAY* saPeriodicity = NULL;
    VARIANT_BOOL singular = VARIANT_FALSE;
    edge->GetParamAnomaly( &saPeriodicity, &singular );
    ::SafeArrayDestroy( saPeriodicity );

    double limits[2];
    edge->GetParamExtents( &limits[0], &limits[1] );

    if ( singular )
    {
        double tParamOnCurve;
        Evaluate_t_Snap( edge, &tParamOnCurve, limits, point, singular );
    }
    else
        Utils::getPointOnEdgeByParameter( edge, ( limits[0] + limits[1] ) * 0.5, point );
}

int EvaluateFaceSnapPoint( const FacePtr& face, Point3D& point )
{
    int code = 0;
    LoopsPtr loopsCollection = face->Loops;
    EdgesPtr edgesCollection = face->Edges;
    int i = 1;

    for ( ; i <= loopsCollection->Count; i++ )
    {
        LoopPtr loop = loopsCollection->Item( i );

        if ( loop->IsOuterLoop )
            break;
    }

    LoopBasePointInfo loopBasePointInfo;

    LoopPtr outerLoop = loopsCollection->Item( i );

    if ( outerLoop && outerLoop->Edges )
    {
        if ( code = EvaluateLoopBasePoint( outerLoop, loopBasePointInfo ) )
            return code;
    }
    else
    {
        if ( edgesCollection->Count == 0 )
        {
            double limits[4];
            SAFEARRAY* saMinParams = NULL;
            SAFEARRAY* saMaxParams = NULL;
            face->GetParamRange( &saMinParams, &saMaxParams );
            long indexOfElement = 0;
            ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[0] );
            ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[1] );
            ++indexOfElement;
            ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[2] );
            ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[3] );
            ::SafeArrayDestroy( saMinParams );
            ::SafeArrayDestroy( saMaxParams );

            double uv[2];
            EvaluateUVSnap( face, uv, limits, true );

            point = Utils::getPointOnFaceByParameter( face, uv );
            return code;
        }
        else
        {
            for ( int j = 1; j <= loopsCollection->Count; ++j )
            {
                LoopPtr loop = loopsCollection->Item( j );
                LoopBasePointInfo prevLoopBasePointInfo;

                if ( code = EvaluateLoopBasePoint( loop, prevLoopBasePointInfo ) )
                    return code;

                if ( prevLoopBasePointInfo.point < loopBasePointInfo.point )
                    loopBasePointInfo = prevLoopBasePointInfo;
            }
        }
    }

    Points3DCollection intersectionPointsCollection;

    for ( int j = 1; j < edgesCollection->Count; ++j )
    {
        EdgePtr edge = edgesCollection->Item( j );
        EdgeCutPlaneIntersect( edge, intersectionPointsCollection, loopBasePointInfo );
    }

    Point3D middlePoint, lineNormal;

    Q_ASSERT( intersectionPointsCollection.size() != 0 );

    while ( !intersectionPointsCollection.empty() )
    {
        Points3DCollection::const_iterator closedPointIterator;
        FindClosedIntersectionPoint( loopBasePointInfo.point, intersectionPointsCollection, closedPointIterator );

        Point3D closedPoint( *closedPointIterator );
        middlePoint = ( closedPoint + loopBasePointInfo.point ) * 0.5;
        lineNormal = ( closedPoint - loopBasePointInfo.point ) ^ Point3D( loopBasePointInfo.cutPlane );

        if ( FindLineFaceIntersect( face, lineNormal, middlePoint, point ) )
            break;
        else
            intersectionPointsCollection.erase( closedPointIterator );
    }

    if ( intersectionPointsCollection.empty() )
        return ClosedIntersectionPointNotFound;

    return code;
}

void AddOnlyUniquePoint( Points3DCollection& intersectionPointsCollection,
                         Points3DCollection& intersectionPointsCollectionCurrent, double e )
{
    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            ti = intersectionPointsCollection.end(); i != ti; ++i )
    {
        for ( Points3DCollection::const_iterator j = intersectionPointsCollectionCurrent.begin(),
                tj = intersectionPointsCollectionCurrent.end(); j != tj; ++j )
        {
            if ( i->IsClosedTo( *j, e ) )
            {
                intersectionPointsCollectionCurrent.erase( j );
                break;
            }
        }
    }

    intersectionPointsCollection.insert( intersectionPointsCollection.end(),
                                         intersectionPointsCollectionCurrent.begin(),
                                         intersectionPointsCollectionCurrent.end() );
}

int EvaluateFaceSnapPoint3( const FacePtr& face, Point3D& point )
{
    PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
    QFileInfo fileInfo( QString( partDocument->FullName ) );
    QString path = QDir::toNativeSeparators( fileInfo.absolutePath() );
    QString name = fileInfo.baseName();

    int code = 0;
    EdgesPtr edgesList = face->GetEdges();

    if ( edgesList && edgesList->Count )
    {
        Point3D boundBox[2];
        SAFEARRAY* saMinPoint = 0x0;
        SAFEARRAY* saMaxPoint = 0x0;
        HRESULT hr = face->GetRange( &saMinPoint, &saMaxPoint );
        Q_ASSERT( SUCCEEDED( hr ) );
        Utils::getCoordinatesFromSafeArray( saMinPoint, boundBox[0] );
        Utils::getCoordinatesFromSafeArray( saMaxPoint, boundBox[1] );
        ::SafeArrayDestroy( saMinPoint );
        ::SafeArrayDestroy( saMaxPoint );
        double boxSize[3] = {boundBox[1].x - boundBox[0].x,
                             boundBox[1].y - boundBox[0].y,
                             boundBox[1].z - boundBox[0].z
                            };
        Point3D boxCenter( ( boundBox[0] + boundBox[1] ) * .5 );
        double* pos = std::max_element( boxSize, boxSize + 3 );
        int boxMaxSideIndex = pos - boxSize;
        Vector3D cutPlaneNormal( 0.0 );
        cutPlaneNormal[boxMaxSideIndex] = boxSize[boxMaxSideIndex];
        double e = boxSize[boxMaxSideIndex] * 1.e-5;
        double cutPlane[4] = {cutPlaneNormal.x, cutPlaneNormal.y, cutPlaneNormal.z, boxCenter* cutPlaneNormal * -1.0};
        Utils::writeVectorToFile( path + QDir::separator() + "cutPlane." + name, boxCenter, Vector3D( cutPlane ) );
        //Utils::writeFaceToFile("face",face);
        //Utils::writeFaceTopology("topology",face);
        Points3DCollection intersectionPointsCollection;
        Vector3D tangent;
        int i = 1;

        for ( ; i <= edgesList->Count; i++ )
        {
            EdgePtr edge = edgesList->Item( i );
            EdgeCutPlaneIntersect( edge, intersectionPointsCollection, cutPlane );

            if ( intersectionPointsCollection.size() > 0 )
            {
                SAFEARRAY* saPoints = Utils::createSafeArrayForPoint( intersectionPointsCollection[0] );
                Q_ASSERT( saPoints != 0x0 );
                SAFEARRAY* saGuessParams = 0x0;
                SAFEARRAY* saMaxDeviations = 0x0;
                SAFEARRAY* saParams = 0x0;
                SAFEARRAY* saFlags = 0x0;
                hr = edge->GetParamAtPoint( 1, &saPoints, &saGuessParams, &saMaxDeviations, &saParams, &saFlags );
                Q_ASSERT( SUCCEEDED( hr ) );
                ::SafeArrayDestroy( saPoints );
                ::SafeArrayDestroy( saGuessParams );
                ::SafeArrayDestroy( saMaxDeviations );
                ::SafeArrayDestroy( saFlags );
                SAFEARRAY* saTangent = 0x0;
                hr = edge->GetTangent( 1, &saParams, &saTangent );
                Q_ASSERT( SUCCEEDED( hr ) );
                ::SafeArrayDestroy( saParams );
                Utils::getCoordinatesFromSafeArray( saTangent, tangent );
                int orientation;
                ExtractEdgeOrientation( face, edge, orientation );
                tangent *= ( double )orientation;
                Utils::writeVectorToFile( path + QDir::separator() + "tangent." + name, intersectionPointsCollection[0], tangent );
                i++;
                break;
            }
        }

        for ( ; i <= edgesList->Count; i++ )
        {
            EdgePtr edge = edgesList->Item( i );
            Points3DCollection intersectionPointsCollectionCurrent;
            EdgeCutPlaneIntersect( edge, intersectionPointsCollectionCurrent, cutPlane );
            AddOnlyUniquePoint( intersectionPointsCollection, intersectionPointsCollectionCurrent, e ); // TODO: Add copy function
        }

        Utils::writePointsToFile( path + QDir::separator() + "intersections." + name, intersectionPointsCollection );

        if ( intersectionPointsCollection.empty() )
            EvaluateFacePlaneIntersectionPoint( face, cutPlane, point );
        else
        {
            {
                if ( intersectionPointsCollection.size() == 1 )
                    return 3;

                Point3D first = intersectionPointsCollection[0];
                Point3D facePoint = first;
                intersectionPointsCollection.erase( intersectionPointsCollection.begin() );

                Points3DCollection::const_iterator closedPointIterator = intersectionPointsCollection.end();

                if ( intersectionPointsCollection.size() > 1 )
                {
                    double uv[2];
                    //GetClosestUVPointOnSurface(face,facePoint,uv);

                    /*SAFEARRAY* saMin = 0x0;
                    SAFEARRAY* saMax = 0x0;
                    hr = face->GetParamRange(&saMin,&saMax);
                    Q_ASSERT(SUCCEEDED(hr));
                    double lim[2][2];
                    for (long i = 0; i < 2; i++)
                      ::SafeArrayGetElement(saMin, &i, &lim[i][0]);
                    for (long i = 0; i < 2; i++)
                      ::SafeArrayGetElement(saMax, &i, &lim[i][1]);*/
                    SAFEARRAY* saPoints = Utils::createSafeArrayForPoint( facePoint );
                    SAFEARRAY* saParams = 0x0;
                    SAFEARRAY* saGuessParam = 0x0;
                    /*SAFEARRAYBOUND sabCoordinates;
                    sabCoordinates.lLbound = 0;
                    sabCoordinates.cElements = 1;
                    SAFEARRAY* saMaxDeviation = ::SafeArrayCreate(VT_R8, 1, &sabCoordinates);
                    long indexOfElement = 0;
                    double x = 1.e-1;
                    ::SafeArrayPutElement(saMaxDeviation, &indexOfElement, &x);*/
                    SAFEARRAY* saMaxDeviation = 0x0;
                    SAFEARRAY* saFlags = 0x0;

                    hr = face->GetParamAtPoint( 1, &saPoints, &saGuessParam, &saMaxDeviation, &saParams, &saFlags );
                    Q_ASSERT( SUCCEEDED( hr ) );

                    for ( long i = 0; i < 2; i++ )
                        ::SafeArrayGetElement( saParams, &i, &uv[i] );

                    /*SAFEARRAY* saCoordinates = NULL;
                    hr = face->GetPointAtParam(1, &saParams, &saCoordinates);
                    Q_ASSERT(SUCCEEDED(hr));
                    Point3D ptest;
                    Utils::getCoordinatesFromSafeArray(saCoordinates, ptest);
                    ::SafeArrayDestroy(saCoordinates);*/

                    ::SafeArrayDestroy( saPoints );
                    ::SafeArrayDestroy( saGuessParam );
                    ::SafeArrayDestroy( saMaxDeviation );
                    //::SafeArrayDestroy(saParams);
                    ::SafeArrayDestroy( saFlags );

                    Vector3D normalAtStartPoint, duv[2];
                    //SAFEARRAYBOUND sabParams;
                    //sabParams.lLbound = 0;
                    //sabParams.cElements = 2;
                    //SAFEARRAY* saParams = ::SafeArrayCreate(VT_R8, 1, &sabParams);
                    //Q_ASSERT(saParams != 0x0);
                    long index = 0;
                    //::SafeArrayPutElement(saParams, &index, &uv[index]);
                    //++index;
                    //::SafeArrayPutElement(saParams, &index, &uv[index]);
                    //SAFEARRAYBOUND sabParams;
                    //sabParams.lLbound = 0;
                    //sabParams.cElements = 3;
                    //SAFEARRAY* saUTangents = ::SafeArrayCreate(VT_R8, 1, &sabParams);
                    //SAFEARRAY* saVTangents = ::SafeArrayCreate(VT_R8, 1, &sabParams);
                    /*SAFEARRAY* saUTangents = 0x0;
                    SAFEARRAY* saVTangents = 0x0;
                    SAFEARRAY* saD[7] = {0x0,};*/
                    /*double x = 0.;
                    ::SafeArrayPutElement(saParams, &index, &x);
                    index++;
                    ::SafeArrayPutElement(saParams, &index, &x);*/
                    /*hr = face->GetTangents(1, &saParams, &saUTangents, &saVTangents);
                    Q_ASSERT(SUCCEEDED(hr));*/
                    //Utils::getCoordinatesFromSafeArray(saUTangents, duv[0]);
                    //Utils::getCoordinatesFromSafeArray(saVTangents, duv[1]);
                    //double v;
                    //index=0;
                    //hr = ::SafeArrayGetElement(saUTangents, &index, &v);
                    //Q_ASSERT(SUCCEEDED(hr));
                    //hr = ::SafeArrayGetElement(saVTangents, &index, &v);
                    //Q_ASSERT(SUCCEEDED(hr));

                    SAFEARRAY* saNormal = 0x0;
                    hr = face->GetNormal( 1, &saParams, &saNormal );
                    Q_ASSERT( SUCCEEDED( hr ) );
                    Utils::getCoordinatesFromSafeArray( saNormal, normalAtStartPoint );
                    ::SafeArrayDestroy( saNormal );
                    Utils::writeVectorToFile( path + QDir::separator() + "normal." + name, facePoint, normalAtStartPoint );
                    /*hr = face->GetDerivatives(1, &saParams, &saD[0], &saD[1], &saD[2], &saD[3], &saD[4], &saD[5], &saD[6]);
                    Q_ASSERT(SUCCEEDED(hr));*/
                    //hr = face->GetCurvatures(1, &saParams, &saD[1], &saD[2], &saD[3]);
                    //Q_ASSERT(SUCCEEDED(hr));


                    //Utils::getCoordinatesFromSafeArray(saUTangents, duv[0]);
                    //Utils::getCoordinatesFromSafeArray(saVTangents, duv[1]);
                    /*::SafeArrayDestroy(saParams);
                    ::SafeArrayDestroy(saUTangents);
                    ::SafeArrayDestroy(saVTangents);*/

                    Utils::getFaceFirstPartialDerivative( face, uv, duv[0], duv[1] );
                    Utils::writeVectorToFile( path + QDir::separator() + "du." + name, facePoint, duv[0] );
                    Utils::writeVectorToFile( path + QDir::separator() + "dv." + name, facePoint, duv[1] );

                    double duvm[2] = {duv[0].Length(), duv[1].Length()};

                    Vector3D t = normalAtStartPoint ^ Vector3D( cutPlane ) * ( Vector3D( cutPlane ) * tangent );
                    Utils::writeVectorToFile( path + QDir::separator() + "t." + name, facePoint, t );
                    t.Norm();

                    double tuv[2] = {duv[0]* t, duv[1]* t};

                    double limits[4];
                    SAFEARRAY* saMinParams = NULL;
                    SAFEARRAY* saMaxParams = NULL;
                    face->GetParamRange( &saMinParams, &saMaxParams );
                    long indexOfElement = 0;
                    ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[0] );
                    ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[1] );
                    ++indexOfElement;
                    ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[2] );
                    ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[3] );
                    ::SafeArrayDestroy( saMinParams );
                    ::SafeArrayDestroy( saMaxParams );

                    //int i/*, j*/;
                    double UV_period[2];
                    SAFEARRAY* saPeriodicityU = 0x0;
                    SAFEARRAY* saPeriodicityV = 0x0;
                    long endSingularityU = 0;
                    long endSingularityV = 0;
                    SAFEARRAY* saSingularityU = 0x0;
                    SAFEARRAY* saSingularityV = 0x0;
                    HRESULT hr = face->GetParamAnomaly( &saPeriodicityU, &saPeriodicityV, &endSingularityU, &saSingularityU,
                                                        &endSingularityV, &saSingularityV );
                    Q_ASSERT( SUCCEEDED( hr ) );
                    index = 0;
                    ::SafeArrayGetElement( saPeriodicityU, &index, &UV_period[0] );
                    ::SafeArrayGetElement( saPeriodicityV, &index, &UV_period[1] );
                    ::SafeArrayDestroy( saPeriodicityU );
                    ::SafeArrayDestroy( saPeriodicityV );
                    ::SafeArrayDestroy( saSingularityU );
                    ::SafeArrayDestroy( saSingularityV );

                    bool uvperiodic[2] = {( UV_period[0] > 0. ) ? true : false, ( UV_period[1] > 0. ) ? true : false};

                    if ( uvperiodic[0] )
                    {
                        if ( limits[0] > uv[0] && !( ::fabs( ( limits[0] - uv[0] ) / limits[0] ) < e_percent ) )
                            uv[0] += UV_period[0];
                        else if ( limits[1] < uv[0] && !( ::fabs( ( limits[1] - uv[0] ) / limits[1] ) < e_percent ) )
                            uv[0] -= UV_period[0];
                    }

                    if ( uvperiodic[1] )
                    {
                        if ( limits[2] > uv[1] && !( ::fabs( ( limits[2] - uv[1] ) / limits[2] ) < e_percent ) )
                            uv[1] += UV_period[1];
                        else if ( limits[3] < uv[1] && !( ::fabs( ( limits[3] - uv[1] ) / limits[3] ) < e_percent ) )
                            uv[1] -= UV_period[1];
                    }

                    double rmin = std::numeric_limits<double>::max();

                    for ( Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                            t = intersectionPointsCollection.end(); it != t; ++it )
                    {
                        double _uv[2];
                        //GetClosestUVPointOnSurface(face, *it, _uv);

                        SAFEARRAY* saPoints = Utils::createSafeArrayForPoint( *it );
                        SAFEARRAY* saParams = 0x0;
                        SAFEARRAY* saGuessParam = 0x0;
                        SAFEARRAY* saMaxDeviation = 0x0;
                        SAFEARRAY* saFlags = 0x0;

                        hr = face->GetParamAtPoint( 1, &saPoints, &saGuessParam, &saMaxDeviation, &saParams, &saFlags );
                        Q_ASSERT( SUCCEEDED( hr ) );

                        for ( long ii = 0; ii < 2; ii++ )
                            ::SafeArrayGetElement( saParams, &ii, &_uv[ii] );

                        ::SafeArrayDestroy( saParams );
                        ::SafeArrayDestroy( saPoints );
                        ::SafeArrayDestroy( saGuessParam );
                        ::SafeArrayDestroy( saMaxDeviation );

                        if ( uvperiodic[0] )
                        {
                            if ( limits[0] > _uv[0] && !( ::fabs( ( limits[0] - _uv[0] ) / limits[0] ) < e_percent ) )
                                _uv[0] += UV_period[0];
                            else if ( limits[1] < _uv[0] && !( ::fabs( ( limits[1] - _uv[0] ) / limits[1] ) < e_percent ) )
                                _uv[0] -= UV_period[0];
                        }

                        if ( uvperiodic[1] )
                        {
                            if ( limits[2] > _uv[1] && !( ::fabs( ( limits[2] - _uv[1] ) / limits[2] ) < e_percent ) )
                                _uv[1] += UV_period[1];
                            else if ( limits[3] < _uv[1] && !( ::fabs( ( limits[3] - _uv[1] ) / limits[3] ) < e_percent ) )
                                _uv[1] -= UV_period[1];
                        }

                        double ruv[2] = {( _uv[0] - uv[0] )* duvm[0], ( _uv[1] - uv[1] )* duvm[1]};

                        double sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                        double d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                        if ( sign > 0. && d < rmin )
                        {
                            closedPointIterator = it;
                            rmin = d;
                        }

                        double u = _uv[0];

                        for ( int i = 0; i < 2; i++ )
                        {
                            if ( uvperiodic[i] )
                            {
                                _uv[i] = ( ruv[i] >= 0. ) ? ( _uv[i] - UV_period[i] ) : ( _uv[i] + UV_period[i] );

                                ruv[0] = ( _uv[0] - uv[0] ) * duvm[0];
                                ruv[1] = ( _uv[1] - uv[1] ) * duvm[1];

                                sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                                d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                                if ( sign > 0. && d < rmin )
                                {
                                    closedPointIterator = it;
                                    rmin = d;
                                }
                            }
                        }

                        if ( uvperiodic[0] && uvperiodic[1] )
                        {
                            _uv[0] = u;

                            ruv[0] = ( _uv[0] - uv[0] ) * duvm[0];
                            ruv[1] = ( _uv[1] - uv[1] ) * duvm[1];

                            sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                            d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                            if ( sign > 0. && d < rmin )
                            {
                                closedPointIterator = it;
                                rmin = d;
                            }
                        }
                    }
                }
                else
                    closedPointIterator = intersectionPointsCollection.begin();

                if ( closedPointIterator == intersectionPointsCollection.end() )
                    return 1;

                Point3D closedPoint( *closedPointIterator );

                Point3D middlePoint = ( closedPoint + first ) * 0.5;
                Vector3D lineNormal = ( closedPoint - first ) ^ Vector3D( cutPlane );
                Utils::writeVectorToFile( path + QDir::separator() + "awl." + name, middlePoint, lineNormal );

                if ( !FindLineFaceIntersect( face, lineNormal, middlePoint, point ) )
                    return 2;
            }
        }
    }
    else
    {
        double limits[4];
        SAFEARRAY* saMinParams = NULL;
        SAFEARRAY* saMaxParams = NULL;
        face->GetParamRange( &saMinParams, &saMaxParams );
        long indexOfElement = 0;
        ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[0] );
        ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[1] );
        ++indexOfElement;
        ::SafeArrayGetElement( saMinParams, &indexOfElement, &limits[2] );
        ::SafeArrayGetElement( saMaxParams, &indexOfElement, &limits[3] );
        ::SafeArrayDestroy( saMinParams );
        ::SafeArrayDestroy( saMaxParams );

        double uv[2] = {( limits[0] + limits[1] ) / 2., ( limits[2] + limits[3] ) / 2.};

//    double p[3],r[2];
//    UF_CALL(::UF_MODL_ask_face_props(face,uv,(double*)&point,p,p,p,p,p,r)); // WARNING: ???

        //CreateDatumPointFeature(QString("SNAPFACE%1").arg(tagFace),point);
        SAFEARRAYBOUND sabParams;
        sabParams.lLbound = 0;
        sabParams.cElements = 2;
        SAFEARRAY* saParams = ::SafeArrayCreate( VT_R8, 1, &sabParams );
        Q_ASSERT( saParams != 0x0 );
        long index = 0;
        ::SafeArrayPutElement( saParams, &index, &uv[index] );
        ++index;
        ::SafeArrayPutElement( saParams, &index, &uv[index] );
        SAFEARRAY* saCoordinates = 0x0;
        HRESULT hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
        Q_ASSERT( SUCCEEDED( hr ) );
        Utils::getCoordinatesFromSafeArray( saCoordinates, point );
        ::SafeArrayDestroy( saCoordinates );
    }

    return code;
}

/*int EvaluateFaceSnapPoint2(const FacePtr& face, Point3D& point)
{
//  void H_test();
  //H_test();

  int code = 0;
  LoopsPtr loops = face->Loops;
  EdgesPtr edges = face->Edges;

  LoopPtr loop = 0x0;
  for (long i = 1; i < loops->Count; i++)
  {
    loop = loops->Item(i);
    if ( loop->type == 1 ) // WARNING: LoopPtr->IsOuter?
      break;
  }

  double bb[6];
  UF_CALL(::UF_MODL_ask_bounding_box(face,bb)); // WARNING: ???
  double defeatureEdgeLength = 1.e-3 * Point3D(bb).DistanceTo(Point3D(&bb[3]));
  double minEdgeFaceLength = std::numeric_limits<double>::max();
  for (long i = 1; i < edges->Count; i++)
  {
    EdgePtr edge = edges->Item(i);
    int vertex_count;
    double (*points)[3];
    UF_CALL(::UF_MODL_ask_curve_points(edge->eid, 0, 0, 0, &vertex_count, (double**)&points)); // WARNING: EdgePtr::GetStrokeData()?

    double edgeLength = 0.;
    for (int i = 0, t = vertex_count - 1; i < t; i++)
      edgeLength += Point3D((double*)points[i]).DistanceTo(Point3D((double*)points[i+1]));

    if (edgeLength > defeatureEdgeLength && edgeLength < minEdgeFaceLength)
      minEdgeFaceLength = edgeLength;
  }

  Q_ASSERT(minEdgeFaceLength != std::numeric_limits<double>::max());

  double patch = minEdgeFaceLength * 0.05;

  Points3DCollection pointsOnOuterLoop;
  for (long i = 1; i < edges->Count; i++)
  {
    EdgePtr edge = edges->Item(i);
    //int edgeType;
    //UF_CALL(::UF_MODL_ask_edge_type(edge->eid, &edgeType));

    int vertex_count;
    double (*points)[3];
    UF_CALL(::UF_MODL_ask_curve_points(edge->eid, 1.e-3* patch, 0, patch, &vertex_count, (double**)&points)); // WARNING: EdgePtr::GetStrokeData()?

    for (int i = 0; i < vertex_count; i++)
    {
      pointsOnOuterLoop.push_back(Point3D((double*)points[i]));
      //CreateDatumPointFeature("",Point3D((double*)points[i]));
    }

    ::UF_free(points);
  }

  FacePointsCloud_H(face,pointsOnOuterLoop); // TODO: realize function

  return code;
}*/

double H( const Point3D& probe, const Points3DCollection& points )
{
#if 1
    Vector3D H( 0. );

    for ( Points3DCollection::const_iterator i = points.begin(), t = points.end(); i != t; ++i )
    {
        Vector3D r = probe - *i;
        H = H + r.Norm() * ( 1. / ( r * r ) );
    }

    return H.Length();
#else
    double s = 0.;

    for ( Points3DCollection::const_iterator i = points.begin(), t = points.end(); i != t; ++i )
    {
        Vector3D r = probe - *i;
        s += 1. / ( r * r );
    }

    return s;
#endif
}

void H_test()
{
    Points3DCollection c;

    c.push_back( Point3D( 3., 0, 0 ) );
    c.push_back( Point3D( 0, 3., 0 ) );
    c.push_back( Point3D( -3., 0, 0 ) );
    c.push_back( Point3D( 0, -3., 0 ) );
    c.push_back( Point3D( 0, 0, 3. ) );
    c.push_back( Point3D( 0, 0, -3. ) );

    if ( H( Point3D( 0. ), c ) != 0.0 )
        throw;
}

void SolidEdgeExtensionImpl::NamingTest()
{
    PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;

    if ( !partDocument )
        return;

    QFileInfo fileInfo( QString( partDocument->FullName ) );
    QString path = QDir::toNativeSeparators( fileInfo.absolutePath() );
    QString name = fileInfo.baseName();

    ModelsPtr modelsCollection = partDocument->Models;

    if ( !modelsCollection )
        return;

    ModelPtr model = modelsCollection->Item( 1 );

    if ( !model )
        return;

    BodyPtr body = model->Body;
    Point3D boundBox[2];
    SAFEARRAY* saMinPoint = 0x0;
    SAFEARRAY* saMaxPoint = 0x0;
    HRESULT hr = body->GetRange( &saMinPoint, &saMaxPoint );
    Q_ASSERT( SUCCEEDED( hr ) );
    Utils::getCoordinatesFromSafeArray( saMinPoint, boundBox[0] );
    Utils::getCoordinatesFromSafeArray( saMaxPoint, boundBox[1] );
    ::SafeArrayDestroy( saMinPoint );
    ::SafeArrayDestroy( saMaxPoint );
    Point3D::SetDistanceTolerance( ( boundBox[1] - boundBox[0] ).Length() * 1.e-6 );

    EdgesPtr edgesList = body->GetEdges( igQueryAll );
    std::set<Point3D> snapPointsCollection;

    QFile edgeVertecesSnapPointsFile( path + QDir::separator() + "edgeVertecesSnapPoints." + name + ".obj" );
    edgeVertecesSnapPointsFile.open( QFile::WriteOnly );
    QTextStream ts1( &edgeVertecesSnapPointsFile );
    int evsp = 0;

    if ( edgesList )
    {
        for ( int i = 1; i <= edgesList->Count; i++ )
        {
            EdgePtr edge = edgesList->Item( i );
            {
                SAFEARRAY* saStartPoint = NULL;
                SAFEARRAY* saEndPoint = NULL;
                //edge->GetEndPoints(&saStartPoint, &saEndPoint);
                VertexPtr svertex = edge->StartVertex;

                if ( svertex )
                {
                    HRESULT hr = svertex->GetPointData( &saStartPoint );
                    Q_ASSERT( SUCCEEDED( hr ) );
                    VertexPtr evertex = edge->EndVertex;
                    hr = evertex->GetPointData( &saEndPoint );
                    Q_ASSERT( SUCCEEDED( hr ) );
                    Point3D startPoint, endPoint;

                    Utils::getCoordinatesFromSafeArray( saStartPoint, startPoint );
                    Utils::getCoordinatesFromSafeArray( saEndPoint, endPoint );

                    int vertex_count = 2;
                    Point3D p[] = {startPoint, endPoint};

                    for ( int i = 0; i < vertex_count; i++ )
                    {
                        if ( snapPointsCollection.find( p[i] ) == snapPointsCollection.end() )
                        {
                            snapPointsCollection.insert( p[i] );
                            ts1 << QString( "v %1 %2 %3\n" ).arg( p[i].x ).arg( p[i].y ).arg( p[i].z );
                            evsp++;
                        }
                    }
                }

                if ( saStartPoint )
                    ::SafeArrayDestroy( saStartPoint );

                if ( saEndPoint )
                    ::SafeArrayDestroy( saEndPoint );
            }
        }
    }

    int k = 1;

    while ( k <= evsp )
    {
        ts1 << QString( "p %1\n" ).arg( k );
        k++;
    }

    ts1.flush();
    edgeVertecesSnapPointsFile.close();

    QFile edgeSnapPointsFile( path + QDir::separator() + "edgeSnapPoints." + name + ".obj" );
    edgeSnapPointsFile.open( QFile::WriteOnly );
    QTextStream ts2( &edgeSnapPointsFile );
    int esp = 0;

    if ( edgesList )
    {
        for ( int i = 1; i <= edgesList->Count; i++ )
        {
            EdgePtr edge = edgesList->Item( i );
            Point3D p;
            {
                EvaluateEdgeSnapPoint( edge, p );
                std::set<Point3D>::const_iterator closest;

                if ( ( closest = snapPointsCollection.find( p ) ) != snapPointsCollection.end() )
                {
                    QMessageBox::critical( GetTopWindow(), "Naming test",
                                           QString( "DOUBLICATE SNAP POINT EDGE ID %1" ).arg( edge->GetID() ) );
                    return;
                }

                snapPointsCollection.insert( p );
                ts2 << QString( "v %1 %2 %3\n" ).arg( p.x ).arg( p.y ).arg( p.z );
                esp++;
            }
        }
    }

    k = 1;

    while ( k <= evsp )
    {
        ts2 << QString( "p %1\n" ).arg( k );
        k++;
    }

    ts2.flush();
    edgeSnapPointsFile.close();

    FacesPtr facesList = body->GetFaces( igQueryAll );

    QFile faceSnapPointsFile( path + QDir::separator() + "faceSnapPoints." + name + ".obj" );
    faceSnapPointsFile.open( QFile::WriteOnly );
    QTextStream ts3( &faceSnapPointsFile );
    int fsp = 1;

    for ( int i = 1; i <= facesList->Count; i++ )
    {
        FacePtr face = facesList->Item( i );
        int faceId = face->GetTag();
        Point3D p;
        //if ( face->GetTag() == 3590 )
        {
            int code = EvaluateFaceSnapPoint3( face, p );
            std::set<Point3D>::const_iterator closest = snapPointsCollection.find( p );

            if ( code != 0 || ( closest != snapPointsCollection.end() ) )
            {
                QMessageBox::critical( GetTopWindow(), qApp->applicationName(),
                                       QString( "DOUBLICATE SNAP POINT FACE ID %1, CODE %2" ).arg( faceId ).arg( code ) );
                Utils::writeFaceToFile( path + QDir::separator() + "face." + name, face );
                break;
            }
            else
            {
                snapPointsCollection.insert( p );
                ts3 << QString( "o %1\n" ).arg( face->GetTag() );
                ts3 << QString( "v %1 %2 %3\n" ).arg( p.x ).arg( p.y ).arg( p.z );
                ts3 << QString( "f %1 %1 %1\n" ).arg( fsp );
                fsp++;
            }
        }
    }

    ts3.flush();
    faceSnapPointsFile.close();

    PartDocumentPtr pPartDocument =
        CCommands::GetApplicationPtr()->ActiveDocument;
    ModelsPtr pModelsCollection = pPartDocument->Models;

    for ( int i = 1; i <= pModelsCollection->Count; i++ )
    {
        ModelPtr pModel = pModelsCollection->Item( i );
        BodyPtr pBody = pModel->Body;
        FacesPtr pFacesCollection = pBody->Faces[igQueryAll];
        int j = 1;
        FacePtr face = 0x0;

        for ( ; j <= pFacesCollection->Count; j++ )
        {
            face = pFacesCollection->Item( j );

            //if ( face->GetTag() == 3590 )
            {
                std::set<Point3D>::const_iterator it = snapPointsCollection.begin(),
                                                  t = snapPointsCollection.end();
                Point3D point;

                for ( ; it != t; ++it )
                {
                    point = *it;
                    SAFEARRAY* saPoints = Utils::createSafeArrayForPoint( point );
                    SAFEARRAY* saParams = 0x0;
                    SAFEARRAY* saGuessParam = 0x0;
                    SAFEARRAY* saMaxDeviation = 0x0;
                    SAFEARRAY* saFlags = 0x0;

                    hr = face->GetParamAtPoint( 1, &saPoints, &saGuessParam, &saMaxDeviation, &saParams, &saFlags );
                    Q_ASSERT( SUCCEEDED( hr ) );

                    double uv[2];

                    for ( long k = 0; k < 2; k++ )
                        ::SafeArrayGetElement( saParams, &k, &uv[k] );

                    ::SafeArrayDestroy( saPoints );
                    ::SafeArrayDestroy( saMaxDeviation );
                    ::SafeArrayDestroy( saFlags );

                    SAFEARRAY* saCoordinates = NULL;
                    hr = face->GetPointAtParam( 1, &saParams, &saCoordinates );
                    Q_ASSERT( SUCCEEDED( hr ) );
                    Point3D pointonsurf;
                    Utils::getCoordinatesFromSafeArray( saCoordinates, pointonsurf );
                    ::SafeArrayDestroy( saCoordinates );
                    ::SafeArrayDestroy( saParams );

                    if ( pointonsurf.DistanceTo( point ) < 1.e-3 )
                        break;
                }

                if ( it == snapPointsCollection.end() )
                {
                    QMessageBox::critical( GetTopWindow(), qApp->applicationName(),
                                           QString( "UNABLE TO FIND ANY FACE BY GIVEN POINT" ) );
                    Utils::writeFaceToFile( path + QDir::separator() + "emptyface." + name, face );
                    return;
                }
                else
                    snapPointsCollection.erase( it );
            }
        }
    }

    QMessageBox::information( GetTopWindow(), qApp->applicationName(), "Naming successful" );
}
