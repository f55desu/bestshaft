#ifndef UTILS_H
#define UTILS_H

#define CURVE_ON_PLANE 1
#define MINIMUM_ON_BOUNDARY 2
#define MAXIMUM_ON_BOUNDARY MINIMUM_ON_BOUNDARY
#define MINIMUM_EXISTS 0
#define MAXIMUM_EXISTS MINIMUM_EXISTS
#define NO_EXTREMUM_EXISTS CURVE_ON_PLANE

const double e_absolute = 1.e-3;
const double e_point_absolute = 1.e-1;
const double e_absolute_quad = e_absolute * e_absolute;
const double e_percent = 1.e-2;
const double root_tolerance_percent = 1.e-2;
const double boundary_tolerance_percent = 1.e-1;
const double tau = ( 1. - ::sqrt( 5. ) ) / 2.;

struct Point3D;
class Utils
{
public:
//  static double GoldenSection(const FacePtr& face, double* uv, double* x, double a, double b, int xyzIndex);
    static bool isEdgeALine( const EdgePtr& edge );

    static bool getPointOnEdgeUseByParameter( const EdgeUsePtr& edgeUse, double parameter, Point3D& pointCoordinates );
    static bool getPointOnEdgeByParameter( const EdgePtr& edge, double parameter, Point3D& point );
    static Point3D getPointOnFaceByParameter( const FacePtr& face, double* parameterUV );
    static bool getVertexCoordinates( const VertexPtr& vertex, double* vertexCoordinates );
    static bool getRootByChord( const EdgePtr& edge, double minT, double maxT, const Point3D& basePoint,
                                const Point3D& normalVector, double& result );
    static void writeVectorToFile( const QString& filename, const QVector3D& startPoint, const QVector3D& endPoint );
    static void writeVectorToFile( const QString& filename, const Point3D& startPoint, const Point3D& endPoint );
    static void writeVectorsToFile( const QString& filename, const QVector3D& startPoint,
                                    const QList<QVector3D>& endPoints );
    static void writePointToFile( const QString& filename, const Point3D& point );
    static void writePointToFile( const QString& filename, const QVector3D& point );
    static void writePointsToFile( const QString& filename, const QList<QVector3D>& points );
    static bool getCoordinatesFromSafeArray( SAFEARRAY* safeArray, QVector3D& resultVector );
    static bool getCoordinatesFromSafeArray( SAFEARRAY* safeArray, double* coordinates );
    static bool getCoordinatesFromSafeArray( SAFEARRAY* safeArray, Point3D& point );
    static SAFEARRAY* createSafeArrayForCoordinates( const QVector3D& coordinates );
    static SAFEARRAY* createSafeArrayForPoint( const Point3D& point );
    static SAFEARRAY* createSafeArrayForParam( double param );
    static double pointToPointDistance( const QVector3D& a, const QVector3D& b );
    static QVector3D getMinPointOnClosedEdge( const EdgePtr& edge );
    static bool isFirstQVector3DLess( const QVector3D& first, const QVector3D& second );
    static IDispatchPtr getEntityByID( const PartDocumentPtr& document, int id );
    static bool checkDifference( double a, double b, double e );
    static double planeEquation( const Point3D& point, const Point3D& basePoint, const Point3D& normalVector );
    static void writeFaceToFile( const QString& filename, const FacePtr& face );
    static void writePointsToFile( const QString& filename, const std::vector<Point3D>& intersectionColl );
    static void getFaceFirstPartialDerivative( const FacePtr face, const double* uv, Point3D& du, Point3D& dv );
    static void writeFaceTopology( const QString& filename, const FacePtr& face );
};

#endif // UTILS_H
