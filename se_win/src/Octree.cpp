#include <QList>
#include "SolidEdgeExtensionImpl.h"

#include "Octree.h"

Octant::Octant( const QVector3D& minPoint, const QVector3D& maxPoint )
    : m_minPoint( minPoint ),
      m_maxPoint( maxPoint )
{
    m_primitives.clear();
    m_subOctants.clear();
}

Octant::~Octant()
{
    qDeleteAll( m_subOctants );
}

void Octant::createSubOctants()
{
    double minXPoint = m_minPoint.x();
    double midXPoint = ( m_maxPoint.x() + m_minPoint.x() ) / 2;
    double maxXPoint = m_maxPoint.x();
    double minYPoint = m_minPoint.y();
    double midYPoint = ( m_maxPoint.y() + m_minPoint.y() ) / 2;
    double maxYPoint = m_maxPoint.y();
    double minZPoint = m_minPoint.z();
    double midZPoint = ( m_maxPoint.z() + m_minPoint.z() ) / 2;
    double maxZPoint = m_maxPoint.z();

    m_subOctants.append( new Octant( QVector3D( minXPoint, minYPoint, minZPoint ), QVector3D( midXPoint, midYPoint,
                                                                                              midZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( minXPoint, midYPoint, minZPoint ), QVector3D( midXPoint, maxYPoint,
                                                                                              midZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( midXPoint, minYPoint, minZPoint ), QVector3D( maxXPoint, midYPoint,
                                                                                              midZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( midXPoint, midYPoint, minZPoint ), QVector3D( maxXPoint, maxYPoint,
                                                                                              midZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( minXPoint, minYPoint, midZPoint ), QVector3D( midXPoint, midYPoint,
                                                                                              maxZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( minXPoint, midYPoint, midZPoint ), QVector3D( midXPoint, maxYPoint,
                                                                                              maxZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( midXPoint, minYPoint, midZPoint ), QVector3D( maxXPoint, midYPoint,
                                                                                              maxZPoint ) ) );
    m_subOctants.append( new Octant( QVector3D( midXPoint, midYPoint, midZPoint ), QVector3D( maxXPoint, maxYPoint,
                                                                                              maxZPoint ) ) );

    foreach ( Octant* subOctant, m_subOctants )
    {
        if ( subOctant->checkForEntities( m_primitives ) > 1 )
            subOctant->createSubOctants();
    }
}

int Octant::checkForEntities( const QHash<int, QVector3D>& primitivesList )
{
    for ( QHash<int, QVector3D>::ConstIterator it = primitivesList.constBegin(); it != primitivesList.constEnd(); ++it )
    {
        QVector3D primitiveCoordinates = it.value();

        if ( checkIsPointInsideOctant( primitiveCoordinates ) )
            m_primitives.insert( it.key(), it.value() );
    }

    return m_primitives.count();
}

int Octant::findPrimitiveByPoint( const QVector3D& selectPoint ) const
{
    if ( checkIsPointInsideOctant( selectPoint ) == true )
    {
        if ( m_subOctants.isEmpty() )
        {
            if ( m_primitives.isEmpty() )
                return 0;
            else
                return m_primitives.keys().at( 0 ); // There is only one primitive in sub-octant
        }
        else
        {
            foreach ( Octant* subOctant, m_subOctants )
            {
                int primitiveId = subOctant->findPrimitiveByPoint( selectPoint );

                if ( primitiveId > -1 )
                    return primitiveId;
            }
        }
    }

    return -1;
}

int Octant::getSubOctantsCount() const
{
    int count = m_subOctants.count();

    foreach ( Octant* subOctant, m_subOctants )
        count += subOctant->getSubOctantsCount();

    return count;
}

bool Octant::checkIsPointInsideOctant( const QVector3D& point ) const
{
    return ( point.x() >= m_minPoint.x() &&
             point.y() >= m_minPoint.y() &&
             point.z() >= m_minPoint.z() &&
             point.x() <= m_maxPoint.x() &&
             point.y() <= m_maxPoint.y() &&
             point.z() <= m_maxPoint.z() );
}

