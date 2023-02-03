#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <QVector3D>

#define SUB_VECTOR(a,b,c)    ((a)[0] = (b)[0] - (c)[0], \
                              (a)[1] = (b)[1] - (c)[1], \
                              (a)[2] = (b)[2] - (c)[2])
#define MUL_VECTOR(a,b,c)    ((a)[0] = (b)[0]*(c), (a)[1] = (b)[1]*(c), (a)[2] = (b)[2]*(c))
#define DOT_PRODUCT(a,b)     ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])
#define CROSS_PRODUCT(a,b,c) ((a)[0] = (b)[1]*(c)[2] - (b)[2]*(c)[1], \
                              (a)[1] = (b)[2]*(c)[0] - (b)[0]*(c)[2], \
                              (a)[2] = (b)[0]*(c)[1] - (b)[1]*(c)[0])

class GeometryUtils
{
public:
    static double pointToLineDistance( const double* a, const double* b, const double* point );
    static double pointToPlaneDistance( const double* a, const double* b, const double* c, const double* point );
};

#endif // GEOMETRYUTILS_H
