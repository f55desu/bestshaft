#pragma once

#include "../../extension/src/BaseExtension.h"
#include "../../extension/src/IAbstractModeler.h"


/* This defines the UF_CALL(X) macro. */
#define UF_CALL(X) (NXExtensionImpl::Report( __FILE__, __LINE__, #X, (X)))

class NXExtensionImpl : public BaseExtension, public IAbstractModeler
{
    Q_OBJECT

private:
    NXExtensionImpl();
public:
    ~NXExtensionImpl();

public:
    inline static NXExtensionImpl& Instance()
    {
        return m_instance;
    }

public:
    virtual void Initialize();
public:
    virtual void Terminate();

    //------------------------- Callback Prototypes -----------------------
private:
    static UF_MB_cb_status_t RunEditorAction( UF_MB_widget_t widget,
                                              UF_MB_data_t client_data,
                                              UF_MB_activated_button_p_t call_button );
private:
    static UF_MB_cb_status_t AboutAction( UF_MB_widget_t widget,
                                          UF_MB_data_t client_data,
                                          UF_MB_activated_button_p_t call_button );

private:
    static LRESULT CALLBACK QtFilterProc( int nCode,
                                          WPARAM wParam,
                                          LPARAM lParam );

private:
    QWidget* GetTopWindow();
private:
    void InitQt();
protected:
    QString PreprocessModel();
public slots:
    void CheckContextIsValid();
public slots:
    QJSValue DUMP_All();

protected:
    QString Test();
protected:
    void NamingTest();
    // for extrude
public:
    uf_list_p_t objectList;
public:
    bool firstSolidFlag;
public:
    tag_t globalTagPart;
public:
    QVector<int> getAllFace();
public:
    QVector<int> getAllEdge();

    //public: QVector<int> allFace;
    //public: QVector<int> allEdge;
    //public: QVector<int> allSoPoint;

    /*public: struct intersect_info_face
    {
    int intersected_face;
    double t;
    };

    public: struct intersect_info_edge
    {
    int intersected_edge;
    double t;
    };

    public:    QVector<intersect_info_face> allIntersectFace;
    public:    QVector<intersect_info_edge> allIntersectEdge;*/

    // other function
public:
    void MapToWorld( double sket_csys[12], double point[3] ); // translate coordinats
public:
    void CheckFirstSolidInPart();
public:
    void CheckPostprocess();
public:
    QJSValue getActiveSketch();


    //public: void FindAllSoPoint();
    //public: static bool compareFace (const intersect_info_face &value1, const intersect_info_face &value2);

    //public: static bool compareEdge (const intersect_info_edge &value1, const intersect_info_edge &value2);

public:
    QJSValue createBaseCsys( double matrix[9] );
public:
    QJSValue createPointOnCurve ( QJSValue curve, double t );

public:
    QJSValue getPlaneXOY( QJSValue csys );

public:
    QJSValue getAxisOX( QJSValue csys );

    /*public slots: QJSValue SELECT_Face(double Xp, double Yp, double Zp,
    double Xr, double Yr, double Zr,
    int number);

    public slots: QJSValue SELECT_Edge(double Xp, double Yp, double Zp,
    double Xr, double Yr, double Zr,
    int number);*/

public slots:
    QJSValue SELECT_Object ( double xPoint, double yPoint, double zPoint,
                             TopologyPriority topologyPriority = TP_ALL );

//public slots: QJSValue SELECT_Point(double Xr, double Yr, double Zr, QString direction);

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                       QJSValue base_axis,
                                                                       QJSValue base_point );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue y_dir );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                   QJSValue y_dir,
                                                                   QJSValue z_dir );

public slots:
    QJSValue CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                   QJSValue x_dir,
                                                                   QJSValue z_dir );

//public/* slots*/: QJSValue CONSTRAINTS_Create_3DReference_Point_on_curve(QJSValue curve, double t);

public slots:
    QJSValue CONSTRAINTS_Create_Base_CoorSys( QVariantList matr );

public slots:
    QJSValue SKETCH_Open( QString skt_name, QJSValue csys );

public slots:
    QJSValue SKETCH_Close( QJSValue sketch );

public slots:
    QJSValue SKETCH_Create_2D_Line_2Points( QJSValue sketch, QString line_name,
                                            double x1,
                                            double y1,
                                            double x2,
                                            double y2,
                                            bool flag = true );
public slots:
    QJSValue SKETCH_Create_2D_Arc_3Points( QJSValue sketch, QString arc_name,
                                           double x1,
                                           double y1,
                                           double x2,
                                           double y2,
                                           double x3,
                                           double y3,
                                           bool flag = true );
public slots:
    QJSValue SOLID_Create_Block( QString name,
                                 double xOrigin,
                                 double yOrigin,
                                 double zOrigin,
                                 double xLen,
                                 double yLen,
                                 double zLen );

public slots:
    QJSValue SOLID_Create_Protrusion_Extrude( QJSValue selected_sketch,
                                              QString extrude_name,
                                              double depth_start,
                                              double depth_end,
                                              bool sign = true );

public slots:
    void SOLID_Create_Block_WDH( double x,
                                 double y,
                                 double z,
                                 double width,
                                 double depth,
                                 double height );

public slots:
    void SOLID_Create_Block_2Points( double x1,
                                     double y1,
                                     double z1,
                                     double x2,
                                     double y2,
                                     double z2 );

public:
    static int Report( const char* file, int line, const char* call, int irc );

private:
    int m_registered;

private:
    static NXExtensionImpl m_instance;
private:
    static HHOOK m_hHook;
private:
    static UF_MB_action_t m_actionTable[];

    // preprocess
public:
    QString content;
public:
    QString indent;
public:
    QString name_csys;
public:
    QString name_edge;
public:
    QString name_face;
public:
    QString name_point;
public:
    int count_feature_sketch, count_new_csys, count_edge, count_face, count_point_on_edge;
public:
    int numberCurrentFeature;
//public: bool isSoDirection;
public:
    QVector<int> allFeature;
public:
    std::set<int> visitedFeaturesSet;
public:
    std::set<int> visitedSmartObjectsSet;

public:
    void TraverseFeatureRelatives( tag_t feature, QString dumpIndent );
public:
    void preprocessProtrusionExtrude_SWP( tag_t feature );
public:
    void preprocessProtrusionExtrude_Extrude( tag_t feature );
public:
    void preprocessBlock( tag_t feature );
public:
    void preprocessBaseCSYS( double matrix[] );
public:
    void preprocessLine2Points( tag_t feature, double sketchCSYS[], int num );
public:
    void preprocessSketch( tag_t feature );
public:
    void preprocessCSYSPlaneAxisPoint( tag_t face, tag_t edge, tag_t edgeForPoint, double scalar );
public:
    void MapToSketch( double sket_csys[], double point[] );

public:
    void testCsysPre();
public:
    void testExtrudePre();
    struct information_xform
    {
        tag_t edge,
              face,
              point_scalar,
              edgeForPoint;
        int status_edge,
            status_face,
            status_edgeForPoint;
    };
private:
    static information_xform info_xform;
private:
    static bool isSoDirection;
    struct Visitor
    {
        Visitor() : tagCsys( NULL_TAG ), tagCurrentFeature( NULL_TAG ) {}
        tag_t tagCsys;
        std::vector<tag_t> tagSketches;
        tag_t tagCurrentFeature;
        std::set<tag_t> tagAlreadyVisited;
        QString PreprocessBody( tag_t tagBody );
        tag_t ExtractFirstFeature( tag_t tagBody );
        QString PreprocessFeature( tag_t tagFeature );
        QString PreprocessExtrudeFeatureSWP( tag_t tagExtrudeFeature );
        QString PreprocessExtrudeFeatureEXTRUDE( tag_t tagExtrudeFeature );
        QString PreprocessSketch( tag_t tagSketchFeature );
        QString PreprocessCSYS( tag_t tagCSYSFeature );
        tag_t TraverseParent( tag_t tagParentFeature );
        void TraverseSmartObjects( tag_t feature );
        void MapToSketch( double sket_csys[12], double point[3] );
        QString scriptContent;
        QString name_csys;
        QString name_face;
        QString name_edge;
        QString name_point;
        QString indent;
        int sketch_counter;
        int face_counter;
        int edge_counter;
        int point_counter;
        int csys_counter;
    };
private:
    Visitor visitor;
};
