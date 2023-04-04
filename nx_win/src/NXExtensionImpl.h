#pragma once

#include "../../extension/src/BaseExtension.h"
#include "../../utils/Point.h"

/* This defines the UF_CALL(X) macro. */
#define UF_CALL(X) (NXExtensionImpl::Report( __FILE__, __LINE__, #X, (X)))

class NXExtensionImpl : public BaseExtension
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

    static LRESULT CALLBACK QtFilterProc( int nCode,
                                          WPARAM wParam,
                                          LPARAM lParam );

    QWidget* GetTopWindow();

    void InitQt();

//protected:
//    QString PreprocessModel();
protected:
    Variant ExtractVariant();
    void ApplyVariant( BaseExtension::Variant variant );

//    void CalculateMaxTension( BaseExtension::Variant variant );
    enum BoundaryMarker
    {
        DEFAULT,
        CONSTRAINT,
        FORCE
    };

    struct TetgenPoint3D
    {
        Point3D point;
        BoundaryMarker marker;

        TetgenPoint3D& operator=( const TetgenPoint3D& other )
        {
            point = other.point;
            marker = other.marker;

            return *this;
        }

        bool operator==( const TetgenPoint3D& other ) const
        {
            return point == other.point;
        }

        bool operator!=( const TetgenPoint3D& other ) const
        {
            return point != other.point;
        }

        bool operator<( const TetgenPoint3D& other ) const
        {
            return point < other.point;
        }
    };

    struct TetgenFacet
    {
        TetgenPoint3D p1;
        TetgenPoint3D p2;
        TetgenPoint3D p3;
        TetgenPoint3D p4;

        BoundaryMarker marker;

        TetgenFacet& operator=( const TetgenFacet& other )
        {
            p1 = other.p1;
            p2 = other.p2;
            p3 = other.p3;
            p4 = other.p4;

            marker = other.marker;

            return *this;
        }

        // Define operator< for TetgenFacet to use in set
        bool operator<( const TetgenFacet& f2 ) const
        {
            if ( p1 != f2.p1 )
                return p1 < f2.p1;
            else if ( p2 != f2.p2 )
                return p2 < f2.p2;
            else if ( p3 != f2.p3 )
                return p3 < f2.p3;
            else
                return p4 < f2.p4;

//            // Sort the points in each facet so that the order of the points does not matter
//            std::vector<TetgenPoint3D> thisPoints = {p1, p2, p3, p4};
//            std::vector<TetgenPoint3D> otherPoints = {other.p1, other.p2, other.p3, other.p4};
//            std::sort( thisPoints.begin(), thisPoints.end() );
//            std::sort( otherPoints.begin(), otherPoints.end() );

//            // Compare the sorted points
//            return ( thisPoints[0] < otherPoints[0] ) ||
//                   ( thisPoints[0] == otherPoints[0] && thisPoints[1] < otherPoints[1] ) ||
//                   ( thisPoints[0] == otherPoints[0] && thisPoints[1] == otherPoints[1] && thisPoints[2] < otherPoints[2] ) ||
//                   ( thisPoints[0] == otherPoints[0] && thisPoints[1] == otherPoints[1] && thisPoints[2] == otherPoints[2] &&
//                     thisPoints[3] < otherPoints[3] );
        }

//        bool compareFacets( const TetgenFacet& f1, const TetgenFacet& f2 )
//        {
//            if ( f1.p1 != f2.p1 )
//                return f1.p1 < f2.p1;
//            else if ( f1.p2 != f2.p2 )
//                return f1.p2 < f2.p2;
//            else if ( f1.p3 != f2.p3 )
//                return f1.p3 < f2.p3;
//            else
//                return f1.p4 < f2.p4;
//        }
    };

    int SaveSTL( const QString& variant_name, QString& returned_file_path, double& returned_max_facet_size );
    void writePolyFile( std::string fileName, std::set<TetgenPoint3D>& points, std::set<TetgenFacet>& facets );
//    void writeSTL( const std::vector<double>& vertices, const std::string& filename );

//public slots:
//    void CheckContextIsValid();
//public slots:
//    QJSValue DUMP_All();

//protected:
//    QString Test();
//protected:
//    void NamingTest();
    // for extrude
public:
    uf_list_p_t objectList;
//public:
//    bool firstSolidFlag;
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
    static int Report( const char* file, int line, const char* call, int irc );

private:
    int m_registered;

private:
    static NXExtensionImpl m_instance;
private:
    static HHOOK m_hHook;
private:
    static UF_MB_action_t m_actionTable[];
};
