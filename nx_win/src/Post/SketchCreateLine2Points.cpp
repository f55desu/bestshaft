#include "../Stable.h"
#include "../NXExtensionImpl.h"

double* InvertMatrix ( double* tm, double* m );

QJSValue NXExtensionImpl::SKETCH_Create_2D_Line_2Points( QJSValue sketch,
                                                         QString line_name,
                                                         double x1,
                                                         double y1,
                                                         double x2,
                                                         double y2,
                                                         bool /*flag*/ )
{
    Q_UNUSED( sketch );

    //tag_t active_sketch = getActiveSketch().toInt();
    tag_t active_sketch = sketch.toInt();

    if ( active_sketch == NULL_TAG )
    {
        m_scriptEngine->throwError( QString( "Sketch doesn't exist." ) );
        return QJSValue();
    }

    tag_t line_tag = NULL_TAG;
    UF_CURVE_line_t line;
    UF_SKET_info_t sketchInfo;
    int result;

    double start[3], end[3];
    /*line.start_point[0] = x1;
    line.start_point[1] = y1;
    line.start_point[2] = 0.0;
    line.end_point[0] = x2;
    line.end_point[1] = y2;
    line.end_point[2] = 0.0;*/
    start[0] = x1;
    start[1] = y1;
    start[2] = 0.0;
    end[0] = x2;
    end[1] = y2;
    end[2] = 0.0;


    result = UF_SKET_ask_sketch_info( active_sketch, &sketchInfo );
    UF_CALL( result );

    double mtx_4D[16];
    UF_MTX4_initialize( 1, &sketchInfo.csys[9], sketchInfo.csys, mtx_4D );

    UF_MTX4_vec3_multiply( start, mtx_4D, line.start_point );
    UF_MTX4_vec3_multiply( end, mtx_4D, line.end_point );

    //MapToWorld(sketch_info.csys, line.start_point);
    //MapToWorld(sketch_info.csys, line.end_point);

    result = UF_CURVE_create_line( &line, &line_tag );
    UF_CALL( result );

    result = UF_SKET_add_objects( active_sketch, 1, &line_tag );
    UF_CALL( result );

    result = UF_MODL_put_list_item( objectList, line_tag );
    UF_CALL( result );

    std::string name = line_name.toStdString();
    result = UF_OBJ_set_name( line_tag, ( char* )name.c_str() );
    UF_CALL( result );

    return QJSValue( result );
}

QJSValue NXExtensionImpl::SKETCH_Create_2D_Arc_3Points( QJSValue sketch,
                                                        QString arc_name,
                                                        double x1,
                                                        double y1,
                                                        double x2,
                                                        double y2,
                                                        double x3,
                                                        double y3,
                                                        bool /*flag*/ )
{
    Q_UNUSED( sketch );

    //tag_t active_sketch = getActiveSketch().toInt();
    tag_t active_sketch = sketch.toInt();

    if ( active_sketch == NULL_TAG )
    {
        m_scriptEngine->throwError( QString( "Sketch doesn't exist." ) );
        return QJSValue();
    }

    tag_t arc_tag = NULL_TAG;
    UF_SKET_info_t sketch_info;

    double p1[3], p2[3], p3[3];
    p1[0] = x1;
    p1[1] = y1;
    p1[2] = 0.0;
    p2[0] = x2;
    p2[1] = y2;
    p2[2] = 0.0;
    p3[0] = x3;
    p3[1] = y3;
    p3[2] = 0.0;

    UF_CALL( ::UF_SKET_ask_sketch_info( active_sketch, &sketch_info ) );

    MapToWorld( sketch_info.csys, p1 );
    MapToWorld( sketch_info.csys, p2 );
    MapToWorld( sketch_info.csys, p3 );

    /*UF_CALL(::UF_CURVE_create_arc_thru_3pts(1,p1,p2,p3,&arc_tag));*/
    UF_CURVE_arc_t arc_coords;

    arc_coords.arc_center[0] = 0.;
    arc_coords.arc_center[1] = 0.;
    arc_coords.arc_center[2] = 0.;
    //MapToWorld(sketch_info.csys, arc_coords.arc_center);
    /*tag_t mtx_tag = NULL_TAG;
    UF_CALL(UF_CSYS_ask_wcs(&mtx_tag));
    UF_CALL(UF_CSYS_ask_matrix_of_object(mtx_tag,
                                         &arc_coords.matrix_tag));*/
    double mtx[12];
    InvertMatrix( sketch_info.csys, mtx );
    MapToSketch( mtx, arc_coords.arc_center );

    //UF_CALL(::UF_MTX3_initialize(sketch_info.csys,&sketch_info.csys[3],mtx));
    UF_CALL( ::UF_CSYS_create_matrix( sketch_info.csys, &arc_coords.matrix_tag ) );
    //double csys_origin[3];
    //UF_CALL(::UF_CSYS_ask_csys_info(sketch_info.csys_tag, &arc_coords.matrix_tag, csys_origin));
    //arc_coords.matrix_tag = sketch_info.csys_tag;
    arc_coords.start_angle = 0.;
    arc_coords.end_angle = M_PI;
    arc_coords.radius = 0.5;

    UF_CALL( ::UF_CURVE_create_arc( &arc_coords, &arc_tag ) );
    UF_CALL( ::UF_SKET_add_objects( active_sketch, 1, &arc_tag ) );
    UF_CALL( ::UF_OBJ_set_name( arc_tag, qPrintable( arc_name ) ) );

    return QJSValue( arc_tag );
}
