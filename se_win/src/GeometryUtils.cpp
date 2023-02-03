#include "GeometryUtils.h"

double GeometryUtils::pointToLineDistance( const double* a, const double* b, const double* point )
{
    double p[3], q[3];
    double v[3];

    SUB_VECTOR( q, b, a );
    SUB_VECTOR( p, point, a );
    double t = DOT_PRODUCT( p, q ) / ( double )DOT_PRODUCT( q, q );

    MUL_VECTOR( v, q, t );
    SUB_VECTOR( v, p, v );

    return sqrt( DOT_PRODUCT( v, v ) );
}

double GeometryUtils::pointToPlaneDistance( const double* a, const double* b, const double* c, const double* point )
{
    double v1[3]; // vector ab
    v1[0] = b[0] - a[0];
    v1[1] = b[1] - a[1];
    v1[2] = b[2] - a[2];
    double v2[3]; // vector ac
    v2[0] = c[0] - a[0];
    v2[1] = c[1] - a[1];
    v2[2] = c[2] - a[2];
    double n[3]; // normal vector
    CROSS_PRODUCT( n, v1, v2 );

    double p = n[0] * a[0] + n[1] * a[1] + n[2] * a[2];
    return fabs( n[0] * point[0] + n[1] * point[1] + n[2] * point[2] - p ) /
           sqrt( n[0] * n[0] + n[1] * n[1] + n[2] * n[2] );
}
