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
public:
    enum BoundaryMarker
    {
        DEFAULT,                // грань без атрибутов
        CONSTRAINT,             // закрепления (нижняя горизонтальная грань)
        FORCE,                  // нагрузки (верхняя горизонтальная грань)
        MORE_DETAILED,          // более мелкая сетка в районе перехода эксцентрикового вала
        INTERMEDIATE_TOP,       // верхняя боковая грань
        INTERMEDIATE_MIDDLE,    // центральная горизонтальная грань
        INTERMEDIATE_BOTTOM     // нижняя боковая грань
    };

protected:
    Variant ExtractVariant();
    void ApplyVariant( BaseExtension::Variant variant );

//    void CalculateMaxTension( BaseExtension::Variant variant );

    struct TetgenPoint3D
    {
        Point3D point;
        BoundaryMarker marker;

        double force_value = 0.0;

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

        static void ConvertIntToBoundaryMarker( BoundaryMarker& obj, int value )
        {
            if ( value == 0 || value == -1 )
                obj = BoundaryMarker::DEFAULT;
            else if ( value == 1 )
                obj = BoundaryMarker::CONSTRAINT;
            else if ( value == 2 )
                obj = BoundaryMarker::FORCE;
            else if ( value == 3 )
                obj = BoundaryMarker::MORE_DETAILED;
            else if ( value == 4 )
                obj = BoundaryMarker::INTERMEDIATE_TOP;
            else if ( value == 5 )
                obj = BoundaryMarker::INTERMEDIATE_MIDDLE;
            else if ( value == 6 )
                obj = BoundaryMarker::INTERMEDIATE_BOTTOM;
            else
                throw std::invalid_argument( "Invalid value for BoundaryMarker" );
        }

        friend std::istream& operator>>( std::istream& in, TetgenPoint3D& obj )
        {
            int id/* ignore */, marker_value;
            in >> id >> obj.point.x >> obj.point.y >> obj.point.z >> marker_value;
            TetgenPoint3D::ConvertIntToBoundaryMarker( obj.marker, marker_value );

            return in;
        }

//        friend std::ostream& operator<<( std::ostream& out, const TetgenPoint3D& obj )
//        {
//            out << obj.point.x << " " << obj.point.y << " " << obj.point.z;
//            return out;
//        }
    };

    struct TetgenFacet
    {
        TetgenPoint3D p1;   // first
        TetgenPoint3D p2;   // middle
        TetgenPoint3D p3;   // end

        BoundaryMarker marker;

        TetgenFacet& operator=( const TetgenFacet& other )
        {
            p1 = other.p1;
            p2 = other.p2;
            p3 = other.p3;

            marker = other.marker;

            return *this;
        }

        // Define operator< for TetgenFacet to use in set
        bool operator<( const TetgenFacet& other ) const
        {
            if ( p1 != other.p1 )
                return p1 < other.p1;
            else if ( p2 != other.p2 )
                return p2 < other.p2;
            else
                return p3 < other.p3;
        }
    };

    struct Facet
    {
        int _p1_id,
            _p2_id,
            _p3_id;

        double facet_area;

        Facet() {}    // default
        Facet( int p1_id, int p2_id, int p3_id ):
            _p1_id( p1_id ), _p2_id( p2_id ), _p3_id( p3_id ), facet_area( 0.0 )
        {}

        Facet& operator=( const Facet& other )
        {
            _p1_id = other._p1_id;
            _p2_id = other._p2_id;
            _p3_id = other._p3_id;

            return *this;
        }

        // Define operator< for TetgenFacet to use in set
        bool operator<( const Facet& other ) const
        {
            if ( _p1_id != other._p1_id )
                return _p1_id < other._p1_id;
            else if ( _p2_id != other._p2_id )
                return _p2_id < other._p2_id;
            else
                return _p3_id < other._p3_id;
        }
    };

    struct Tetrahedron
    {
        int p1_id,
            p2_id,
            p3_id,
            p4_id;

        // Define operator< for TetgenFacet to use in set
        bool operator<( const Tetrahedron& other ) const
        {
            if ( p1_id != other.p1_id )
                return p1_id < other.p1_id;
            else if ( p2_id != other.p2_id )
                return p2_id < other.p2_id;
            else if ( p3_id != other.p3_id )
                return p3_id < other.p3_id;
            else
                return p4_id < other.p4_id;
        }

        friend std::istream& operator>>( std::istream& in, Tetrahedron& obj )
        {
            int id/* ignore */;
            in >> id >> obj.p1_id >> obj.p2_id >> obj.p3_id >> obj.p4_id;
            return in;
        }

    };

    void WriteWavefrontObjFile( const QString& fileName,
                                const std::set<TetgenPoint3D>& points,
                                const std::vector<TetgenFacet>& facets );

    void WriteStlFile( const QString& fileName,
                       const std::vector<TetgenFacet>& facets );

    void WritePolyFile( const QString& fileName,
                        const std::set<TetgenPoint3D>& points,
                        const std::vector<TetgenFacet>& facets );

    void WriteSmeshFile( const QString& fileName,
                         const std::set<TetgenPoint3D>& points,
                         const std::vector<TetgenFacet>& facets );

    void WriteMtrFile( const QString& fileName,
                       const std::set<TetgenPoint3D>& points,
                       const double& bounding_box_diagonal_size );

    void GetMeshData( std::set<TetgenPoint3D>& mesh_points,
                      std::vector<TetgenFacet>& mesh_facets,
                      double& mesh_bounding_box_diagonal_size,
                      double& mesh_max_facet_size );

    void SaveMeshDatabase( const QString& wavefront_obj_file_path,
                           const QString& stl_file_path,
                           const QString& tetgen_input_poly_file_path,
                           const QString& tetgen_input_smesh_file_path,
                           const QString& tetgen_input_mtr_file_path,
                           double& max_facet_size );

    void WriteAbaqusInputFile( const QString& tetgen_output_nodes_file_path,
                               const QString& tetgen_output_faces_file_path,
                               const QString& tetgen_output_tetrahedrons_file_path,
                               const QString& calculix_input_file_path,
                               const QString& variant_name,
                               const double& applied_force );

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
