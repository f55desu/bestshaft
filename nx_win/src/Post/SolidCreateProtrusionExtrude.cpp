#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::SOLID_Create_Protrusion_Extrude( QJSValue selected_sketch, QString extrude_name,
                                                           double depth_start,
                                                           double depth_end, bool /*sign*/ )
{
    int result;
    double direction[3], point[3] = {0, 0, 0};
    const char* taper_angle = "0.0";
    char* limit[2];
    std::string depth_start_Str = QString::number( depth_start ).toStdString();
    std::string depth_end_Str = QString::number( depth_end ).toStdString();
    uf_list_p_t new_features = 0x0;
    UF_SKET_info_t sketch_info;

    result = UF_SKET_ask_sketch_info( selected_sketch.toInt(), &sketch_info );
    UF_CALL( result );
    direction[0] = sketch_info.csys[6];
    direction[1] = sketch_info.csys[7];
    direction[2] = sketch_info.csys[8];

    limit[0] = ( char* )depth_start_Str.c_str();
    limit[1] = ( char* )depth_end_Str.c_str();

    CheckFirstSolidInPart();

    if ( firstSolidFlag )
    {
        result = UF_MODL_create_extruded2( objectList, ( char* )taper_angle, limit, point, direction, UF_POSITIVE,
                                           &new_features );
        UF_CALL( result );
    }
    else
    {
        result = UF_MODL_create_extruded2( objectList, ( char* )taper_angle, limit, point, direction, UF_NULLSIGN,
                                           &new_features );
        UF_CALL( result );
    }

    if ( result )
        m_scriptEngine->throwError( QString( "You can't create extrude." ) );
    else
    {
        std::string line_name = extrude_name.toStdString();
        result = UF_OBJ_set_name( new_features->eid, ( char* )line_name.c_str() );
        UF_CALL( result );
    }

    return QJSValue( result );
}
