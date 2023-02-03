#pragma once

#include <limits>

const double e_point_absolute = 1.e-1;

#define CHECK_DIFFERENCE(a,b,e) (::fabs((a) - (b)) < (e))

struct Point3D
{
public:
    inline Point3D() : x( std::numeric_limits<double>::max() ),
        y( std::numeric_limits<double>::max() ),
        z( std::numeric_limits<double>::max() ) {}
    inline Point3D( double v ) : x( v ), y( v ), z( v ) {}
    inline Point3D( double x, double y, double z ) : x( x ), y( y ), z( z ) {}
    inline Point3D( const double* p ) : x( p[0] ), y( p[1] ), z( p[2] ) {}
    inline Point3D( const Point3D& p )
    {
        x = p.x, y = p.y, z = p.z;
    }
    inline bool operator <( const Point3D& p ) const
    {
        return CHECK_DIFFERENCE( x, p.x, tolerance ) ?
               (
                   CHECK_DIFFERENCE( y, p.y, tolerance ) ?
                   (
                       CHECK_DIFFERENCE( z, p.z, tolerance ) ?
                       false
                       :
                       ( z < p.z ) ?
                       true
                       :
                       false
                   )
                   :
                   ( y < p.y ) ?
                   true
                   :
                   false

               )
               :
               ( x < p.x ) ?
               true
               :
               false;
    }
    inline bool operator <=( const Point3D& p ) const
    {
        return CHECK_DIFFERENCE( x, p.x, tolerance ) ?
               (
                   CHECK_DIFFERENCE( y, p.y, tolerance ) ?
                   (
                       CHECK_DIFFERENCE( z, p.z, tolerance ) ?
                       true
                       :
                       ( z < p.z ) ?
                       true
                       :
                       false
                   )
                   :
                   ( y < p.y ) ?
                   true
                   :
                   false

               )
               :
               ( x < p.x ) ?
               true
               :
               false;
    }
    inline Point3D operator *( double c ) const
    {
        return Point3D( x * c, y * c, z * c );
    }
    inline Point3D operator ^( const Point3D& p ) const
    {
        return Point3D( y * p.z - z * p.y,
                        z * p.x - x * p.z, x * p.y - y * p.x );
    }
    inline Point3D& operator *=( double c )
    {
        x *= c, y *= c, z *= c;
        return *this;
    }
    inline Point3D& Invert()
    {
        x *= -1., y *= -1., z *= -1.;
        return *this;
    }
    inline Point3D& Norm()
    {
        double d = Length();
        x /= d, y /= d, z /= d;
        return *this;
    }
    inline double Length() const
    {
        return ::sqrt( x * x + y * y + z * z );
    }
    inline double operator *( const Point3D& p ) const
    {
        return x * p.x + y * p.y + z * p.z;
    }
    inline operator bool() const
    {
        return !( x == std::numeric_limits<double>::max() ||
                  y == std::numeric_limits<double>::max() ||
                  z == std::numeric_limits<double>::max() );
    }
    inline double operator []( int index ) const
    {
        return ( ( double* )this )[index];
    }
    inline double& operator []( int index )
    {
        return ( ( double* )this )[index];
    }
    inline Point3D& operator =( const Point3D& p )
    {
        x = p.x, y = p.y, z = p.z;
        return *this;
    }
    inline bool operator ==( const Point3D& p ) const
    {
        return /*x == p.x && y == p.y && z == p.z;*/IsClosedTo( p );
    }
    inline bool operator !=( const Point3D& p ) const
    {
        return !( *this == p );
    }
    inline Point3D operator +( const Point3D& p ) const
    {
        return Point3D( x + p.x, y + p.y, z + p.z );
    }
    inline Point3D operator -( const Point3D& p ) const
    {
        return Point3D( x - p.x, y - p.y, z - p.z );
    }
    inline bool IsClosedTo( const Point3D& p, double e ) const
    {
        const Point3D v( *this - p );
        return v * v < e;
    }
    inline bool IsClosedTo( const Point3D& p ) const
    {
        const Point3D v( *this - p );
        return v * v < tolerance;
    }
    inline bool IsClosedTo2( const Point3D& p ) const
    {
        return CHECK_DIFFERENCE( x, p.x, e_point_absolute ) &&
               CHECK_DIFFERENCE( y, p.y, e_point_absolute ) &&
               CHECK_DIFFERENCE( z, p.z, e_point_absolute );
    }
    inline double DistanceTo( const Point3D& p ) const
    {
        const Point3D v( *this - p );
        return ::sqrt( v * v );
    }
    inline void Reset()
    {
        *this = Point3D();
    }
    inline static void SetDistanceTolerance( const double tolerance )
    {
        Point3D::tolerance = tolerance;
    }
    inline static double GetDistanceTolerance()
    {
        return Point3D::tolerance;
    }

    double x;
    double y;
    double z;
private:
    static double tolerance;
};

typedef Point3D Vector3D;
typedef std::vector<Point3D> Points3DCollection;
