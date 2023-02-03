#include "Stable.h"
#include "NXExtensionImpl.h"
#include "nxUtils.h"
#include "../../extension/src/Point.h"

const double e_absolute = 1.e-3;
//const double e_absolute_quad = e_absolute * e_absolute;
const double e_percent = 1.e-2;
const double root_tolerance_percent = 1.e-2;
const double boundary_tolerance_percent = 1.e-1;
const double tau = ( 1. - ::sqrt( 5. ) ) / 2.;

#define CURVE_ON_PLANE 1
#define NO_EXTREMUM_EXISTS CURVE_ON_PLANE
#define MINIMUM_ON_BOUNDARY 2
#define MAXIMUM_ON_BOUNDARY MINIMUM_ON_BOUNDARY
#define MINIMUM_EXISTS 0
#define MAXIMUM_EXISTS MINIMUM_EXISTS

int Evaluate_t_Snap( const UF_EVAL_p_t evaluator, double* X, const double* limits, Point3D& point, bool periodic );
Point3D& edgeObjective( const UF_EVAL_p_t evaluator, const double* t, Point3D& point );
int GoldenSection( const UF_EVAL_p_t evaluator, const double* product, double* x, double a, double b,
                   Point3D& lastPoint, double& lastValue, double e );
int GoldenSectionMax( const UF_EVAL_p_t evaluator, const double* product, double* x, double a, double b,
                      Point3D& lastPoint, double& lastValue, double e );
int GoldenSection( const tag_t face, const double* uv, const double* product, double* x, double a, double b,
                   double& lastValue, Point3D& surfacePoint, double e );

enum ErrorCode
{
    NoError = 0,
    CommonPointNotFound,
    EdgeCutPlaneIntersectEmpty,
    ClosedIntersectionPointNotFound
};

double Point3D::tolerance = e_point_absolute;

struct EdgeInfo
{
public:
    inline EdgeInfo()
    {
        id = NULL_TAG,
        limits[0] = std::numeric_limits<double>::max(),
                    limits[1] = std::numeric_limits<double>::max(),
                                tParamBasePoint = std::numeric_limits<double>::max(),
                                minmax = -2;
    }
    Point3D vertexes[2];
    tag_t id;
    double limits[2];
    double tParamBasePoint;
    int minmax;
    Point3D onEdgePoint;
};

struct LoopBasePointInfo
{
public:
    inline LoopBasePointInfo()
    {
        index[0] = 0, index[1] = 1,
                                 cutPlane[0] = 0.0, cutPlane[1] = 0.0, cutPlane[2] = 0.0, cutPlane[3] = 0.0;
    }

    void SwapEdgesIndex()
    {
        int tmp = index[0];
        index[0] = index[1];
        index[1] = tmp;
    }

    int FindBasePoint()
    {
        if ( adjEdges[0].id == adjEdges[1].id )
        {
            if ( adjEdges[0].minmax == -2 )
            {
                point = adjEdges[0].vertexes[0];
                adjEdges[0].minmax = -1;
                adjEdges[0].tParamBasePoint = adjEdges[0].limits[0];
            }
            else
                point = adjEdges[0].onEdgePoint;

            return NoError;
        }

        int i, j;

        for ( i = 0; i < 2; i++ )
        {
            if ( adjEdges[i].minmax == -1 ) //find edge base point lying on
            {
                if ( adjEdges[i].onEdgePoint < point )
                {
                    point = adjEdges[i].onEdgePoint;

                    if ( i == 1 )
                        SwapEdgesIndex();
                }
            }
        }

        if ( point )
            return NoError;

        for ( i = 0; i < 2; i++ )
            for ( j = 0; j < 2; j++ )
                if ( adjEdges[0].vertexes[i] == adjEdges[1].vertexes[j] &&
                        adjEdges[0].vertexes[i] < point )
                {
                    point = adjEdges[0].vertexes[i];
                    adjEdges[0].tParamBasePoint = adjEdges[0].limits[i];
                    adjEdges[0].minmax = i;
                    adjEdges[1].tParamBasePoint = adjEdges[1].limits[j];
                    adjEdges[1].minmax = j;
                }

        if ( point )
            return NoError;

        return CommonPointNotFound;
    }

    bool CalcCutPlane()
    {
        //Q_ASSERT(loopBasePointInfo.orientation[0] != UF_BREP_ORIENTATION_NONE);

        int n;
        double direction[2] = {1.};

        if ( adjEdges[index[0]].minmax == -1 )
            n = 1;
        else
        {
            n = 2;
            direction[1] = ( ( adjEdges[0].minmax != adjEdges[1].minmax ) ? -1. : 1. );
        }

        Vector3D derivatives[4] = {Vector3D( 0. ), Vector3D( 0. ), Vector3D( 0. ), Vector3D( 0. )};

        for ( int i = 0, j = 0/*, n = (loopBasePointInfo.adjEdge[1] == NULL_TAG) ? 1 : 2*/; i < n; i++, j += 2 )
        {
            UF_EVAL_p_t evaluator;
            UF_CALL( ::UF_EVAL_initialize( adjEdges[index[i]].id, &evaluator ) );

            Point3D p;
            UF_CALL( ::UF_EVAL_evaluate( evaluator, 2, adjEdges[index[i]].tParamBasePoint,
                                         ( double* )&p, ( double* )&derivatives[j] ) );

            UF_CALL( ::UF_EVAL_free( evaluator ) );

            derivatives[j] *= direction[i];
            //tangentVectorOnCurve[i] *= (loopBasePointInfo.orientation[i] == UF_BREP_ORIENTATION_FORWARD) ? 1. : -1.;
            derivatives[j].Norm();
        }

        Vector3D normal = derivatives[0] - derivatives[2];
        //Vector3D normal = tangentVectorOnCurve[0] - tangentVectorOnCurve[1].Invert();

        if ( normal.Length() > e_absolute )
        {
            cutPlane[0] = normal[0];
            cutPlane[1] = normal[1];
            cutPlane[2] = normal[2];
            cutPlane[3] = point * normal * -1.;
        }
        else
        {
            if ( derivatives[1].Length() > e_absolute && derivatives[3].Length() > e_absolute )
            {
                cutPlane[0] = derivatives[1].x;
                cutPlane[1] = derivatives[1].y;
                cutPlane[2] = derivatives[1].z;
                cutPlane[3] = point * normal * -1.;
            }
            else
                return false;
        }

        return true;
    }

    void LoadEdgeInfo( int edgeIndex, tag_t edge )
    {
        Q_ASSERT( edgeIndex >= 0 && edgeIndex < 2 );

        adjEdges[edgeIndex].id = edge;

        UF_EVAL_p_t evaluator;
        UF_CALL( ::UF_EVAL_initialize( edge, &evaluator ) );

        UF_CALL( ::UF_EVAL_ask_limits( evaluator, adjEdges[edgeIndex].limits ) );

        int edgeType;
        UF_CALL( ::UF_MODL_ask_edge_type( edge, &edgeType ) );

        logical is_periodic = false;

        if ( edgeType != UF_MODL_LINEAR_EDGE )
        {
            UF_CALL( ::UF_EVAL_is_periodic( evaluator, &is_periodic ) );

            int result = Evaluate_t_Snap( evaluator, &adjEdges[edgeIndex].tParamBasePoint, adjEdges[edgeIndex].limits,
                                          adjEdges[edgeIndex].onEdgePoint, is_periodic );

            if ( result == MINIMUM_EXISTS )
                adjEdges[edgeIndex].minmax = -1;
            else
                adjEdges[edgeIndex].minmax = -2;
        }

        UF_CALL( ::UF_EVAL_evaluate( evaluator, 0, adjEdges[edgeIndex].limits[0],
                                     ( double* )&adjEdges[edgeIndex].vertexes[0], 0x0 ) );

        if ( !is_periodic )
            UF_CALL( ::UF_EVAL_evaluate( evaluator, 0, adjEdges[edgeIndex].limits[1],
                                         ( double* )&adjEdges[edgeIndex].vertexes[1], 0x0 ) );
        else
            adjEdges[edgeIndex].vertexes[1] = adjEdges[edgeIndex].vertexes[0];

        UF_CALL( ::UF_EVAL_free( evaluator ) );
    }

    void MoveNextEdge()
    {
        adjEdges[0] = adjEdges[1];
        point.Reset();
        index[0] = 0;
        index[1] = 1;
    }

    Point3D point;
    EdgeInfo adjEdges[2];
    double cutPlane[4];
    int index[2];
    //std::string loopID;
    //int orientation[2];
};

/*struct SortItem
{
public:
    inline SortItem() : obj(NULL_TAG) {}
    inline SortItem(const Point3D& point, tag_t obj) : obj(obj), point(point) {}
    inline SortItem(const SortItem& item) : point(item.point), obj(item.obj) {}
    inline bool operator <(const SortItem& item) const {return point < item.point;}

    Point3D point;
    tag_t obj;
};*/

//typedef std::set<Point3D> Points3DSet;
//typedef std::vector<SortItem> SortItemCollection;

/*void GetClosestUVPointOnSurface(const tag_t face, const Point3D& point, double* uv)
{
    const double tol = 1.e-6;
    double p[3],_uv[2];
    UF_CALL(::UF_MODL_ask_face_parm(face, (double*)&point, _uv, p));

    UF_EVALSF_p_t evaluator;
    UF_CALL(::UF_EVALSF_initialize(face, &evaluator));
    UF_EVALSF_pos3_t srf_pos3;
    UF_CALL(::UF_EVALSF_find_closest_point(evaluator, (double*)&point, &srf_pos3));
    UF_CALL(::UF_EVALSF_free(&evaluator));

    if ( CHECK_DIFFERENCE(_uv[0],srf_pos3.uv[0],tol) && CHECK_DIFFERENCE(_uv[1],srf_pos3.uv[1],tol) )
    {
        uv[0] = _uv[0];
        uv[1] = _uv[1];
    }
    else
    {
        double limits[4];
        UF_CALL(::UF_MODL_ask_face_uv_minmax(face,limits));

        if ( (limits[0] < _uv[0] || CHECK_DIFFERENCE(limits[0],_uv[0],tol)) &&
             (limits[1] > _uv[0] || CHECK_DIFFERENCE(limits[1],_uv[0],tol)) &&
             (limits[2] < _uv[1] || CHECK_DIFFERENCE(limits[2],_uv[1],tol)) &&
             (limits[3] > _uv[1] || CHECK_DIFFERENCE(limits[3],_uv[1],tol)) )
        {
            uv[0] = _uv[0];
            uv[1] = _uv[1];
        }
        else
        {
            if ( (limits[0] < srf_pos3.uv[0] || CHECK_DIFFERENCE(limits[0],srf_pos3.uv[0],tol)) &&
                 (limits[1] > srf_pos3.uv[0] || CHECK_DIFFERENCE(limits[1],srf_pos3.uv[0],tol)) &&
                 (limits[2] < srf_pos3.uv[1] || CHECK_DIFFERENCE(limits[2],srf_pos3.uv[1],tol)) &&
                 (limits[3] > srf_pos3.uv[1] || CHECK_DIFFERENCE(limits[3],srf_pos3.uv[1],tol)) )
            {
                uv[0] = srf_pos3.uv[0];
                uv[1] = srf_pos3.uv[1];
            }
            else
            {
                Q_ASSERT(false);
            }
        }
    }
}*/

tag_t CreateDatumPointFeature( const QString& name, const Point3D& point )
{
    tag_t scalar_tags[3];
    tag_t tag_point_feature;
    UF_CALL( ::UF_SO_create_scalar_double( ::UF_PART_ask_display_part(), UF_SO_update_within_modeling, point[0],
                                           &scalar_tags[0] ) );
    UF_CALL( ::UF_SO_create_scalar_double( ::UF_PART_ask_display_part(), UF_SO_update_within_modeling, point[1],
                                           &scalar_tags[1] ) );
    UF_CALL( ::UF_SO_create_scalar_double( ::UF_PART_ask_display_part(), UF_SO_update_within_modeling, point[2],
                                           &scalar_tags[2] ) );
    UF_CALL( ::UF_POINT_create_3_scalars( scalar_tags, &tag_point_feature ) );
    UF_CALL( ::UF_OBJ_set_name( tag_point_feature, qPrintable( name ) ) );
    return tag_point_feature;
}

tag_t CreateDatumAxisFeature( const QString& name, const Point3D& point, const Vector3D& direction )
{
    tag_t point_feature = CreateDatumPointFeature( name, point );

    tag_t tag_point;
    UF_CALL( ::UF_POINT_ask_point_output( point_feature, &tag_point ) );

    tag_t tag_direction;
    UF_CALL( ::UF_SO_create_dirr_doubles( ::UF_PART_ask_display_part(), UF_SO_update_within_modeling, ( double* )&direction,
                                          &tag_direction ) );


    tag_t tag_axis;
    UF_CALL( ::UF_MODL_create_point_dirr_daxis( tag_point, tag_direction, &tag_axis ) );

    tag_t axis_feature;
    UF_CALL( ::UF_MODL_ask_object_feat( tag_axis, &axis_feature ) );
    UF_CALL( ::UF_OBJ_set_name( axis_feature, qPrintable( name ) ) );

    return axis_feature;
}

tag_t CreateDatumPlaneFeature( const QString& name, const Point3D& point, const Vector3D& direction )
{
    tag_t point_feature = CreateDatumPointFeature( name, point );

    tag_t tag_point;
    UF_CALL( ::UF_POINT_ask_point_output( point_feature, &tag_point ) );

    tag_t tag_direction;
    UF_CALL( ::UF_SO_create_dirr_doubles( ::UF_PART_ask_display_part(), UF_SO_update_within_modeling, ( double* )&direction,
                                          &tag_direction ) );

    tag_t tag_plane;
    UF_CALL( ::UF_MODL_create_point_dirr_dplane( tag_point, tag_direction, &tag_plane ) );

    tag_t plane_feature;
    UF_CALL( ::UF_MODL_ask_object_feat( tag_plane, &plane_feature ) );
    UF_CALL( ::UF_OBJ_set_name( plane_feature, qPrintable( name ) ) );

    return plane_feature;
}

void FacePointsCloud( tag_t face )
{
    const int n = 10;

    /*UF_EVALSF_p_t evaluator;
    UF_CALL(::UF_EVALSF_initialize(face, &evaluator));

    int U_status, V_status;
    double U_period, V_period;
    UF_CALL(::UF_MODL_ask_face_periodicity(face, &U_status, &U_period, &V_status, &V_period));*/

    double limits[4];
    //UF_CALL(::UF_EVALSF_ask_face_uv_minmax(evaluator,limits));
    UF_CALL( ::UF_MODL_ask_face_uv_minmax( face, limits ) );

    /*if ( U_status == UF_MODL_NON_PERIODIC )
    {
        limits[0] = 0.;
        limits[1] = 1.;
    }
    if ( V_status == UF_MODL_NON_PERIODIC )
    {
        limits[2] = 0.;
        limits[3] = 1.;
    }*/

    double uv[2];
    double ud = ( limits[1] - limits[0] ) / n;
    double vd = ( limits[3] - limits[2] ) / n;
    uv[0] = limits[0];

    for ( int i = 0, t = n + 1; i < t; i++ )
    {
        uv[1] = limits[2];

        for ( int j = 0; j < t; j++ )
        {
            UF_MODL_SRF_VALUE_t evals;
            //UF_CALL(::UF_EVALSF_evaluate(evaluator, UF_MODL_EVAL, uv, &evals));
            //UF_CALL(::UF_MODL_evaluate_face(face, UF_MODL_EVAL, uv, &evals));

            double p[3], r[2];
            UF_CALL( ::UF_MODL_ask_face_props( face, uv, evals.srf_pos, p, p, p, p, p, r ) );

            CreateDatumPointFeature( QString( "SURFPOINT%1" ).arg( face ), Point3D( evals.srf_pos ) );
            uv[1] += vd;
        }

        uv[0] += ud;
    }

    //UF_CALL(::UF_EVALSF_free(&evaluator));
}

int EvaluateLoopBasePoint( const uf_loop_p_t loop, LoopBasePointInfo& loopBasePointInfo )
{
    int code = 0;
    //Point3D pointOnCurve, vertex[2];

    /*int edgesCount;
    UF_CALL(::UF_MODL_ask_list_count(loop->edge_list,&edgesCount));
    std::vector<tag_t> loopEdges;
    loopEdges.reserve(edgesCount);*/

    uf_list_p_t edge = loop->edge_list;

    if ( edge == 0x0 )
        return code;

    LoopBasePointInfo currentLoopBasePointInfo;

    currentLoopBasePointInfo.LoadEdgeInfo( 0, edge->eid );

    EdgeInfo firstEdgeInfo = currentLoopBasePointInfo.adjEdges[0];

    for ( edge = edge->next; edge != 0x0; edge = edge->next )
    {
        currentLoopBasePointInfo.LoadEdgeInfo( 1, edge->eid );

        if ( ( code = currentLoopBasePointInfo.FindBasePoint() ) )
            return code;

        if ( currentLoopBasePointInfo.point < loopBasePointInfo.point &&
                currentLoopBasePointInfo.CalcCutPlane() )
            loopBasePointInfo = currentLoopBasePointInfo;

        currentLoopBasePointInfo.MoveNextEdge();
    }

    //Check first and last edges of loop
    currentLoopBasePointInfo.adjEdges[1] = firstEdgeInfo;

    if ( ( code = currentLoopBasePointInfo.FindBasePoint() ) )
        return code;

    if ( currentLoopBasePointInfo.point < loopBasePointInfo.point &&
            currentLoopBasePointInfo.CalcCutPlane() )
        loopBasePointInfo = currentLoopBasePointInfo;

    return code;

    /*Point3D prevEdgeVertexes[2];
    for (uf_list_p_t edge = loop->edge_list; edge != 0x0; edge = edge->next)
    {
        //loopEdges.push_back(edge->eid);
        UF_EVAL_p_t evaluator;
        UF_CALL(::UF_EVAL_initialize(edge->eid, &evaluator));

        double limits [2];
        UF_CALL(::UF_EVAL_ask_limits(evaluator, limits));

        int edgeType;
        UF_CALL(::UF_MODL_ask_edge_type(edge->eid, &edgeType));

        double tParamOnCurve;
        int result = MINIMUM_ON_BOUNDARY;
        logical is_periodic = false;
        if ( edgeType != UF_MODL_LINEAR_EDGE )
        {
            UF_CALL(::UF_EVAL_is_periodic(evaluator, &is_periodic));

            result = Evaluate_t_Snap(evaluator, &tParamOnCurve, limits, pointOnCurve, is_periodic);
        }

        if ( result == MINIMUM_ON_BOUNDARY )
        {
            UF_CALL(::UF_EVAL_evaluate(evaluator, 0, limits[0], (double*)&vertexes[0], 0x0));

            int n;
            if ( !is_periodic )
            {
                UF_CALL(::UF_EVAL_evaluate(evaluator, 0, limits[1], (double*)&vertexes[1], 0x0));
                n = 2;
            }
            else
                n = 1;

            if ( currentLoopBasePointInfo.adjEdges[0] == NULL_TAG )
            {
                for (int i = 0; i < n; i++)
                {
                    prevEdgeVertexes[i] = vertexes[i];
                    if ( vertexes[i] <= currentLoopBasePointInfo.point )
                    {
                        currentLoopBasePointInfo.point = vertexes[i];
                        currentLoopBasePointInfo.adjEdges[0] = edge->eid;
                        currentLoopBasePointInfo.tParamBasePoint[0] = limits[i];
                        currentLoopBasePointInfo.minmax[0] = i;
                    }
                }
                continue;
            }

            Point3D currentBasePoint;
            for (int i = 0; i < n; i++)
                for(int j = 0; j < n; j++)
                    if ( vertexes[j] == prevEdgeVertexes[i] )
                    {
                        currentLoopBasePointInfo.point = vertexes[i];
                        currentLoopBasePointInfo.adjEdges[1] = currentLoopBasePointInfo.adjEdges[0];
                        currentLoopBasePointInfo.adjEdges[0] = edge->eid;
                        currentLoopBasePointInfo.tParamBasePoint[1] = currentLoopBasePointInfo.tParamBasePoint[0];
                        currentLoopBasePointInfo.tParamBasePoint[0] = limits[i];
                        currentLoopBasePointInfo.minmax[1] = currentLoopBasePointInfo.minmax[0];
                        currentLoopBasePointInfo.minmax[0] = i;
                    }

            if ( basePointFound )
            {
                currentLoopBasePointInfo.point = currentBasePoint;
                currentLoopBasePointInfo.adjEdges[1] = currentLoopBasePointInfo.adjEdges[0];
                currentLoopBasePointInfo.adjEdges[0] = edge->eid;
                currentLoopBasePointInfo.tParamBasePoint[1] = currentLoopBasePointInfo.tParamBasePoint[0];
                currentLoopBasePointInfo.tParamBasePoint[0] = limits[i];
                currentLoopBasePointInfo.minmax[1] = currentLoopBasePointInfo.minmax[0];
                currentLoopBasePointInfo.minmax[0] = i;
            }

            if ( currentLoopBasePointInfo.adjEdges[1] != NULL_TAG &&
                 currentLoopBasePointInfo.adjEdges[1] != currentLoopBasePointInfo.adjEdges[0] &&
                 CreateCutPlane(currentLoopBasePointInfo) )
            {
                loopBasePointInfo = currentLoopBasePointInfo;
            }
        }
        else
        {
            if ( pointOnCurve < currentLoopBasePointInfo.point )
            {
                currentLoopBasePointInfo.point = pointOnCurve;
                currentLoopBasePointInfo.adjEdges[1] = NULL_TAG;
                currentLoopBasePointInfo.adjEdges[0] = edge->eid;
                currentLoopBasePointInfo.tParamBasePoint[0] = tParamOnCurve;
            }
        }

        UF_CALL(::UF_EVAL_free(evaluator));
    }

    if ( currentLoopBasePointInfo.adjEdges[1] == NULL_TAG )
    {
        CreateCutPlane(currentLoopBasePointInfo);
        loopBasePointInfo = currentLoopBasePointInfo;
    }*/

    /*std::sort(loopEdges.begin(),loopEdges.end());

    //Create our loop identificator because it has not in NX API
    loopBasePointInfo.loopID = QCryptographicHash::hash(
        QByteArray((char*)&loopEdges[0],loopEdges.size() * sizeof(uint)), QCryptographicHash::Md5).toHex().data();*/
}

/*void TraverseTopology(const tag_t tagFace, LoopBasePointInfo& loopBasePointInfo, UF_BREP_topology_p_t child_topology)
{
    if ( child_topology->type == UF_BREP_FACE_TOPO && child_topology->tag == tagFace )
    {
        for(int grand_child_ndx = 0; grand_child_ndx < child_topology->num_children; grand_child_ndx++)
        {
            Q_ASSERT(child_topology->children[grand_child_ndx].child->type == UF_BREP_LOOP_TOPO);

            UF_BREP_topology_p_t loop = child_topology->children[grand_child_ndx].child;

            std::vector<tag_t> loopEdges;
            loopEdges.reserve(loop->num_children);

            for(int grand_child_ndx = 0; grand_child_ndx < loop->num_children; grand_child_ndx++)
            {
                Q_ASSERT(loop->children[grand_child_ndx].child->type == UF_BREP_FIN_TOPO);
                Q_ASSERT(loop->children[grand_child_ndx].child->num_children == 1);

                loopEdges.push_back(loop->children[grand_child_ndx].child->children[0].child->tag);
            }

            std::sort(loopEdges.begin(),loopEdges.end());

            std::string loopID = QCryptographicHash::hash(
                        QByteArray((char*)&loopEdges[0],loopEdges.size() * sizeof(uint)),
                        QCryptographicHash::Md5).toHex().data();

            if ( loopBasePointInfo.loopID == loopID )
            {
                for(int grand_child_ndx = 0; grand_child_ndx < loop->num_children; grand_child_ndx++)
                {
                    Q_ASSERT(loop->children[grand_child_ndx].child->children[0].orientation !=
                             UF_BREP_ORIENTATION_NONE);

                    if ( loop->children[grand_child_ndx].child->children[0].child->tag ==
                         loopBasePointInfo.adjEdge[0] )
                    {
                        loopBasePointInfo.orientation[0] =
                            loop->children[grand_child_ndx].child->children[0].orientation;
                    }
                    else if ( loop->children[grand_child_ndx].child->children[0].child->tag ==
                              loopBasePointInfo.adjEdge[1] )
                    {
                        loopBasePointInfo.orientation[1] =
                            loop->children[grand_child_ndx].child->children[0].orientation;
                    }
                }
            }
        }
        return;
    }

    for(int grand_child_ndx = 0; grand_child_ndx < child_topology->num_children; grand_child_ndx++)
    {
        TraverseTopology(tagFace, loopBasePointInfo, child_topology->children[grand_child_ndx].child);
    }
}

void ExtractTopologyInfo(const tag_t tagFace, LoopBasePointInfo& loopBasePointInfo)
{
    tag_t body;
    UF_BREP_topology_p_t topo;
    UF_BREP_state_p_t states;
    int nstates;

    UF_CALL(::UF_MODL_ask_edge_body(loopBasePointInfo.adjEdge[0],&body));

    UF_CALL(::UF_BREP_ask_topology(body,0,&topo,&nstates,&states));

    TraverseTopology(tagFace, loopBasePointInfo, topo);

    UF_CALL(::UF_BREP_release_topology(topo,0x0));
}*/

bool TraverseTopology( const tag_t tagFace, const tag_t tagEdge, int& orientation, UF_BREP_topology_p_t child_topology )
{
    if ( child_topology->type == UF_BREP_FACE_TOPO && child_topology->tag == tagFace )
    {
        for ( int grand_child_ndx_loop = 0; grand_child_ndx_loop < child_topology->num_children; grand_child_ndx_loop++ )
        {
            Q_ASSERT( child_topology->children[grand_child_ndx_loop].child->type == UF_BREP_LOOP_TOPO );

            UF_BREP_topology_p_t loop = child_topology->children[grand_child_ndx_loop].child;

            for ( int grand_child_ndx = 0; grand_child_ndx < loop->num_children; grand_child_ndx++ )
            {
                //Q_ASSERT(loop->children[grand_child_ndx].child->children[0].orientation !=
                //         UF_BREP_ORIENTATION_NONE);

                if ( loop->children[grand_child_ndx].child->type == UF_BREP_FIN_TOPO &&
                        loop->children[grand_child_ndx].child->children[0].child->tag == tagEdge )
                {
                    orientation =
                        ( loop->children[grand_child_ndx].child->children[0].orientation == UF_BREP_ORIENTATION_FORWARD ) ?
                        1 : -1;
                    return false;
                }
            }
        }
    }

    for ( int grand_child_ndx = 0; grand_child_ndx < child_topology->num_children; grand_child_ndx++ )
    {
        if ( !TraverseTopology( tagFace, tagEdge, orientation, child_topology->children[grand_child_ndx].child ) )
            return false;
    }

    return true;
}

void ExtractEdgeOrientation( const tag_t tagFace, const tag_t tagEdge, int& orientation )
{
    tag_t body;
    UF_BREP_topology_p_t topo;
    UF_BREP_state_p_t states;
    int nstates;

    UF_CALL( ::UF_MODL_ask_edge_body( tagEdge, &body ) );

    UF_CALL( ::UF_BREP_ask_topology( body, 0, &topo, &nstates, &states ) );

    TraverseTopology( tagFace, tagEdge, orientation, topo );

    UF_CALL( ::UF_BREP_release_topology( topo, 0x0 ) );

}

void GoldenSectionAllRoots( const UF_EVAL_p_t evaluator, const double* product, double a, double b,
                            const Point3D& intersectPointA, const Point3D& intersectPointB,
                            Points3DCollection& intersectionPointsCollection, double x_tolerance, double y_tolerance )
{
    Point3D rootPoint;
    double t, lastObjectiveValue;
    int result = GoldenSection( evaluator, product, &t, a, b, rootPoint, lastObjectiveValue, x_tolerance );

    double realDistanceTolerance = 4. * y_tolerance * y_tolerance;

    if ( lastObjectiveValue > realDistanceTolerance )
        return;

    if ( rootPoint.IsClosedTo( intersectPointA, y_tolerance ) ||
            rootPoint.IsClosedTo( intersectPointB, y_tolerance ) )
    {
        Point3D _rootPoint( rootPoint );
        result = GoldenSectionMax( evaluator, product, &t, a, b, rootPoint, lastObjectiveValue, x_tolerance );

        if ( result == MAXIMUM_EXISTS )
        {
            GoldenSectionAllRoots( evaluator, product, a, t, intersectPointA, rootPoint, intersectionPointsCollection,
                                   x_tolerance, y_tolerance );
            GoldenSectionAllRoots( evaluator, product, t, b, rootPoint, intersectPointB, intersectionPointsCollection,
                                   x_tolerance, y_tolerance );
        }
        else
        {
            Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                                               t = intersectionPointsCollection.end();

            for ( ; it != t; ++it )
                if ( it->IsClosedTo( _rootPoint, y_tolerance ) )
                    break;

            if ( it == t )
                intersectionPointsCollection.push_back( _rootPoint );
        }

        return;
    }

    if ( result == MINIMUM_EXISTS )
    {
        //CreateDatumPointFeature(QString("INTERSECT%1").arg((int)evaluator),rootPoint);
        intersectionPointsCollection.push_back( rootPoint );
        GoldenSectionAllRoots( evaluator, product, a, t, intersectPointA, rootPoint,
                               intersectionPointsCollection, x_tolerance, y_tolerance );
        GoldenSectionAllRoots( evaluator, product, t, b, rootPoint, intersectPointB,
                               intersectionPointsCollection, x_tolerance, y_tolerance );
    }
    else
    {
        if ( result == MINIMUM_ON_BOUNDARY /*&&
             !rootPoint.IsClosedTo(intersectPointA) &&
             !rootPoint.IsClosedTo(intersectPointB)*/ )
        {
            //CreateDatumPointFeature(QString("INTERSECT%1").arg((int)evaluator),rootPoint);
            Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                                               t = intersectionPointsCollection.end();

            for ( ; it != t; ++it )
                if ( it->IsClosedTo( rootPoint, y_tolerance ) )
                    break;

            if ( it == t )
                intersectionPointsCollection.push_back( rootPoint );
        }
    }
}

void EdgeCutPlaneIntersect( const tag_t edge,
                            Points3DCollection& intersectionPointsCollection, const LoopBasePointInfo& loopBasePointInfo )
{
    int edgeType;
    UF_CALL( ::UF_MODL_ask_edge_type( edge, &edgeType ) );

    bool basePointOnEdge = loopBasePointInfo.adjEdges[loopBasePointInfo.index[0]].minmax == -1;

    if ( edgeType == UF_MODL_LINEAR_EDGE && !basePointOnEdge &&
            ( edge == loopBasePointInfo.adjEdges[0].id || edge == loopBasePointInfo.adjEdges[1].id ) )
        return;

    bool oneEdge = basePointOnEdge && edge == loopBasePointInfo.adjEdges[loopBasePointInfo.index[0]].id;

    int vertex_count;
    double ( *points )[3];
    UF_CALL( ::UF_MODL_ask_curve_points( edge, 0, 0, 0, &vertex_count, ( double** )&points ) );

    double curveLength = 0.;

    for ( int i = 0, t = vertex_count - 1; i < t; i++ )
        curveLength += Point3D( ( double* )points[i] ).DistanceTo( Point3D( ( double* )points[i + 1] ) );

    ::UF_free( points );

    UF_EVAL_p_t evaluator;
    UF_CALL( ::UF_EVAL_initialize( edge, &evaluator ) );

    double limits [2];
    UF_CALL( ::UF_EVAL_ask_limits( evaluator, limits ) );

    Vector3D normal( loopBasePointInfo.cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {loopBasePointInfo.cutPlane[0] / q, loopBasePointInfo.cutPlane[1] / q,
                         loopBasePointInfo.cutPlane[2] / q, loopBasePointInfo.cutPlane[3] / q
                        };
    double bound[3] = {limits[0], oneEdge ?
                       loopBasePointInfo.adjEdges[loopBasePointInfo.index[0]].tParamBasePoint : limits[1], limits[1]
                      };

    for ( int i = 0, n = ( oneEdge ? 2 : 1 ); i < n; i++ )
    {
        GoldenSectionAllRoots( evaluator, product, bound[i], bound[i + 1],
                               loopBasePointInfo.point, loopBasePointInfo.point,
                               intersectionPointsCollection, ( limits[1] - limits[0] ) * 1.e-2 * e_percent,
                               curveLength * 1.e-2 * e_percent );
    }

    UF_CALL( ::UF_EVAL_free( evaluator ) );
}

void EdgeCutPlaneIntersect( const tag_t edge,
                            Points3DCollection& intersectionPointsCollection, const double* cutPlane )
{
    int vertex_count;
    double ( *points )[3];
    UF_CALL( ::UF_MODL_ask_curve_points( edge, 0, 0, 0, &vertex_count, ( double** )&points ) );

    double curveLength = 0.;

    for ( int i = 0, t = vertex_count - 1; i < t; i++ )
        curveLength += Point3D( ( double* )points[i] ).DistanceTo( Point3D( ( double* )points[i + 1] ) );

    ::UF_free( points );

    UF_EVAL_p_t evaluator;
    UF_CALL( ::UF_EVAL_initialize( edge, &evaluator ) );

    double limits [2];
    UF_CALL( ::UF_EVAL_ask_limits( evaluator, limits ) );

    Vector3D normal( cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {cutPlane[0] / q, cutPlane[1] / q, cutPlane[2] / q, cutPlane[3] / q};

    Point3D point[2];
    GoldenSectionAllRoots( evaluator, product, limits[0], limits[1], point[0], point[1],
                           intersectionPointsCollection, ( limits[1] - limits[0] ) * 1.e-2 * e_percent,
                           curveLength * 1.e-2 * e_percent );

    UF_CALL( ::UF_EVAL_free( evaluator ) );
}

double faceObjective( const tag_t face, double* uv, int xyzIndex )
{
    UF_MODL_SRF_VALUE_t evals;
    //UF_CALL(::UF_EVALSF_evaluate(evaluator, UF_MODL_EVAL, uv, &evals));
    //UF_CALL(::UF_MODL_evaluate_face(face, UF_MODL_EVAL, uv, &evals));
    double p[3], r[2];
    UF_CALL( ::UF_MODL_ask_face_props( face, uv, evals.srf_pos, p, p, p, p, p, r ) );
    return evals.srf_pos[xyzIndex];
}

double intersectObjective( const tag_t face, double* uv, const Point3D& middlePoint,
                           const Point3D& lineNormal, Point3D& last )
{
    UF_MODL_SRF_VALUE_t evals;
    //UF_CALL(::UF_EVALSF_evaluate(evaluator, UF_MODL_EVAL, uv, &evals));
    //UF_CALL(::UF_MODL_evaluate_face(face, UF_MODL_EVAL, uv, &evals));
    double p[3], r[2];
    UF_CALL( ::UF_MODL_ask_face_props( face, uv, evals.srf_pos, p, p, p, p, p, r ) );
    Point3D surfacePoint( evals.srf_pos );
    last = surfacePoint;
    return ( ( surfacePoint - middlePoint ) ^ lineNormal ).Length() / lineNormal.Length();
}

void EvaluateFacePlaneIntersectionPoint( const tag_t face, const double* cutPlane, Point3D& surfacePoint )
{
    Vector3D normal( cutPlane );
    double q = ::sqrt( normal * normal );
    double product[4] = {cutPlane[0] / q, cutPlane[1] / q, cutPlane[2] / q, cutPlane[3] / q};

    double limits[4];
    UF_CALL( ::UF_MODL_ask_face_uv_minmax( face, limits ) );

    double uv[2] = {( limits[0] + limits[1] ) / 2., ( limits[2] + limits[3] ) / 2.};

    for ( int i = 0, j = 0; i < 2; i++, j += 2 )
    {
        double lastObjectiveValue;
        int result = GoldenSection( face, uv, product, &uv[i], limits[j], limits[j + 1],
                                    lastObjectiveValue, surfacePoint, ( limits[j + 1] - limits[j] ) * 1.e-2 * e_percent );

        if ( MINIMUM_EXISTS == result )
            break;

        uv[i] = ( limits[j] + limits[j + 1] ) / 2.;
    }
}

double FacePlaneIntersectObjective( const tag_t face, const double* uv, const double* product, Point3D& surfacePoint )
{
    UF_MODL_SRF_VALUE_t evals;
    double p[3], rr[2];
    UF_CALL( ::UF_MODL_ask_face_props( face, ( double* )uv, evals.srf_pos, p, p, p, p, p, rr ) );
    surfacePoint = evals.srf_pos;
    double r = surfacePoint[0] * product[0] + surfacePoint[1] * product[1] + surfacePoint[2] * product[2] + product[3];
    return r * r;
}

Point3D& edgeObjective( const UF_EVAL_p_t evaluator, const double* t, Point3D& point )
{
    UF_CALL( ::UF_EVAL_evaluate( evaluator, 0, *t, ( double* )&point, 0x0 ) );
    return point;
}

double intersectObjective( const UF_EVAL_p_t evaluator, const double* product, const double* t, Point3D& point )
{
    UF_CALL( ::UF_EVAL_evaluate( evaluator, 0, *t, ( double* )&point, 0x0 ) );
    double r = point[0] * product[0] + point[1] * product[1] + point[2] * product[2] + product[3];
    return r * r;
}

int GoldenSection( const tag_t face, const double* uv, const double* product, double* x, double a, double b,
                   double& lastValue, Point3D& surfacePoint, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = FacePlaneIntersectObjective( face, uv, product, surfacePoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( CHECK_DIFFERENCE( *x, inia, e ) || CHECK_DIFFERENCE( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int GoldenSection( const UF_EVAL_p_t evaluator, const double* product, double* x, double a, double b,
                   Point3D& lastPoint, double& lastValue, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( evaluator, product, x, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( evaluator, product, x, lastPoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = intersectObjective( evaluator, product, x, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = intersectObjective( evaluator, product, x, lastPoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( CHECK_DIFFERENCE( *x, inia, e ) || CHECK_DIFFERENCE( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int GoldenSectionMax( const UF_EVAL_p_t evaluator, const double* product, double* x, double a, double b,
                      Point3D& lastPoint, double& lastValue, double e )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( evaluator, product, x, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( evaluator, product, x, lastPoint );

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 > y1 )//maximum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            lastValue = y2 = intersectObjective( evaluator, product, x, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            lastValue = y1 = intersectObjective( evaluator, product, x, lastPoint );
        }
    } while ( ( b - a ) > e );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( CHECK_DIFFERENCE( *x, inia, e ) || CHECK_DIFFERENCE( *x, inib, e ) )
        return MAXIMUM_ON_BOUNDARY;

    return MAXIMUM_EXISTS;
}
int GoldenSection( const UF_EVAL_p_t evaluator, double* x, double a, double b, int xyzIndex, Point3D& last,
                   double root_tolerance, double boundary_tolerance )
{
    double x1, x2, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = edgeObjective( evaluator, x, last )[xyzIndex];
    *x = x2 = a + ( a - b ) * tau;
    double y2 = edgeObjective( evaluator, x, last )[xyzIndex];

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            y2 = edgeObjective( evaluator, x, last )[xyzIndex];
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            y1 = edgeObjective( evaluator, x, last )[xyzIndex];
        }
    } while ( ( b - a ) > root_tolerance );

    *x = ( b + a ) * 0.5;

    //Check curve is on plane so edgeObjective() doesn't change
    if ( integr < e_absolute )
        return CURVE_ON_PLANE;

    //Check root is on boundaries
    if ( CHECK_DIFFERENCE( *x, inia, boundary_tolerance ) || CHECK_DIFFERENCE( *x, inib, boundary_tolerance ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

double GoldenSection( const tag_t face, double* X, double* x, double a, double b, int xyzIndex )
{
    double x1, x2, last;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = faceObjective( face, X, xyzIndex );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = faceObjective( face, X, xyzIndex );

    do
    {
        if ( y2 < y1 )//minimum
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            last = y2 = faceObjective( face, X, xyzIndex );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            last = y1 = faceObjective( face, X, xyzIndex );
        }
    } while ( ( b - a ) > e_absolute );

    *x = ( b + a ) * 0.5;

    return last;
}

int GoldenSection( const tag_t face, double* X, double* x, double a, double b, const Point3D& middlePoint,
                   const Point3D& lineNormal, Point3D& lastPoint, double& last, bool min = true )
{
    double x1, x2, e = ( b - a ) * 1.e-2 * e_percent, inia = a, inib = b, integr = 0.0;

    *x = x1 = b + ( b - a ) * tau;
    double y1 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
    *x = x2 = a + ( a - b ) * tau;
    double y2 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );

    int iterLimit = 100;

    do
    {
        integr += ::fabs( y2 - y1 );

        if ( min ? y2 < y1 : y2 > y1 )
        {
            a = x1;
            x1 = x2;
            y1 = y2;
            *x = x2 = a + ( a - b ) * tau;
            last = y2 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
        }
        else
        {
            b = x2;
            x2 = x1;
            y2 = y1;
            *x = x1 = b + ( b - a ) * tau;
            last = y1 = intersectObjective( face, X, middlePoint, lineNormal, lastPoint );
        }
    } while ( ( b - a ) > e && iterLimit-- > 0 );

    *x = ( b + a ) * 0.5;

    //Check line intersect surface so intersectObjective() is variable
    if ( integr < 1.e-6 )
        return NO_EXTREMUM_EXISTS;

    //Check root is on boundaries
    if ( CHECK_DIFFERENCE( *x, inia, e ) || CHECK_DIFFERENCE( *x, inib, e ) )
        return MINIMUM_ON_BOUNDARY;

    return MINIMUM_EXISTS;
}

int Evaluate_t_Snap( const UF_EVAL_p_t evaluator, double* X, const double* limits, Point3D& point, bool /*periodic*/ )
{
    //TODO find all minimums
    int result;

    for ( int xyzEnum = 0; xyzEnum < 3; xyzEnum++ )
    {
        result = GoldenSection( evaluator, X, limits[0], limits[1], xyzEnum, point,
                                ( limits[1] - limits[0] ) * 1.e-2 * root_tolerance_percent,
                                ( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent );

        //Check curve is on plane so edgeObjective() doesn't change
        if ( result == CURVE_ON_PLANE )
            continue;

        break;
    }

    return result;
}

void EvaluateUVSnap( const tag_t face, double* X, double* limits, bool periodic )
{
    X[0] = ( limits[0] + limits[1] ) * 0.5;
    X[1] = ( limits[2] + limits[3] ) * 0.5;

    for ( int xyzEnum = 0; xyzEnum < 3; xyzEnum++ )
    {
        double first;

        for ( int i = 0, j = 0; i < 2; i++, j += 2 )
            first = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], xyzEnum );

        for ( ;; )
        {
            double last;

            for ( int i = 0, j = 0; i < 2; i++, j += 2 )
                last = GoldenSection( face, X, &X[i], limits[j], limits[j + 1], xyzEnum );

            if ( CHECK_DIFFERENCE( first, last, e_absolute ) )
                break;

            first = last;
        }

        //Check root on boundaries
        if ( CHECK_DIFFERENCE( X[0], limits[0], e_absolute ) && CHECK_DIFFERENCE( X[1], limits[2], e_absolute ) ||
                CHECK_DIFFERENCE( X[0], limits[1], e_absolute ) && CHECK_DIFFERENCE( X[1], limits[3], e_absolute ) )
        {
            if ( periodic )
                break;
        }
        else
            break;
    }
}

void FindClosedIntersectionPoint( const Point3D& basePoint,
                                  const Points3DCollection& intersectionPointsCollection, Points3DCollection::const_iterator& closedPoint )
{
    //TODO REMAKE THIS APROACH. FIND INTERSECT CURVE ROUGHLY AND THAN TAKE ITS END POINT

    double rmin = std::numeric_limits<double>::max();
    /*UF_EVALSF_p_t evaluator;
    UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));

    double uv_min_max[4];
    UF_CALL(::UF_EVALSF_ask_face_uv_minmax(evaluator, uv_min_max));

    UF_EVALSF_pos3_t srf_pos3;
    double uBaseNorm, vBaseNorm, rmin = std::numeric_limits<double>::max();
    UF_CALL(::UF_EVALSF_find_closest_point(evaluator, (double*)&basePoint, &srf_pos3));

    UF_MODL_SRF_VALUE_t evals;
    UF_CALL(::UF_EVALSF_evaluate(evaluator, UF_MODL_EVAL_DERIV1, srf_pos3.uv, &evals));

    Vector3D deriv[2] = {Vector3D((double*)&evals.srf_du), Vector3D((double*)&evals.srf_dv)};

    double du = ::sqrt(deriv[0] * deriv[0]), dv = ::sqrt(deriv[1] * deriv[1]);*/

    /*uBaseNorm = (srf_pos3.uv[0] - uv_min_max[0]) / (uv_min_max[1] - uv_min_max[0]);
    vBaseNorm = (srf_pos3.uv[1] - uv_min_max[2]) / (uv_min_max[3] - uv_min_max[2]);*/

    /*uBaseNorm = srf_pos3.uv[0] * du;
    vBaseNorm = srf_pos3.uv[1] * dv;*/

    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            t = intersectionPointsCollection.end(); i != t; ++i )
    {
        //UF_CALL(::UF_EVALSF_find_closest_point(evaluator, (double*)&*i, &srf_pos3));

        /*UF_CALL(::UF_EVALSF_evaluate(evaluator, UF_MODL_EVAL_DERIV1, srf_pos3.uv, &evals));

        deriv[0] = Vector3D((double*)&evals.srf_du);
        deriv[1] = Vector3D((double*)&evals.srf_dv);

        du = ::sqrt(deriv[0] * deriv[0]), dv = ::sqrt(deriv[1] * deriv[1]);

        double uu = (srf_pos3.uv[0] - uv_min_max[0]) / (uv_min_max[1] - uv_min_max[0]) - uBaseNorm,
               vv = (srf_pos3.uv[1] - uv_min_max[2]) / (uv_min_max[3] - uv_min_max[2]) - vBaseNorm;

        double r = uu * uu * du + vv * vv * dv;*/

        /*double uu = srf_pos3.uv[0] * du - uBaseNorm,
               vv = srf_pos3.uv[1] * dv - vBaseNorm;

        double r = uu * uu + vv * vv;*/

        double r = basePoint.DistanceTo( *i );

        if ( r < rmin )
        {
            closedPoint = i;
            rmin = r;
        }
    }

    //UF_CALL(::UF_EVALSF_free(&evaluator));

    //FacePointsCloud(tagFace);

    /*double limits[4];
    UF_CALL(::UF_MODL_ask_face_uv_minmax(tagFace,limits));

    double p[3],uvPoint[2];
    UF_CALL(::UF_MODL_ask_face_parm(tagFace, (double*)&basePoint, uvPoint, p));

    double curvature[2];
    Vector3D du,dv,ddu,ddv,surfaceNormal;
    UF_CALL(::UF_MODL_ask_face_props(tagFace,uvPoint,p,
                                     (double*)&du,(double*)&dv,(double*)&ddu,(double*)&ddv,
                                     (double*)&surfaceNormal,curvature));

    Vector3D planeNormal(cutPlane);
    Vector3D t = surfaceNormal ^ planeNormal;*/

    /*double usign = du * t, ue = (limits[1] - limits[0]) * 1.e-2 * e_percent,
           vsign = dv * t, ve = (limits[3] - limits[2]) * 1.e-2 * e_percent;

    if ( usign > 0 && CHECK_DIFFERENCE(uvPoint[0], limits[1], ue) ||
         usign < 0 && CHECK_DIFFERENCE(uvPoint[0], limits[0], ue) ||
         vsign > 0 && CHECK_DIFFERENCE(uvPoint[1], limits[3], ve) ||
         vsign < 0 && CHECK_DIFFERENCE(uvPoint[1], limits[2], ve) )
        t.Invert();*/

    /*CreateDatumAxisFeature(QString("DIRECTION%1").arg(tagFace),basePoint,t);

    double delta = rmin * 1.e-3;

    double denom = surfaceNormal * ddu;

    double deltaU;
    if( denom < e_absolute )
        deltaU = std::numeric_limits<double>::max();
    else
        deltaU = ::fabs(delta * du.Length() / denom);*/

    //return closedPoint;
}

void FindClosedIntersectionPoint( const Point3D& basePoint, Vector3D dir,
                                  const Points3DCollection& intersectionPointsCollection, Points3DCollection::const_iterator& closedPoint )
{
    closedPoint = intersectionPointsCollection.end();
    double rmin = std::numeric_limits<double>::max();

    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            t = intersectionPointsCollection.end(); i != t; ++i )
    {
        Vector3D v = ( *i - basePoint );
        v.Norm();

        if ( !v.IsClosedTo2( dir ) )
            continue;

        double r = basePoint.DistanceTo( *i );

        if ( r < rmin )
        {
            closedPoint = i;
            rmin = r;
        }
    }
}

void FindAllLineFaceIntersections( const tag_t face, const Vector3D& middlePoint,
                                   const Point3D& lineNormal, Points3DCollection& intersectionPointsCollection,
                                   double* limits, double* toplimits, double* etop, logical* periodic )
{
    double X[2] = {( limits[0] + limits[1] ) * 0.5, ( limits[2] + limits[3] ) * 0.5};

    Point3D intersectionPoint;
    double first;
    int result;

    for ( int i = 0, j = 0; i < 2; i++, j += 2 )
    {
        result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1],
                                middlePoint, lineNormal, intersectionPoint, first, true );

        if ( NO_EXTREMUM_EXISTS == result )
            return;
    }

    double last;

    for ( ;; )
    {
        for ( int i = 0, j = 0; i < 2; i++, j += 2 )
        {
            result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1],
                                    middlePoint, lineNormal, intersectionPoint, last, true );

            if ( NO_EXTREMUM_EXISTS == result )
                return;
        }

        if ( CHECK_DIFFERENCE( first, last, e_absolute ) )
        {
            //if ( last > 5.e-2 ) //tolerance of intersection (distance between point on surface and intersection line)
            //TODO it really depends on first derivation of U (or V)
            //so the tolerance must be calculated dynamically
            //return;

            break;
        }

        first = last;
    }

    double e[2] = {( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent,
                   ( limits[3] - limits[2] ) * 1.e-2 * boundary_tolerance_percent
                  };

    if ( ( ( CHECK_DIFFERENCE( X[0], limits[0], e[0] ) || CHECK_DIFFERENCE( X[0], limits[1], e[0] ) ) &&
            !( periodic[0] && ( CHECK_DIFFERENCE( X[0], toplimits[0], etop[0] ) ||
                                CHECK_DIFFERENCE( X[0], toplimits[1], etop[0] ) ) ) ) &&
            ( CHECK_DIFFERENCE( X[1], limits[2], e[1] ) || CHECK_DIFFERENCE( X[1], limits[3], e[1] ) ) &&
            !( periodic[1] && ( CHECK_DIFFERENCE( X[1], toplimits[2], etop[1] ) ||
                                CHECK_DIFFERENCE( X[1], toplimits[3], etop[1] ) ) ) )
    {
        X[0] = ( limits[0] + limits[1] ) * 0.5;
        X[1] = ( limits[2] + limits[3] ) * 0.5;

        for ( int i = 0, j = 0; i < 2; i++, j += 2 )
        {
            result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1],
                                    middlePoint, lineNormal, intersectionPoint, first, false );

            if ( NO_EXTREMUM_EXISTS == result )
                return;
        }

        double last;

        for ( ;; )
        {
            for ( int i = 0, j = 0; i < 2; i++, j += 2 )
            {
                result = GoldenSection( face, X, &X[i], limits[j], limits[j + 1],
                                        middlePoint, lineNormal, intersectionPoint, last, false );

                if ( NO_EXTREMUM_EXISTS == result )
                    return;
            }

            if ( CHECK_DIFFERENCE( first, last, e_absolute ) )
                break;

            first = last;
        }

        if ( ( CHECK_DIFFERENCE( X[0], limits[0], e[0] ) || CHECK_DIFFERENCE( X[0], limits[1], e[0] ) ) &&
                ( CHECK_DIFFERENCE( X[1], limits[2], e[1] ) || CHECK_DIFFERENCE( X[1], limits[3], e[1] ) ) )
            return;
    }
    else
    {
        if ( std::find( intersectionPointsCollection.begin(), intersectionPointsCollection.end(), intersectionPoint ) ==
                intersectionPointsCollection.end() && !( last > 5.e-2 ) )
            intersectionPointsCollection.push_back( intersectionPoint );

        if ( ( periodic[0] && ( CHECK_DIFFERENCE( X[0], toplimits[0], etop[0] ) ||
                                CHECK_DIFFERENCE( X[0], toplimits[1], etop[0] ) ) ) ||
                ( periodic[1] && ( CHECK_DIFFERENCE( X[1], toplimits[2], etop[1] ) ||
                                   CHECK_DIFFERENCE( X[1], toplimits[3], etop[1] ) ) ) )
            return;
    }

    double roi1[4] = {limits[0], X[0], limits[2], X[1]};
    double roi2[4] = {X[0], limits[1], X[1], limits[3]};
    double roi3[4] = {limits[0], X[0], X[1], limits[3]};
    double roi4[4] = {X[0], limits[1], limits[2], X[1]};

    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi1, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi2, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi3, toplimits, etop,
                                  periodic );
    FindAllLineFaceIntersections( face, middlePoint, lineNormal, intersectionPointsCollection, roi4, toplimits, etop,
                                  periodic );
}

bool FindLineFaceIntersect( tag_t tagFace, const Vector3D& lineNormal,
                            const Point3D& middlePoint, Point3D& intersectionPoint )
{
    /*UF_EVALSF_p_t evaluator;
    UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));*/

    int U_status, V_status;
    double U_period, V_period;
    UF_CALL( ::UF_MODL_ask_face_periodicity( tagFace, &U_status, &U_period, &V_status, &V_period ) );
    logical periodic[2] = {U_status == UF_MODL_PERIODIC, V_status == UF_MODL_PERIODIC};

    double limits[4];
    //UF_CALL(::UF_EVALSF_ask_face_uv_minmax(evaluator,limits));

    UF_CALL( ::UF_MODL_ask_face_uv_minmax( tagFace, limits ) );

    /*UF_EVALSF_p_t evaluator;
    UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));
    UF_EVALSF_pos3_s srf_pos3;
    UF_CALL(::UF_EVALSF_ask_minimum_face_dist(evaluator,(double*)&middlePoint,&srf_pos3));
    UF_CALL(::UF_EVALSF_free(&evaluator));*/
    //Returned limits may be wrong for non-periodic directions, so we always change it
    /*if ( U_status == UF_MODL_NON_PERIODIC )
    {
        limits[0] = 0.;
        limits[1] = 1.;
    }
    if ( V_status == UF_MODL_NON_PERIODIC )
    {
        limits[2] = 0.;
        limits[3] = 1.;
    }*/

    //if (tagFace == 28966/*28932*//*28925*/)
    //    FacePointsCloud(tagFace);

    Points3DCollection intersectionPointsCollection;
    double e[2] = {( limits[1] - limits[0] ) * 1.e-2 * boundary_tolerance_percent,
                   ( limits[3] - limits[2] ) * 1.e-2 * boundary_tolerance_percent
                  };
    FindAllLineFaceIntersections( tagFace, middlePoint, lineNormal, intersectionPointsCollection, limits, limits, e,
                                  periodic );

    if ( !intersectionPointsCollection.empty() )
    {
        Points3DCollection::const_iterator closestPointIterator;
        FindClosedIntersectionPoint( middlePoint, intersectionPointsCollection, closestPointIterator );
        intersectionPoint = *closestPointIterator;
        return true;
    }
    else
        return false;

    //X[0] = (limits[0] + limits[1]) * 0.5;
    //X[1] = (limits[2] + limits[3]) * 0.5;
    //X[0] = srf_pos3.uv[0];
    //X[1] = srf_pos3.uv[1];

    /*double first;
    int result;
    for(int i = 0, j = 0; i < 2; i++, j += 2)
    {
        result = GoldenSection(tagFace, X, &X[i], limits[j], limits[j+1],
                               middlePoint, lineNormal, intersectionPoint, first);
        if ( NO_EXTREMUM_EXISTS == result )
            return false;
        if ( first < e_absolute )
            return true;
    }

    double last;
    for(;;)
    {
        for(int i = 0, j = 0; i < 2; i++, j += 2)
        {
            result = GoldenSection(tagFace, X, &X[i], limits[j], limits[j+1],
                                   middlePoint, lineNormal, intersectionPoint, last);
            if ( NO_EXTREMUM_EXISTS == result )
                return false;
            if ( last < e_absolute )
                return true;
        }

        if ( CHECK_DIFFERENCE(first,last,e_absolute) )
            return true;

        first = last;
    }*/

    //UF_CALL(::UF_EVALSF_free(&evaluator));

    //return last < e_absolute;
}

void test()
{
    {
        //Sort test
        Points3DCollection c;

        c.push_back( Point3D( 0, -1, -2 ) );
        c.push_back( Point3D( -2, -1, -2 ) );
        c.push_back( Point3D( -1, -2, -2 ) );
        c.push_back( Point3D( 0, -2, -2 ) );
        c.push_back( Point3D( 1, -1, -2 ) );
        c.push_back( Point3D( -2, -2, 0 ) );
        c.push_back( Point3D( 1, -1, -1 ) );

        std::sort( c.begin(), c.end() );

        Points3DCollection etalon;

        etalon.push_back( Point3D( -2, -2, 0 ) );
        etalon.push_back( Point3D( -2, -1, -2 ) );
        etalon.push_back( Point3D( -1, -2, -2 ) );
        etalon.push_back( Point3D( 0, -2, -2 ) );
        etalon.push_back( Point3D( 0, -1, -2 ) );
        etalon.push_back( Point3D( 1, -1, -2 ) );
        etalon.push_back( Point3D( 1, -1, -1 ) );

        if ( c != etalon )
            throw;
    }

    {
        //Sort test 2
        Points3DCollection c;

        c.push_back( Point3D( 0, 2, 0 ) );
        c.push_back( Point3D( 0, 0, 0 ) );
        c.push_back( Point3D( 0, -1, 0 ) );
        c.push_back( Point3D( 0, 1, 0 ) );

        std::sort( c.begin(), c.end() );

        Points3DCollection etalon;

        etalon.push_back( Point3D( 0, -1, 0 ) );
        etalon.push_back( Point3D( 0, 0, 0 ) );
        etalon.push_back( Point3D( 0, 1, 0 ) );
        etalon.push_back( Point3D( 0, 2, 0 ) );

        if ( c != etalon )
            throw;
    }

    {
        //Sort test 3
        Points3DCollection c;

        c.push_back( Point3D( 0, 0, 2 ) );
        c.push_back( Point3D( 0, 0, 0 ) );
        c.push_back( Point3D( 0, 0, -1 ) );
        c.push_back( Point3D( 0, 0, 1 ) );

        std::sort( c.begin(), c.end() );

        Points3DCollection etalon;

        etalon.push_back( Point3D( 0, 0, -1 ) );
        etalon.push_back( Point3D( 0, 0, 0 ) );
        etalon.push_back( Point3D( 0, 0, 1 ) );
        etalon.push_back( Point3D( 0, 0, 2 ) );

        if ( c != etalon )
            throw;
    }
}

void EvaluateEdgeSnapPoint( const tag_t tagEdge, double* point )
{
    UF_EVAL_p_t evaluator;
    UF_CALL( ::UF_EVAL_initialize( tagEdge, &evaluator ) );

    logical is_periodic;
    UF_CALL( ::UF_EVAL_is_periodic( evaluator, &is_periodic ) );

    double limits[2];
    UF_CALL( ::UF_EVAL_ask_limits( evaluator, limits ) );

    if ( is_periodic )
    {
        double tParamOnCurve;
        Evaluate_t_Snap( evaluator, &tParamOnCurve, limits, *( ( Point3D* )point ), is_periodic );
    }
    else
        UF_CALL( ::UF_EVAL_evaluate( evaluator, 0, ( limits[0] + limits[1] ) * 0.5, point, 0x0 ) );

    UF_CALL( ::UF_EVAL_free( evaluator ) );
}

int EvaluateFaceSnapPoint( const tag_t tagFace, Point3D& point )
{
    test();

    int code = 0;
    uf_loop_p_t loopsList;
    UF_CALL( ::UF_MODL_ask_face_loops( tagFace,
                                       &loopsList ) );

    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_face_edges( tagFace,
                                       &edgesList ) );

    uf_loop_p_t loop = 0x0;

    for ( loop = loopsList; loop != 0x0; loop = loop->next )
    {
        if ( loop->type == 1 )
            break;
    }

    LoopBasePointInfo loopBasePointInfo;

    if ( loop && loop->edge_list )
    {
        if ( ( code = EvaluateLoopBasePoint( loop, loopBasePointInfo ) ) )
            return code;
    }
    else
    {
        if ( !edgesList )
        {
            /*UF_EVALSF_p_t evaluator;
            UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));*/

            double limits[4];
            //UF_CALL(::UF_EVALSF_ask_face_uv_minmax(evaluator,limits));
            UF_CALL( ::UF_MODL_ask_face_uv_minmax( tagFace, limits ) );

            double uv[2];
            EvaluateUVSnap( tagFace, uv, limits, true );

            //UF_MODL_SRF_VALUE_t evals;
            //UF_CALL(::UF_EVALSF_evaluate(evaluator,UF_MODL_EVAL, uv, &evals));
            //UF_CALL(::UF_MODL_evaluate_face(tagFace, UF_MODL_EVAL, uv, &evals));
            double p[3], r[2];
            UF_CALL( ::UF_MODL_ask_face_props( tagFace, uv, ( double* )&point, p, p, p, p, p, r ) );

            //CreateDatumPointFeature(QString("SNAP%1").arg(tagFace),point);

            //uv[0] = limits[0];
            //uv[1] = limits[2];
            //UF_CALL(::UF_EVALSF_evaluate(evaluator,UF_MODL_EVAL, uv, &evals));
            //UF_CALL(::UF_MODL_evaluate_face(tagFace, UF_MODL_EVAL, uv, &evals));
            //UF_CALL(::UF_MODL_ask_face_props(tagFace,uv,evals.srf_pos,p,p,p,p,p,r));

            //CreateDatumPointFeature(QString("ORIGIN%1").arg(tagFace),Point3D(evals.srf_pos));

            //UF_CALL(::UF_EVALSF_free(&evaluator));
            return code;
        }
        else
        {
            for ( loop = loopsList; loop != 0x0; loop = loop->next )
            {
                LoopBasePointInfo prevLoopBasePointInfo;

                if ( ( code = EvaluateLoopBasePoint( loop, prevLoopBasePointInfo ) ) )
                    return code;

                if ( prevLoopBasePointInfo.point < loopBasePointInfo.point )
                    loopBasePointInfo = prevLoopBasePointInfo;
            }
        }
    }

    //ExtractTopologyInfo(tagFace, loopBasePointInfo);

    //double cutPlane[4];
    //CreateCutPlane(loopBasePointInfo, cutPlane);

    //if ( Vector3D(cutPlane).Length() < e_absolute )
    //{
    //    UF_CALL(::UF_DISP_set_highlight(tagFace,1));
    //}

    //CreateDatumPlaneFeature(QString("CUTPLANE%1").arg(loopBasePointInfo.adjEdges[0].id),
    //        loopBasePointInfo.point, Vector3D(loopBasePointInfo.cutPlane));

    Points3DCollection intersectionPointsCollection;

    for ( uf_list_p_t edge = edgesList; edge != 0x0; edge = edge->next )
        EdgeCutPlaneIntersect( edge->eid, intersectionPointsCollection, loopBasePointInfo );

    if ( intersectionPointsCollection.empty() )
    {
        UF_CALL( ::UF_DISP_set_highlight( tagFace, 1 ) );
        code = EdgeCutPlaneIntersectEmpty;
        goto end;
    }

    int type;
    UF_CALL( ::UF_MODL_ask_face_type( tagFace,
                                      &type ) );

    if ( type == UF_MODL_PLANAR_FACE )
    {
        Points3DCollection::const_iterator closedPointIterator;
        FindClosedIntersectionPoint( loopBasePointInfo.point, intersectionPointsCollection, closedPointIterator );

        Point3D closedPoint( *closedPointIterator );

        //CreateDatumPointFeature(QString("CLOSED%1").arg(tagFace),closedPoint);

        point = ( closedPoint + loopBasePointInfo.point ) * 0.5;
    }
    else
    {
        Vector3D middlePoint, lineNormal;

        for ( ; !intersectionPointsCollection.empty(); )
        {
            Points3DCollection::const_iterator closedPointIterator;
            FindClosedIntersectionPoint( loopBasePointInfo.point, intersectionPointsCollection, closedPointIterator );

            Point3D closedPoint( *closedPointIterator );

            middlePoint = ( closedPoint + loopBasePointInfo.point ) * 0.5;
            lineNormal = ( closedPoint - loopBasePointInfo.point ) ^ Vector3D( loopBasePointInfo.cutPlane );

            if ( FindLineFaceIntersect( tagFace, lineNormal, middlePoint, point ) )
                break;
            else
                intersectionPointsCollection.erase( closedPointIterator );
        }

        //CreateDatumAxisFeature(QString("MIDDLEAXIS%1").arg(tagFace),middlePoint,lineNormal);

        if ( intersectionPointsCollection.empty() )
        {
            UF_CALL( ::UF_DISP_set_highlight( tagFace, 1 ) );
            code = ClosedIntersectionPointNotFound;
            goto end;
        }
    }

end:
    UF_CALL( ::UF_MODL_delete_list( &edgesList ) );
    UF_CALL( ::UF_MODL_delete_loop_list( &loopsList ) );

    return code;
}

void FacePointsCloud_H( tag_t face, const Points3DCollection& points )
{
    double H( const Point3D&, const Points3DCollection& );
    const int n = 30;

    double limits[4];
    UF_CALL( ::UF_MODL_ask_face_uv_minmax( face, limits ) );

    double uv[2];
    double ud = ( limits[1] - limits[0] ) / n;
    double vd = ( limits[3] - limits[2] ) / n;
    uv[0] = limits[0];
    //tag_t seltag;
    double min = std::numeric_limits<double>::max();
    Point3D snapPoint;

    for ( int i = 0, t = n + 1; i < t; i++ )
    {
        uv[1] = limits[2];

        for ( int j = 0; j < t; j++ )
        {
            UF_MODL_SRF_VALUE_t evals;

            double p[3], r[2];
            UF_CALL( ::UF_MODL_ask_face_props( face, uv, evals.srf_pos, p, p, p, p, p, r ) );

            double v = H( Point3D( evals.srf_pos ), points );
            //tag_t tag = CreateDatumPointFeature(QString("%1").arg(v),Point3D(evals.srf_pos));

            if ( min > v )
            {
                min = v;
                //seltag = tag;
                snapPoint = evals.srf_pos;
            }

            uv[1] += vd;
        }

        uv[0] += ud;
    }

    tag_t tag = CreateDatumPointFeature( QString( "%1" ).arg( min ), snapPoint );
    UF_CALL( ::UF_DISP_set_highlight( tag, 1 ) );
}

void AddOnlyUniquePoint( Points3DCollection& intersectionPointsCollection,
                         Points3DCollection& intersectionPointsCollectionCurrent, double e )
{
    for ( Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
            ti = intersectionPointsCollection.end(); i != ti; ++i )
    {
        for ( Points3DCollection::const_iterator j = intersectionPointsCollectionCurrent.begin(),
                tj = intersectionPointsCollectionCurrent.end(); j != tj; ++j )
        {
            if ( i->IsClosedTo( *j, e ) )
            {
                intersectionPointsCollectionCurrent.erase( j );
                break;
            }
        }
    }

    intersectionPointsCollection.insert( intersectionPointsCollection.end(),
                                         intersectionPointsCollectionCurrent.begin(),
                                         intersectionPointsCollectionCurrent.end() );
}

bool ArePointsOnLine( const QVector<double>& pointsCollection )
{
    int PointCount = pointsCollection.count() / 3;
    //double x0,y0,z0;
    //double xn,yn,zn;

    double ( *points )[3] = ( double( * )[3] )&pointsCollection[0];
    Point3D p0( points[0] );
    Point3D pn( points[PointCount - 1] );
    Vector3D v0( pn - p0 );

    //double a,b,c;
    double e = 0.01;

    for ( int i = 1, t = PointCount - 1; i < t; i++ )
    {
        double d = ( v0 ^ Vector3D( Point3D( points[i] ) - p0 ) ).Length();

        if ( d > e )
            return false;
    }

    return true;
}

bool IsEdgeLinear( tag_t edge )
{
    const int numpts = 5;

    UF_EVAL_p_t evaluator;
    UF_CALL( ::UF_EVAL_initialize( edge, &evaluator ) );

    double limits [2];
    UF_CALL( ::UF_EVAL_ask_limits( evaluator, limits ) );

    double delta_t = fabs( limits[1] - limits[0] ) / ( numpts - 1 );

    double t_min = limits[0] > limits[1] ? limits[1] : limits[0];

    double parm[numpts];

    for ( int i = 0; i < numpts; i++ )
    {
        parm[i] = t_min;
        t_min += delta_t;
    }

    double derivatives[1];

    double point[3];

    int n_derivatives = 0;
    QVector<double> pointMatrix;

    for ( int i = 0; i < numpts; i++ )
    {
        UF_CALL( ::UF_EVAL_evaluate( evaluator, n_derivatives, parm[i], point, derivatives ) );
        pointMatrix.append( point[0] );
        pointMatrix.append( point[1] );
        pointMatrix.append( point[2] );
    }

    UF_CALL( ::UF_EVAL_free( evaluator ) );

    return ArePointsOnLine( pointMatrix );
}

int GetBoundingBox( const tag_t tagFace, Point3D ( &box )[2] )
{
    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_face_edges( tagFace,
                                       &edgesList ) );

    uf_list_p_t edge = edgesList;

    if ( edge != 0x0 )
    {
        int edgeType;
        UF_CALL( ::UF_MODL_ask_edge_type( edge->eid, &edgeType ) );

        if ( IsEdgeLinear( edge->eid ) )
        {
            if ( edgeType != UF_MODL_LINEAR_EDGE )
            {
                UF_CALL( ::UF_DISP_set_highlight( edge->eid, 1 ) );
                return 1;
            }

            int vertex_count;
            UF_CALL( ::UF_MODL_ask_edge_verts( edge->eid, ( double* )&box[0], ( double* )&box[1], &vertex_count ) );
            Q_ASSERT( vertex_count == 2 );
        }
        else
        {
            if ( edgeType == UF_MODL_LINEAR_EDGE )
            {
                UF_CALL( ::UF_DISP_set_highlight( edge->eid, 1 ) );
                return 2;
            }
        }

        edge = edge->next;
    }

    bool edgeIsLinear = false;

    for ( ; edge != 0x0; edge = edge->next )
    {
        int edgeType;
        UF_CALL( ::UF_MODL_ask_edge_type( edge->eid, &edgeType ) );

        if ( edgeIsLinear = IsEdgeLinear( edge->eid ) )
        {
            if ( edgeType != UF_MODL_LINEAR_EDGE )
            {
                UF_CALL( ::UF_DISP_set_highlight( edge->eid, 1 ) );
                return 1;
            }

            int vertex_count;
            Point3D p[2];
            UF_CALL( ::UF_MODL_ask_edge_verts( edge->eid, ( double* )&p[0], ( double* )&p[1], &vertex_count ) );
            Q_ASSERT( vertex_count == 2 );

            for ( int i = 0; i < 3; i++ )
            {
                if ( box[0][i] > p[0][i] )
                    box[0][i] = p[0][i];

                if ( box[1][i] < p[0][i] )
                    box[1][i] = p[0][i];
            }

            for ( int i = 0; i < 3; i++ )
            {
                if ( box[0][i] > p[1][i] )
                    box[0][i] = p[1][i];

                if ( box[1][i] < p[1][i] )
                    box[1][i] = p[1][i];
            }
        }
        else
        {
            if ( edgeType == UF_MODL_LINEAR_EDGE )
            {
                UF_CALL( ::UF_DISP_set_highlight( edge->eid, 1 ) );
                return 2;
            }

            //Get min X(u,v) on edge
            //If box[0].x > X
            //box[0].x = X
            //Get max X(u,v) on face
            //If box[1].x < X
            //box[1].x = X
            //Get min Y...
        }
    }

    if ( !edgeIsLinear )
    {
        //Get min X(u,v) on face
        //If box[0].x > X
        //If X(u,v) point inside face (not inside holes)
        //box[0].x = X
        //Get max X(u,v) on face
        //If box[1].x < X
        //If X(u,v) point inside face (not inside holes)
        //box[1].x = X
        //Get min Y...
    }

    UF_CALL( ::UF_MODL_delete_list( &edgesList ) );

    return 0;
}

int EvaluateFaceSnapPoint3( const tag_t tagFace, double* point )
{
    test();

    int code = 0;

    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_face_edges( tagFace,
                                       &edgesList ) );

    if ( edgesList )
    {
        Point3D boundBox[2];
        UF_CALL( ::UF_MODL_ask_bounding_box( tagFace, ( double* )boundBox ) );
        /*Point3D boundBox[2];
        if ( code = GetBoundingBox(tagFace,boundBox) )
        {
            UF_CALL(::UF_MODL_delete_list(&edgesList));
            return code;
        }*/

        double boxSize[3] = {boundBox[1].x - boundBox[0].x,
                             boundBox[1].y - boundBox[0].y,
                             boundBox[1].z - boundBox[0].z
                            };

        Point3D boxCenter( ( boundBox[0] + boundBox[1] ) * .5 );

        double* pos = std::max_element( boxSize, boxSize + 3 );
        int boxMaxSideIndex = int( pos - boxSize );

        Vector3D cutPlaneNormal( 0. );
        cutPlaneNormal[boxMaxSideIndex] = boxSize[boxMaxSideIndex];

        double e = boxSize[boxMaxSideIndex] * 1.e-5;

        double cutPlane[4] = {cutPlaneNormal.x, cutPlaneNormal.y, cutPlaneNormal.z, boxCenter* cutPlaneNormal * -1.};
        //CreateDatumPlaneFeature(QString("CUTPLANE%1").arg(tagFace),boxCenter, Vector3D(cutPlane));
        //CreateDatumAxisFeature(QString("CUTPLANENORMAL%1").arg(tagFace),boxCenter,Vector3D(cutPlane));

        Points3DCollection intersectionPointsCollection;
        uf_list_p_t edge = edgesList;
        Vector3D tangent;

        for ( ; edge != 0x0; edge = edge->next )
        {
            EdgeCutPlaneIntersect( edge->eid, intersectionPointsCollection, cutPlane );

            if ( intersectionPointsCollection.size() > 0 )
            {
                UF_EVAL_p_t evaluator;
                UF_CALL( ::UF_EVAL_initialize( edge->eid, &evaluator ) );

                double p[3], t;
                UF_CALL( ::UF_EVAL_evaluate_closest_point( evaluator, ( double* )&intersectionPointsCollection[0], &t, p ) );

                UF_CALL( ::UF_EVAL_evaluate( evaluator, 1, t, p, ( double* )&tangent ) );

                UF_CALL( ::UF_EVAL_free( evaluator ) );

                int orientation;
                ExtractEdgeOrientation( tagFace, edge->eid, orientation );

                tangent *= ( double )orientation;

                //CreateDatumAxisFeature(QString("TANGENT%1").arg(tagFace),intersectionPointsCollection[0],tangent);

                edge = edge->next;
                break;
            }
        }

        for ( ; edge != 0x0; edge = edge->next )
        {
            Points3DCollection intersectionPointsCollectionCurrent;
            EdgeCutPlaneIntersect( edge->eid, intersectionPointsCollectionCurrent, cutPlane );
            AddOnlyUniquePoint( intersectionPointsCollection, intersectionPointsCollectionCurrent, e );
        }

        if ( intersectionPointsCollection.empty() )
        {
            EvaluateFacePlaneIntersectionPoint( tagFace, cutPlane, *( ( Point3D* )point ) );
            //CreateDatumPointFeature(QString("SNAPFACE%1").arg(tagFace),point);
        }
        else
        {
            //int faceType;
            //UF_CALL(::UF_MODL_ask_face_type(tagFace,
            //                                &faceType));

            //if ( faceType == UF_MODL_PLANAR_FACE )
            //{
            //    std::sort(intersectionPointsCollection.begin(),intersectionPointsCollection.end());

            /*int i = 1;
            foreach(const Point3D& point, intersectionPointsCollection)
                CreateDatumPointFeature(QString("INTERSECT%1").arg(i++),point);*/

            //    point = (intersectionPointsCollection[0] + intersectionPointsCollection[1]) * .5;
            /*for(Points3DCollection::const_iterator i = intersectionPointsCollection.begin(),
                t = intersectionPointsCollection.end(); i != t; ++i)
            {
                CreateDatumPointFeature(QString("ORDER%1").arg(
                    std::distance((Points3DCollection::const_iterator)intersectionPointsCollection.begin(),i)),*i);
            }*/
            //}
            //else
            {
                if ( intersectionPointsCollection.size() == 1 )
                {
                    UF_CALL( ::UF_DISP_set_highlight( tagFace, 1 ) );
                    return 3;
                }

                Point3D first = intersectionPointsCollection[0];
                Point3D facePoint = first;
                //CreateDatumPointFeature(QString("NEXTPOINT%1").arg(tagFace),first);
                intersectionPointsCollection.erase( intersectionPointsCollection.begin() );

                Points3DCollection::const_iterator closedPointIterator = intersectionPointsCollection.end();

                if ( intersectionPointsCollection.size() > 1 )
                {
                    double p[3], uv[2];
                    UF_CALL( ::UF_MODL_ask_face_parm( tagFace, ( double* )&facePoint, uv, p ) );

                    /*UF_EVALSF_p_t evaluator;
                    UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));
                    UF_EVALSF_pos3_t srf_pos3;
                    UF_CALL(::UF_EVALSF_find_closest_point(evaluator, (double*)&facePoint, &srf_pos3));
                    double uv[2] = {srf_pos3.uv[0],srf_pos3.uv[1]};
                    UF_CALL(::UF_EVALSF_free(&evaluator));*/
                    //double uv[2];
                    //GetClosestUVPointOnSurface(tagFace,facePoint,uv);

                    double r[2];
                    Vector3D normalAtStartPoint, duv[2], dduv[2];
                    UF_CALL( ::UF_MODL_ask_face_props( tagFace, uv, p,
                                                       ( double* )&duv[0], ( double* )&duv[1], ( double* )&dduv[0], ( double* )&dduv[1],
                                                       ( double* )&normalAtStartPoint, r ) );

                    double duvm[2] = {duv[0].Length(), duv[1].Length()};

                    //CreateDatumAxisFeature(QString("faceNormalAtStartPoint%1").arg(tagFace),facePoint,faceNormalAtStartPoint);
                    //CreateDatumAxisFeature(QString("cutPlane%1").arg(tagFace),facePoint,cutPlane);
                    //CreateDatumAxisFeature(QString("du_%1").arg(duvm[0]),facePoint,duv[0]);
                    //CreateDatumAxisFeature(QString("dv_%1").arg(duvm[1]),facePoint,duv[1]);

                    Vector3D t = normalAtStartPoint ^ Vector3D( cutPlane ) * ( Vector3D( cutPlane ) * tangent );
                    t.Norm();

                    //CreateDatumAxisFeature(QString("t_%1").arg(tagFace),facePoint,t);

                    double tuv[2] = {duv[0]* t, duv[1]* t};

                    double limits[4];
                    UF_CALL( ::UF_MODL_ask_face_uv_minmax( tagFace, limits ) );

                    int U_status, V_status;
                    double UV_period[2];
                    UF_CALL( ::UF_MODL_ask_face_periodicity( tagFace, &U_status, &UV_period[0], &V_status, &UV_period[1] ) );
                    /*bool uvperiodic[2] = {::fabs((limits[1] - limits[0]) - UV_period[0]) / UV_period[0] < e_percent,
                                          ::fabs((limits[3] - limits[2]) - UV_period[1]) / UV_period[1] < e_percent};*/
                    bool uvperiodic[2] = {( UV_period[0] > 0. ) ?/*(::fabs((limits[1] - limits[0]) - UV_period[0]) / UV_period[0] < e_percent)*/true : false,
                                          ( UV_period[1] > 0. ) ?/*(::fabs((limits[3] - limits[2]) - UV_period[1]) / UV_period[1] < e_percent)*/true : false
                                         };
                    /*if( uvperiodic[0] && uvperiodic[1] )
                    {
                        UF_CALL(::UF_DISP_set_highlight(tagFace,1));
                        return 4;
                    }*/

                    if ( uvperiodic[0] )
                    {
                        //i = 0;
                        // j = 0;
                        if ( limits[0] > uv[0] && !( ::fabs( ( limits[0] - uv[0] ) / limits[0] ) < e_percent ) )
                            uv[0] += UV_period[0];
                        else if ( limits[1] < uv[0] && !( ::fabs( ( limits[1] - uv[0] ) / limits[1] ) < e_percent ) )
                            uv[0] -= UV_period[0];
                    }

                    //else
                    //{
                    if ( uvperiodic[1] )
                    {
                        //    i = 1;
                        //    j = 2;
                        if ( limits[2] > uv[1] && !( ::fabs( ( limits[2] - uv[1] ) / limits[2] ) < e_percent ) )
                            uv[1] += UV_period[1];
                        else if ( limits[3] < uv[1] && !( ::fabs( ( limits[3] - uv[1] ) / limits[3] ) < e_percent ) )
                            uv[1] -= UV_period[1];
                    }

                    //else
                    //    i = -1;
                    //}

                    /*double rr[2];
                    Point3D ppp;
                    double uvminmax[4] = {limits[0],limits[2],limits[1],limits[3]};
                    UF_CALL(::UF_MODL_ask_face_props(tagFace,uvminmax,(double*)&ppp,p,p,p,p,p,rr));
                    CreateDatumPointFeature(QString("MINUV%1").arg(tagFace),ppp);
                    UF_CALL(::UF_MODL_ask_face_props(tagFace,&uvminmax[2],(double*)&ppp,p,p,p,p,p,rr));
                    CreateDatumPointFeature(QString("MAXUV%1").arg(tagFace),ppp);*/

                    double rmin = std::numeric_limits<double>::max();

                    for ( Points3DCollection::const_iterator it = intersectionPointsCollection.begin(),
                            t = intersectionPointsCollection.end(); it != t; ++it )
                    {
                        double p[3], _uv[2];
                        UF_CALL( ::UF_MODL_ask_face_parm( tagFace, ( double* )&*it, _uv, p ) );
                        /*UF_EVALSF_p_t evaluator;
                        UF_CALL(::UF_EVALSF_initialize(tagFace, &evaluator));
                        UF_EVALSF_pos3_t srf_pos3;
                        UF_CALL(::UF_EVALSF_find_closest_point(evaluator, (double*)&*i, &srf_pos3));
                        double _uv[2] = {srf_pos3.uv[0],srf_pos3.uv[1]};
                        UF_CALL(::UF_EVALSF_free(&evaluator));*/
                        //double _uv[2];
                        //GetClosestUVPointOnSurface(tagFace,*it,_uv);

                        if ( uvperiodic[0] )
                        {
                            //i = 0;
                            //j = 0;
                            if ( limits[0] > _uv[0] && !( ::fabs( ( limits[0] - _uv[0] ) / limits[0] ) < e_percent ) )
                                _uv[0] += UV_period[0];
                            else if ( limits[1] < _uv[0] && !( ::fabs( ( limits[1] - _uv[0] ) / limits[1] ) < e_percent ) )
                                _uv[0] -= UV_period[0];
                        }

                        //else
                        //{
                        if ( uvperiodic[1] )
                        {
                            //i = 1;
                            //j = 2;
                            if ( limits[2] > _uv[1] && !( ::fabs( ( limits[2] - _uv[1] ) / limits[2] ) < e_percent ) )
                                _uv[1] += UV_period[1];
                            else if ( limits[3] < _uv[1] && !( ::fabs( ( limits[3] - _uv[1] ) / limits[3] ) < e_percent ) )
                                _uv[1] -= UV_period[1];
                        }

                        //else
                        //  i = -1;
                        //}

                        double ruv[2] = {( _uv[0] - uv[0] )* duvm[0], ( _uv[1] - uv[1] )* duvm[1]};

                        double sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                        double d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                        if ( sign > 0. && d < rmin )
                        {
                            closedPointIterator = it;
                            rmin = d;
                        }

                        double u = _uv[0];

                        for ( int i = 0; i < 2; i++ )
                        {
                            if ( uvperiodic[i] )
                            {
                                _uv[i] = ( ruv[i] >= 0. ) ? ( _uv[i] - UV_period[i] ) : ( _uv[i] + UV_period[i] );

                                ruv[0] = ( _uv[0] - uv[0] ) * duvm[0];
                                ruv[1] = ( _uv[1] - uv[1] ) * duvm[1];

                                sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                                d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                                if ( sign > 0. && d < rmin )
                                {
                                    closedPointIterator = it;
                                    rmin = d;
                                }
                            }
                        }

                        if ( uvperiodic[0] && uvperiodic[1] )
                        {
                            _uv[0] = u;

                            ruv[0] = ( _uv[0] - uv[0] ) * duvm[0];
                            ruv[1] = ( _uv[1] - uv[1] ) * duvm[1];

                            sign = ruv[0] * tuv[0] + ruv[1] * tuv[1];
                            d = ruv[0] * ruv[0] + ruv[1] * ruv[1];

                            if ( sign > 0. && d < rmin )
                            {
                                closedPointIterator = it;
                                rmin = d;
                            }
                        }

                        /*const double da = 1.e-1;

                        double delta[2];
                        int limits[2];

                        double d = ::fabs(normalAtStartPoint * dduv[0]);
                        if ( d < e_absolute )
                        {
                            limits[0] = 0;
                        }
                        else
                        {
                            delta[0] = da * duv[0].Length() / d;
                            limits[0] = 1;
                        }

                        d = ::fabs(normalAtStartPoint * dduv[1]);
                        if ( d < e_absolute )
                        {
                            limits[1] = 0;
                        }
                        else
                        {
                            delta[1] = da * duv[1].Length() / d;
                            limits[1] = 1;
                        }

                        Vector3D duv_norm[2] = {duv[0],duv[1]};
                        duv_norm[0].Norm();
                        duv_norm[1].Norm();

                        double r0[2] = {(limits[0] && ::fabs(duv_norm[0] * t) > e_point_absolute) ?
                                        ::fabs(duv[0] * delta[0] * t) : std::numeric_limits<double>::max(),
                                        (limits[1] && ::fabs(duv_norm[1] * t) > e_point_absolute) ?
                                        ::fabs(duv[1] * delta[1] * t) : std::numeric_limits<double>::max()};

                        r0[0] = (r0[0] < r0[1]) ? r0[0]: r0[1];

                        Points3DCollection::const_iterator closedPointIterator;
                        if ( r0[0] == std::numeric_limits<double>::max() )
                            FindClosedIntersectionPoint(facePoint, t, intersectionPointsCollection, closedPointIterator);
                        else
                            FindClosedIntersectionPoint(facePoint, intersectionPointsCollection, closedPointIterator);

                        if ( intersectionPointsCollection.end() == closedPointIterator )
                            FindClosedIntersectionPoint(facePoint, intersectionPointsCollection, closedPointIterator);

                        Point3D closedPoint(*closedPointIterator);

                        if ( (facePoint - closedPoint).Length() <= r0[0] )
                        {
                            Point3D middlePoint = (closedPoint + first) * 0.5;
                            Vector3D lineNormal = (closedPoint - first) ^ Vector3D(cutPlane);

                            //CreateDatumAxisFeature(QString("INTERLINE%1").arg(tagFace),middlePoint,lineNormal);

                            bool res = FindLineFaceIntersect(tagFace, lineNormal, middlePoint, point);
                            Q_ASSERT(res);
                            break;
                        }

                        //CreateDatumAxisFeature(QString("INTERLINE%1").arg(tagFace),t * r0[0] + facePoint,normalAtStartPoint);

                        if ( !FindLineFaceIntersect(tagFace, normalAtStartPoint, t * r0[0] + facePoint, facePoint) )
                            break;

                        //CreateDatumPointFeature(QString("NEXTPOINT%1").arg(tagFace),facePoint);

                        //points.push_back(point);*/
                    }
                }
                else
                    closedPointIterator = intersectionPointsCollection.begin();

                if ( closedPointIterator == intersectionPointsCollection.end() )
                {
                    UF_CALL( ::UF_DISP_set_highlight( tagFace, 1 ) );
                    return 1;
                }

                Point3D closedPoint( *closedPointIterator );

                Point3D middlePoint = ( closedPoint + first ) * 0.5;
                Vector3D lineNormal = ( closedPoint - first ) ^ Vector3D( cutPlane );

                //CreateDatumPointFeature(QString("CLOSEDPOINT%1").arg(tagFace),closedPoint);
                //CreateDatumAxisFeature(QString("INTERLINE%1").arg(tagFace),middlePoint,lineNormal);

                if ( !FindLineFaceIntersect( tagFace, lineNormal, middlePoint, *( ( Point3D* )point ) ) )
                {
                    UF_CALL( ::UF_DISP_set_highlight( tagFace, 1 ) );
                    return 2;
                }
            }
        }

        UF_CALL( ::UF_MODL_delete_list( &edgesList ) );
    }
    else
    {
        double limits[4];
        UF_CALL( ::UF_MODL_ask_face_uv_minmax( tagFace, limits ) );

        double uv[2] = {( limits[0] + limits[1] ) / 2., ( limits[2] + limits[3] ) / 2.};

        double p[3], r[2];
        UF_CALL( ::UF_MODL_ask_face_props( tagFace, uv, ( double* )&point, p, p, p, p, p, r ) );
        //CreateDatumPointFeature(QString("SNAPFACE%1").arg(tagFace),point);
    }

    return code;
}

int EvaluateFaceSnapPoint2( const tag_t tagFace, Point3D& /*point*/ )
{
    void H_test();
    //H_test();

    int code = 0;
    uf_loop_p_t loopsList;
    UF_CALL( ::UF_MODL_ask_face_loops( tagFace,
                                       &loopsList ) );

    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_face_edges( tagFace,
                                       &edgesList ) );

    uf_loop_p_t loop = 0x0;

    for ( loop = loopsList; loop != 0x0; loop = loop->next )
    {
        if ( loop->type == 1 )
            break;
    }

    double bb[6];
    UF_CALL( ::UF_MODL_ask_bounding_box( tagFace, bb ) );
    double defeatureEdgeLength = 1.e-3 * Point3D( bb ).DistanceTo( Point3D( &bb[3] ) );
    double minEdgeFaceLength = std::numeric_limits<double>::max();

    for ( uf_list_p_t edge = edgesList; edge != 0x0; edge = edge->next )
    {
        int vertex_count;
        double ( *points )[3];
        UF_CALL( ::UF_MODL_ask_curve_points( edge->eid, 0, 0, 0, &vertex_count, ( double** )&points ) );

        double edgeLength = 0.;

        for ( int i = 0, t = vertex_count - 1; i < t; i++ )
            edgeLength += Point3D( ( double* )points[i] ).DistanceTo( Point3D( ( double* )points[i + 1] ) );

        if ( edgeLength > defeatureEdgeLength && edgeLength < minEdgeFaceLength )
            minEdgeFaceLength = edgeLength;
    }

    Q_ASSERT( minEdgeFaceLength != std::numeric_limits<double>::max() );

    double patch = minEdgeFaceLength * 0.05;

    Points3DCollection pointsOnOuterLoop;

    for ( uf_list_p_t edge = edgesList; edge != 0x0; edge = edge->next )
    {
        //int edgeType;
        //UF_CALL(::UF_MODL_ask_edge_type(edge->eid, &edgeType));

        int vertex_count;
        /*if ( edgeType == UF_MODL_LINEAR_EDGE )
        {
            Point3D p[2];
            UF_CALL(::UF_MODL_ask_edge_verts(edge->eid, (double*)&p[0], (double*)&p[1], &vertex_count));
            t = 0.1 * p[0].DistanceTo(p[1]);
        }*/

        double ( *points )[3];
        UF_CALL( ::UF_MODL_ask_curve_points( edge->eid, 1.e-3 * patch, 0, patch, &vertex_count, ( double** )&points ) );

        for ( int i = 0; i < vertex_count; i++ )
        {
            pointsOnOuterLoop.push_back( Point3D( ( double* )points[i] ) );
            //CreateDatumPointFeature("",Point3D((double*)points[i]));
        }

        ::UF_free( points );
    }

    FacePointsCloud_H( tagFace, pointsOnOuterLoop );

    UF_CALL( ::UF_MODL_delete_list( &edgesList ) );
    UF_CALL( ::UF_MODL_delete_loop_list( &loopsList ) );

    return code;
}

double H( const Point3D& probe, const Points3DCollection& points )
{
#if 1
    Vector3D H( 0. );

    for ( Points3DCollection::const_iterator i = points.begin(), t = points.end(); i != t; ++i )
    {
        Vector3D r = probe - *i;
        H = H + r.Norm() * ( 1. / ( r * r ) );
    }

    return H.Length();
#else
    double s = 0.;

    for ( Points3DCollection::const_iterator i = points.begin(), t = points.end(); i != t; ++i )
    {
        Vector3D r = probe - *i;
        s += 1. / ( r * r );
    }

    return s;
#endif
}

void H_test()
{
    Points3DCollection c;

    c.push_back( Point3D( 3., 0, 0 ) );
    c.push_back( Point3D( 0, 3., 0 ) );
    c.push_back( Point3D( -3., 0, 0 ) );
    c.push_back( Point3D( 0, -3., 0 ) );
    c.push_back( Point3D( 0, 0, 3. ) );
    c.push_back( Point3D( 0, 0, -3. ) );

    if ( H( Point3D( 0. ), c ) != 0.0 )
        throw;
}
void NXExtensionImpl::NamingTest()
{
    std::string partFileName( MAX_FSPEC_SIZE + 1, 0 );
    UF_CALL( ::UF_PART_ask_part_name( ::UF_ASSEM_ask_work_part(), &partFileName[0] ) );
    QFileInfo fileInfo( QString::fromLocal8Bit( partFileName.c_str() ) );
    QString path = QDir::toNativeSeparators( fileInfo.absolutePath() );
    QString name = fileInfo.baseName();

    tag_t tagSolidBody = NULL_TAG;
    UF_CALL( ::UF_MODL_ask_object( UF_solid_type,
                                   UF_solid_body_subtype,
                                   &tagSolidBody ) );

    if ( tagSolidBody == NULL_TAG )
        return;
    else
    {
        do
        {
            int bodyType;
            UF_CALL( ::UF_MODL_ask_body_type( tagSolidBody, &bodyType ) );

            if ( bodyType == UF_MODL_SOLID_BODY )
                break;

            ::UF_MODL_ask_object( UF_solid_type,
                                  UF_solid_body_subtype,
                                  &tagSolidBody );
        } while ( tagSolidBody != NULL_TAG );
    }

    if ( tagSolidBody == NULL_TAG )
        return;

    Point3D boundBox[2];
    UF_CALL( ::UF_MODL_ask_bounding_box( tagSolidBody, ( double* )boundBox ) );

    Point3D::SetDistanceTolerance( ( boundBox[1] - boundBox[0] ).Length() * 1.e-6 );

    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_body_edges( tagSolidBody,
                                       &edgesList ) );

    std::set<Point3D> snapPointsCollection;

    if ( edgesList && edgesList->eid )
    {
        for ( uf_list_p_t edge = edgesList; edge != 0x0; edge = edge->next )
        {
            Point3D p[2];
            //if ( face->eid == 28946 )
            {
                int vertex_count;
                UF_CALL( ::UF_MODL_ask_edge_verts( edge->eid, ( double* )&p[0], ( double* )&p[1], &vertex_count ) );

                for ( int i = 0; i < vertex_count; i++ )
                {
                    if ( snapPointsCollection.find( p[i] ) == snapPointsCollection.end() )
                    {
                        //CreateDatumPointFeature(QString("SNAPVERTEX%1").arg(edge->eid),p[i]);
                        snapPointsCollection.insert( p[i] );
                    }
                }
            }
        }
    }

    if ( edgesList && edgesList->eid )
    {
        for ( uf_list_p_t edge = edgesList; edge != 0x0; edge = edge->next )
        {
            Point3D p;
            //if ( face->eid == 28946 )
            {
                EvaluateEdgeSnapPoint( edge->eid, ( double* )&p );
                std::set<Point3D>::const_iterator closest;

                if ( ( closest = snapPointsCollection.find( p ) ) != snapPointsCollection.end() )
                {
                    UF_CALL( ::UF_DISP_set_highlight( edge->eid, 1 ) );
                    CreateDatumPointFeature( QString( "SNAPEDGE%1" ).arg( edge->eid ), p );
                    CreateDatumPointFeature( QString( "CLOSESTTO" ), *closest );
                    QMessageBox::critical( GetTopWindow(), "Naming test", QString( "DOUBLICATE SNAP POINT EDGE TAG %1" ).arg( edge->eid ) );
                    return;
                }

                //CreateDatumPointFeature(QString("SNAPEDGE%1").arg(edge->eid),p);
                snapPointsCollection.insert( p );
            }
        }
    }

    uf_list_p_t facesList;
    UF_CALL( ::UF_MODL_ask_body_faces( tagSolidBody,
                                       &facesList ) );

    QFile faceSnapPointsFile( path + QDir::separator() + "faceSnapPoints." + name + ".obj" );
    faceSnapPointsFile.open( QFile::WriteOnly );
    QTextStream ts3( &faceSnapPointsFile );
    int fsp = 0;

    for ( uf_list_p_t face = facesList; face != 0x0; face = face->next )
    {
        Point3D p;
        //if ( face->eid == 28826 )
        {
            int code;
            std::set<Point3D>::const_iterator closest;

            if ( ( code = EvaluateFaceSnapPoint3( face->eid, ( double* )&p ) ) ||
                    ( closest = snapPointsCollection.find( p ) ) != snapPointsCollection.end() )
            {
                //UF_CALL(::UF_DISP_set_highlight(face->eid,1));
                //CreateDatumPointFeature(QString("CANDIDATE%1").arg(face->eid),p);
                //CreateDatumPointFeature(QString("CLOSESTTO"),*closest);
                QMessageBox::critical( GetTopWindow(), "Naming test",
                                       QString( "DOUBLICATE SNAP POINT FACE TAG %1, CODE %2" ).arg( face->eid ).arg( code ) );
                break;
            }
            else
            {
                CreateDatumPointFeature( QString( "SNAPFACE%1" ).arg( face->eid ), p );
                snapPointsCollection.insert( p );
            }

            ts3 << QString( "v %1 %2 %3\n" ).arg( p.x ).arg( p.y ).arg( p.z );
            fsp++;
        }
    }

    int k = 1;

    while ( k <= fsp )
    {
        ts3 << QString( "p %1\n" ).arg( k );
        k++;
    }
}
