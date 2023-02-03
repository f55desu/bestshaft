#include "../Stable.h"
#include "../NXExtensionImpl.h"

QVector<int> NXExtensionImpl::getAllFace()
{
    int result,
        subtype = 0,
        type = 0;
    tag_t object = NULL_TAG;

    //allFace.clear();
    QVector<int> allFace;

    result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_solid_type, &object );
    UF_CALL( result );

    while ( object != NULL_TAG )
    {
        result = UF_OBJ_ask_type_and_subtype ( object, &type, &subtype );
        UF_CALL( result );

        if ( subtype == UF_solid_face_subtype )
            allFace.push_back( object );

        result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_solid_type, &object );
        UF_CALL( result );
    }

    return allFace;
}

QVector<int> NXExtensionImpl::getAllEdge()
{
    int result,
        subtype = 0,
        type = 0;
    tag_t object = NULL_TAG;

    //allEdge.clear();
    QVector<int> allEdge;

    result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_solid_type, &object );
    UF_CALL( result );

    while ( object != NULL_TAG )
    {
        result = UF_OBJ_ask_type_and_subtype ( object, &type, &subtype );
        UF_CALL( result );

        if ( subtype == UF_solid_edge_subtype )
            allEdge.push_back( object );

        result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_solid_type, &object );
        UF_CALL( result );
    }

    return allEdge;
}

QJSValue NXExtensionImpl::getPlaneXOY( QJSValue csys )
{
    int result;
    //count = 0;
    tag_t //basic_plane[3],
    plane = NULL_TAG,
    //return_plane = NULL_TAG,
    csys_tag = NULL_TAG,
    origin = NULL_TAG,
    daxes[3], dplanes[3];
    bool isSoCSYS;

    result = UF_SO_is_so( csys.toInt(), &isSoCSYS );
    UF_CALL( result );

    if ( isSoCSYS )
    {
        result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_datum_plane_type, &plane );
        UF_CALL( result );
        return QJSValue( plane );

        /*while (ft_plane != NULL_TAG)
        {
        basic_plane[count++] = ft_plane;
        result = UF_OBJ_cycle_objs_in_part (globalTagPart, UF_datum_plane_type, &ft_plane);
        UF_CALL(result);
        }*/
        //return_plane = ft_plane;// basic_plane[0];
    }
    else
    {
        UF_MODL_ask_datum_csys_components( csys.toInt(), &csys_tag, &origin, daxes, dplanes );
        return QJSValue( dplanes[0] );
        //return_plane = dplanes[0];
    }

    //return QJSValue(return_plane);
}

QJSValue NXExtensionImpl::getAxisOX( QJSValue csys )
{
    int result;
    //count = 0;
    tag_t //basic_axis[3],
    ft_axis = NULL_TAG,
    //return_axis = NULL_TAG,
    csys_tag = NULL_TAG,
    origin = NULL_TAG,
    daxes[3],
    dplanes[3];
    bool flag_so;

    result = UF_SO_is_so( csys.toInt(), &flag_so );
    UF_CALL( result );

    if ( flag_so )
    {
        result = UF_OBJ_cycle_objs_in_part( globalTagPart, UF_datum_axis_type, &ft_axis );
        UF_CALL( result );
        return QJSValue( ft_axis );
        /*while (ft_axis != NULL_TAG)
        {
        basic_axis[count++] = ft_axis;
        result = UF_OBJ_cycle_objs_in_part (globalTagPart, UF_datum_axis_type, &ft_axis);
        UF_CALL(result);
        }
        return_axis = basic_axis[0];*/
    }
    else
    {
        UF_MODL_ask_datum_csys_components( csys.toInt(), &csys_tag, &origin, daxes, dplanes );
        return QJSValue( daxes[0] );
        //return_axis = daxes[0];
    }

    //return QJSValue(return_axis);
}

QJSValue NXExtensionImpl::createPointOnCurve( QJSValue curve, double t )
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_t = NULL_TAG,
          point_on_curve = NULL_TAG;

    result = UF_SO_create_scalar_double_dim( globalTagPart, update_option, t, dim_option, &scalar_t );
    UF_CALL( result );

    result = UF_SO_create_point_on_curve( globalTagPart, update_option, curve.toInt(), scalar_t, &point_on_curve );
    UF_CALL( result );

    return QJSValue( point_on_curve );
}

/*QJSValue createHelpLine (double Xp, double Yp, double Zp,
double Xr, double Yr, double Zr)
{
tag_t help_line = NULL_TAG;
UF_CURVE_line_t line;

line.start_point[0] = Xp;
line.start_point[1] = Yp;
line.start_point[2] = Zp;
line.end_point[0] = 100 * Xr + Xp;
line.end_point[1] = 100 * Yr + Yp;
line.end_point[2] = 100 * Zr + Zp;

int result = UF_CURVE_create_line(&line, &help_line);
UF_CALL(result);

return QJSValue(help_line);
}

bool NXExtensionImpl::compareFace (const intersect_info_face &value1, const intersect_info_face &value2)
{
return value1.t < value2.t;
}

bool NXExtensionImpl::compareEdge (const intersect_info_edge &value1, const intersect_info_edge &value2)
{
return value1.t < value2.t;
}

double tValue(double Xp, double Yp, double Zp,
double Xr, double Yr, double Zr,
double Xintr, double Yintr, double Zintr)
{
double allDistance,
distanceToPoint,
value_t;

allDistance = sqrt(pow(100 * Xr, 2) + pow(100 * Yr, 2) + pow (100 * Zr, 2));
distanceToPoint = sqrt(pow(Xp - Xintr, 2) + pow(Yp - Yintr, 2) + pow (Zp - Zintr, 2));
//if ((Xp - Xintr) < 0 || (Yp - Yintr) < 0 || (Zp - Zintr) < 0 ) distanceToPoint*= -1;
value_t = distanceToPoint / allDistance;

return value_t;
}*/

/*QJSValue NXExtensionImpl::SELECT_Face(double Xp, double Yp, double Zp,
double Xr, double Yr, double Zr,
int number)
{
tag_t return_face = NULL_TAG,
help_line = NULL_TAG;
UF_MODL_intersect_info_p_t  *intersections;
int result, num_intersections;
double tolerance = 1.0;
intersect_info_face info_face; // struct
allIntersectFace.clear();

help_line = createHelpLine(Xp, Yp, Zp, Xr, Yr, Zr).toInt();
QVector<int> allFace(findAllFace());

for (int i = 0; i < allFace.size(); i++)
{
result = UF_MODL_intersect_objects(help_line, allFace.at(i), tolerance, &num_intersections, &intersections);
UF_CALL(result);

if (num_intersections >= 1)
{
info_face.intersected_face = allFace.at(i);
info_face.t = tValue(Xp, Yp, Zp, Xr, Yr, Zr,
intersections[0]->intersect.point.coords[0],
intersections[0]->intersect.point.coords[1],
intersections[0]->intersect.point.coords[2]);
allIntersectFace.push_back(info_face);
}
}

result = UF_OBJ_delete_object(help_line);
UF_CALL(result);
if (allIntersectFace.size() == 0 || allIntersectFace.size() < (number + 1))
{
return m_scriptEngine->currentContext()->throwError("No selected objects. Set other parameters.");
}
qSort(allIntersectFace.begin(), allIntersectFace.end(), &NXExtensionImpl::compareFace);
return_face = allIntersectFace.at(number).intersected_face;

return QJSValue(return_face);
}

QJSValue NXExtensionImpl::SELECT_Edge(double Xp, double Yp, double Zp,
double Xr, double Yr, double Zr,
int number)
{
tag_t return_edge = NULL_TAG,
help_line = NULL_TAG;
UF_MODL_intersect_info_p_t  *intersections;
int result,    num_intersections;
double tolerance = 1.0;
intersect_info_edge info_edge; // struct
allIntersectEdge.clear();

help_line = createHelpLine(Xp, Yp, Zp, Xr, Yr, Zr).toInt();
QVector<int> allEdge(findAllEdge());

for (int i = 0; i < allEdge.size(); i++)
{
result = UF_MODL_intersect_objects(help_line, allEdge.at(i), tolerance, &num_intersections, &intersections);
UF_CALL(result);
if (num_intersections >= 1)
{
info_edge.intersected_edge = allEdge.at(i);
info_edge.t = tValue(Xp, Yp, Zp, Xr, Yr, Zr,
intersections[0]->intersect.point.coords[0],
intersections[0]->intersect.point.coords[1],
intersections[0]->intersect.point.coords[2]);
allIntersectEdge.push_back(info_edge);
}
}

result = UF_OBJ_delete_object(help_line);
UF_CALL(result);
if (allIntersectEdge.size() == 0 || allIntersectEdge.size() < (number + 1))
{
return m_scriptEngine->currentContext()->throwError("No selected objects. Set other parameters.");
}
qSort(allIntersectEdge.begin(), allIntersectEdge.end(), &NXExtensionImpl::compareEdge);
return_edge = allIntersectEdge.at(number).intersected_edge;

return QJSValue(return_edge);
}

/*QJSValue NXExtensionImpl::SELECT_Point(double Xr, double Yr, double Zr, QString direction)
{
tag_t return_point = NULL_TAG,
help_line = NULL_TAG;
UF_MODL_intersect_info_p_t  *intersections;
int result,
num_intersections = 0;
double tolerance = 0.001,
line_coor[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

help_line = create_Help_Line(Xr, Yr, Zr, direction, line_coor).toInt();
FindAllSoPoint();

for (int i = 0; i < allSoPoint.size(); i++)
{
result = UF_MODL_intersect_objects(allSoPoint.at(i), help_line, tolerance, &num_intersections, &intersections);
UF_CALL(result);

if (num_intersections >= 1)
{
result = UF_OBJ_delete_object(help_line);
UF_CALL(result);
return_point = allSoPoint.at(i);
break;
}
}
return QJSValue(return_point);
}*/
