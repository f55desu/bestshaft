#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../../../extension/src/Point.h"

tag_t createPointForSelect ( double pointCoord[3] )
{
    tag_t pointTag = NULL_TAG;

    int result = UF_CURVE_create_point( pointCoord, &pointTag );
    UF_CALL( result );

    return pointTag;
}

QJSValue NXExtensionImpl::SELECT_Object( double xPoint, double yPoint, double zPoint,
                                         TopologyPriority topologyPriority )
{
    Q_UNUSED( topologyPriority );

    tag_t tagSolidBody = NULL_TAG;

    do
    {
        ::UF_MODL_ask_object( UF_solid_type,
                              UF_solid_body_subtype,
                              &tagSolidBody );

        int bodyType;
        UF_CALL( ::UF_MODL_ask_body_type( tagSolidBody, &bodyType ) );

        if ( bodyType == UF_MODL_SOLID_BODY )
            break;

    } while ( tagSolidBody != NULL_TAG );

    if ( tagSolidBody == NULL_TAG )
        return QJSValue();

    Point3D boundBox[2];
    UF_CALL( ::UF_MODL_ask_bounding_box( tagSolidBody, ( double* )boundBox ) );

    Point3D::SetDistanceTolerance( ( boundBox[1] - boundBox[0] ).Length() * 1.e-6 );

    uf_list_p_t edgesList;
    UF_CALL( ::UF_MODL_ask_body_edges( tagSolidBody,
                                       &edgesList ) );

    Point3D point( xPoint, yPoint, zPoint );

    if ( edgesList && edgesList->eid )
    {
        uf_list_p_t edge = edgesList;

        //Find edge vertex first
        for ( ; edge != 0x0; edge = edge->next )
        {
            Point3D p[2];
            int vertex_count;
            UF_CALL( ::UF_MODL_ask_edge_verts( edge->eid, ( double* )&p[0], ( double* )&p[1], &vertex_count ) );

            double t = 0.0;

            for ( int i = 0; i < vertex_count; i++, t += 1.0 )
            {
                if ( p[i].IsClosedTo( point ) )
                    return QJSValue( createPointOnCurve( edge->eid, t ) );
            }
        }

        //Find edge
        for ( edge = edgesList; edge != 0x0; edge = edge->next )
        {
            Point3D p( xPoint, yPoint, zPoint ), closest;
            double t;
            UF_EVAL_p_t evaluator;
            UF_CALL( ::UF_EVAL_initialize( edge->eid, &evaluator ) );
            UF_CALL( ::UF_EVAL_evaluate_closest_point( evaluator, ( double* )&p, &t, ( double* )&closest ) );
            UF_CALL( ::UF_EVAL_free( evaluator ) );

            if ( p.IsClosedTo( closest ) )
                return QJSValue( edge->eid );
        }

        UF_CALL( ::UF_MODL_delete_list( &edgesList ) );
    }

    uf_list_p_t facesList;
    UF_CALL( ::UF_MODL_ask_body_faces( tagSolidBody,
                                       &facesList ) );

    //Find face
    for ( uf_list_p_t face = facesList; face != 0x0; face = face->next )
    {
        Point3D p( xPoint, yPoint, zPoint );
        UF_EVALSF_p_t evaluator;
        UF_CALL( ::UF_EVALSF_initialize( face->eid, &evaluator ) );
        UF_EVALSF_pos3_t srf_pos3;
        UF_CALL( ::UF_EVALSF_ask_minimum_face_dist( evaluator, ( double* )&p, &srf_pos3 ) );

        if ( p.IsClosedTo( srf_pos3.pnt3 ) )
            return QJSValue( face->eid );

        UF_CALL( ::UF_EVALSF_free( &evaluator ) );
    }

    UF_CALL( ::UF_MODL_delete_list( &facesList ) );

    /*int result;
    tag_t pointTag = NULL_TAG;
    tag_t returnedObject = NULL_TAG;
    int status = 0;
    int edge_vertex_count;
    double edge_point1[3], edge_point2[3];
    double tolerance = 0.01;
    double pointCoord[3] = {xPoint, yPoint, zPoint};

    pointTag = createPointForSelect(pointCoord);
    QVector<int> allEdge(getAllEdge());
    QVector<int> allFace(getAllFace());

    for (int i = 0; i < allEdge.size(); i++)
    {
        result = UF_MODL_ask_point_containment(pointCoord, allEdge.at(i), &status);
        UF_CALL(result);

        if (status == 3) //point is on the edge
        {
            result = UF_MODL_ask_edge_verts(allEdge.at(i),
                edge_point1,
                edge_point2,
                &edge_vertex_count);
            UF_CALL(result);

            if ( fabs(edge_point1[0] - pointCoord[0]) < tolerance
                && fabs(edge_point1[1] - pointCoord[1]) < tolerance
                && fabs(edge_point1[2] - pointCoord[2]) < tolerance )
            {
                returnedObject = createPointOnCurve(allEdge.at(i), 0.0).toInt();
                break;
            }

            if ( fabs(edge_point2[0] - pointCoord[0]) < tolerance
                && fabs(edge_point2[1] - pointCoord[1]) < tolerance
                && fabs(edge_point2[2] - pointCoord[2]) < tolerance )
            {
                returnedObject = createPointOnCurve(allEdge.at(i), 1.0).toInt();
                break;
            }

        }

        if (status == 1) //point is inside the edge
        {
            returnedObject = allEdge.at(i);
            break;
        }
    }

    for (int i = 0; i < allFace.size(); i++)
    {
        result = UF_MODL_ask_point_containment(pointCoord, allFace.at(i), &status);
        UF_CALL(result);

        if (status == 1) //point is inside the face
        {
            returnedObject = allFace.at(i);
            break;
        }
    }

    result = UF_OBJ_delete_object(pointTag);
    UF_CALL(result);*/

    m_scriptEngine->throwError( QString( "No selected objects. Set other parameters." ) );
    return QJSValue();
}
