#ifndef SOLIDEDGEEXTENSIONIMPL_H
#define SOLIDEDGEEXTENSIONIMPL_H

#include "stdafx.h"
#include "../../extension/src/BaseExtension.h"
#include "../../extension/src/IAbstractModeler.h"
#include "Octree.h"

#define GET_QSCRIPTVALUE_FROM_COM(ptr) m_scriptEngine->newVariant(qVariantFromValue((void*)ptr.Detach()))
#define GET_COM_FROM_QSCRIPTVALUE(ptr, scriptValue, className) ptr.Attach((className*)(scriptValue.toVariant()).value<void*>(), true)
#define GET_QSTRING_FROM_BSTR(bstr) QString::fromUtf16(reinterpret_cast<const ushort*>((const WCHAR*)(bstr)))

struct LoopBasePointInfo;
struct Point3D;

class SolidEdgeExtensionImpl : public BaseExtension, public IAbstractModeler
{
    Q_OBJECT

private:
    SolidEdgeExtensionImpl();

public:
    ~SolidEdgeExtensionImpl();

    static SolidEdgeExtensionImpl& Instance();

    virtual void Initialize();
    virtual void Terminate();

    QWidget* GetTopWindow();
    static LRESULT CALLBACK QtFilterProc( int nCode, WPARAM wParam, LPARAM lParam );

    QJSValue createBaseCsys( double matrix[9] );
    QJSValue CheckPostprocess();
    void CheckFirstSolidInPart();
    QJSValue getActiveSketch();
    QJSValue createPointOnCurve( QJSValue curve, double t );
    QJSValue getPlaneXOY( QJSValue csys );
    QJSValue getAxisOX( QJSValue csys );

public slots:
    QJSValue CheckContextIsValid();
    virtual QJSValue DUMP_All()
    {
        return QJSValue();
    }
    void setEpsilon( double epsilon );
    void setTolerance( double tolerance );
    QJSValue SELECT_Object( double xPoint, double yPoint, double zPoint, TopologyPriority topologyPriority = TP_ALL );
    QJSValue SET_Names_For_Topology( TopologyPriority topologyPriority = TP_ALL );
    void BUILD_Octree();
    QJSValue SELECT_Object_Octree( double xPoint, double yPoint, double zPoint );
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                       QJSValue base_axis,
                                                                       QJSValue base_point );
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue y_dir );
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                   QJSValue y_dir,
                                                                   QJSValue z_dir );
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue z_dir );
    QJSValue CONSTRAINTS_Create_Base_CoorSys( QVariantList matr );
    QJSValue SKETCH_Open( QString skt_name, QJSValue csys );
    QJSValue SKETCH_Close( QJSValue sketch );
    QJSValue SKETCH_Create_2D_Line_2Points( QJSValue sketch, QString line_name,
                                            double x1, double y1,
                                            double x2, double y2, bool flag );
    QJSValue SOLID_Create_Block( QJSValue qsv_sketch, QString name,
                                 double xOrigin, double yOrigin, double zOrigin,
                                 double xLen, double yLen, double zLen );
    QJSValue SOLID_Create_Protrusion_Extrude( QJSValue sketch, QString extrude_name,
                                              double depth_start, double depth_end, bool sign = true );
    void SOLID_Create_Block_WDH( QJSValue sketch, double x, double y, double z,
                                 double width, double depth, double height );
    void SOLID_Create_Block_2Points( QJSValue sketch, double x1, double y1, double z1,
                                     double x2, double y2, double z2 );
public:
    virtual QJSValue SOLID_Create_Block( QString name,
                                         double xOrigin,
                                         double yOrigin,
                                         double zOrigin,
                                         double xLen,
                                         double yLen,
                                         double zLen )
    {
        return QJSValue();
    }
public:
    virtual void SOLID_Create_Block_WDH( double x,
                                         double y,
                                         double z,
                                         double width,
                                         double depth,
                                         double height ) {}

public:
    virtual void SOLID_Create_Block_2Points( double x1,
                                             double y1,
                                             double z1,
                                             double x2,
                                             double y2,
                                             double z2 ) {}

    void DumpTopology();
    void DumpTopologyToObj();

private:
    QVector<IDispatchPtr> getAllVertices() const;
    QVector<IDispatchPtr> getAllEdges() const;
    QVector<IDispatchPtr> getAllFaces() const;

    void setNamesForVertices( const VerticesPtr& vertices );
    void setNamesForEdges( const EdgesPtr& edges );
    void setNamesForFaces( const FacesPtr& faces );

    int selectVertex( double xPoint, double yPoint, double zPoint );
    int selectEdge( double xPoint, double yPoint, double zPoint );
    int selectFace( double xPoint, double yPoint, double zPoint );

    void FillPrimitivesCoordinates();
    bool GetCoordinatesFromVertex( const VertexPtr& vertex, QVector3D& coordinates );
    bool GetPointOnEdge( const EdgePtr& edge, QVector3D& point );
    bool GetEdgeMidpoint( const EdgePtr& edge, QVector3D& edgeMidpoint );
    bool GetFaceMidPoint( const FacePtr& face, QVector3D& faceMidpoint );
    void GetCutPlane( const LoopBasePointInfo& loopBasePointInfo, double* cutPlane, int faceID );
    void GetLoopBasePoint( const LoopPtr& loop, LoopBasePointInfo& loopBasePointInfo );
    int GetTSnap( const EdgePtr& edge, double* X, double* limits, Point3D& point, bool periodic );
    void GetUVSnap( const FacePtr& face, double* uv, double* limits, bool periodic );
    void EdgeCutPlaneIntersect( const EdgePtr& edge, const double* cutPlane, QVector<Point3D>& intersectionPointsCollection,
                                const LoopBasePointInfo& loopBasePointInfo );
    Point3D& GetClosedToBasePoint( const FacePtr& face, const QVector<Point3D>& intersectionPointsCollection,
                                   Point3D& closedPoint );

    void DumpTangents();

    void TraverseFeatureRelatives( const IDispatchPtr& feature );
    void preprocessProtrusionExtrude_Extrude( const IDispatchPtr& feature );
    void preprocessSketch( const SketchPtr& sketch );
    QString preprocessSketchOpen();
    void dumpAll();

protected:
    void InitQt();
    QString Test();
    void NamingTest();
    QString PreprocessModel();

private:
    void sketch( const ProfilePtr& profile );

    int m_registered;
    bool m_firstSolidFlag;

    double m_epsilon;
    double m_tolerance;

    static SolidEdgeExtensionImpl m_instance;
    static HHOOK m_hHook;

    QHash<int, QString> m_topologyObjectsNames;
    QHash<int, QVector3D> m_topologyObjectCoordinates;
    Octant* m_octree;

    // preprocess
    QString m_content;
    QString m_indent;
    QString name_csys;
    QString name_edge;
    QString name_face;
    QString name_point;
    int m_countFeatureSketch, m_countNewCsys, count_edge, count_face, count_point_on_edge;
    int numberCurrentFeature;
    bool isSoDirection;
    QVector<IDispatchPtr> allFeature;
    QSet<int> visitedFeaturesSet;
    QSet<int> visitedSmartObjectsSet;
};

#endif // SOLIDEDGEEXTENSIONIMPL_H
