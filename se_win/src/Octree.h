#ifndef OCTREE_H
#define OCTREE_H

#include <QVector3D>
#include <QList>

class Octant
{

public:
    Octant( const QVector3D& minPoint, const QVector3D& maxPoint );
    ~Octant();

    void createSubOctants();
    int checkForEntities( const QHash<int, QVector3D>& primitivesList );
    int findPrimitiveByPoint( const QVector3D& selectPoint ) const;
    int getSubOctantsCount() const;

private:
    bool checkIsPointInsideOctant( const QVector3D& point ) const;

    QVector3D m_minPoint;
    QVector3D m_maxPoint;
    QHash<int, QVector3D> m_primitives;
    QList<Octant*> m_subOctants;
};

#endif // OCTREE_H
