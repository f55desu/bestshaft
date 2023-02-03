#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

double _round( double num )
{
    return qRound( num * 10 ) / 10.;
}

void NXExtensionImpl::preprocessCSYSPlaneAxisPoint( tag_t face, tag_t edge,
                                                    tag_t edgeForPoint, double scalar )
{
    int face_type, face_norm_dir,
        edge_vertex_count;
    double face_point[3], face_dir[3], face_box[6], face_radius, face_rad_data,
           edge_point1[3], edge_point2[3];

    UF_CALL( ::UF_MODL_ask_face_data( face,
                                      &face_type,
                                      face_point,
                                      face_dir,
                                      face_box,
                                      &face_radius,
                                      &face_rad_data,
                                      &face_norm_dir ) );

    name_face = "face" + QString::number( ++count_face );
    /*content+= QString("var %7 = SELECT_Face(%1, %2, %3, %4, %5, %6, 0);\n")
    .arg(face_point[0]).arg(face_point[1]).arg(face_point[2])
    .arg(face_dir[0]).arg(face_dir[1]).arg(face_dir[2])
    .arg(name_face);*/

    content += indent + QString( "var %1 = SELECT_Object(%2, %3, %4);\n" )
               .arg( name_face )
               .arg( _round( face_point[0] ) ).arg( _round( face_point[1] ) ).arg( _round( face_point[2] ) );

    UF_CALL( ::UF_MODL_ask_edge_verts( edge,
                                       edge_point1,
                                       edge_point2,
                                       &edge_vertex_count ) );

    name_edge = "edge" + QString::number( ++count_edge );
    /*content+= QString("var %7 = SELECT_Edge(%1, %2, %3, %4, %5, %6, 0);\n")
    .arg( (edge_point1[0] + edge_point2[0])/2 )
    .arg( (edge_point1[1] + edge_point2[1])/2 )
    .arg( (edge_point1[2] + edge_point2[2])/2 )
    .arg(face_dir[0]).arg(face_dir[1]).arg(face_dir[2])
    .arg(name_edge);  */

    content += indent + QString( "var %4 = SELECT_Object(%1, %2, %3);\n" )
               .arg( _round( ( edge_point1[0] + edge_point2[0] ) / 2 ) )
               .arg( _round( ( edge_point1[1] + edge_point2[1] ) / 2 ) )
               .arg( _round( ( edge_point1[2] + edge_point2[2] ) / 2 ) )
               .arg( name_edge );

    name_point = "point" + QString::number( ++count_point_on_edge );
    /*content+= indent + QString("var %1 = CONSTRAINTS_Create_3DReference_Point_on_curve(%2, %3);\n")
    .arg(name_point)
    .arg(name_edge)
    .arg(scalar);*/

    UF_CALL( ::UF_MODL_ask_edge_verts( edgeForPoint,
                                       edge_point1,
                                       edge_point2,
                                       &edge_vertex_count ) );

    if ( !scalar )
    {
        content += indent + QString( "var %4 = SELECT_Object(%1, %2, %3);\n" )
                   .arg( _round( edge_point1[0] ) )
                   .arg( _round( edge_point1[1] ) )
                   .arg( _round( edge_point1[2] ) )
                   .arg( name_point );
    }
    else
    {
        content += indent + QString( "var %4 = SELECT_Object(%1, %2, %3);\n" )
                   .arg( _round( edge_point2[0] ) )
                   .arg( _round( edge_point2[1] ) )
                   .arg( _round( edge_point2[2] ) )
                   .arg( name_point );
    }

    name_csys = "new_csys" + QString::number( ++count_new_csys );
    content += indent + QString( "var %1 = CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point(%2, %3, %4);\n" )
               .arg( name_csys )
               .arg( name_face )
               .arg( name_edge )
               .arg( name_point );
}
