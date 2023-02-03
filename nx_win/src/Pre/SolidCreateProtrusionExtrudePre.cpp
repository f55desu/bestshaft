#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

void NXExtensionImpl::preprocessProtrusionExtrude_SWP( tag_t feature )
{
    int num_obj;
    tag_t* objects_tag;
    UF_MODL_SWEEP_TRIM_object_p_t trim_ptr;
    char* taper_angle;
    char* limit[2];
    char* offset[2];
    double point[3], direction[3];
    bool region_specified, solid_creation;
    char name[UF_OBJ_NAME_LEN + 1];

    UF_CALL( ::UF_MODL_ask_extrusion( feature,
                                      &num_obj, &objects_tag,
                                      &trim_ptr,
                                      &taper_angle,
                                      limit,
                                      offset,
                                      point,
                                      &region_specified, &solid_creation,
                                      direction ) );
    ::UF_OBJ_ask_name( feature, name );

    content += indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"%1\", %3, %4);\n" )
               .arg( name )
               .arg( count_feature_sketch )
               .arg( limit[0] )
               .arg( limit[1] );

    ::UF_free( objects_tag );
    ::UF_free( taper_angle );
    ::UF_free( limit[0] );
    ::UF_free( limit[1] );
    ::UF_free( offset[0] );
    ::UF_free( offset[1] );
    ::UF_free( trim_ptr );
}
void NXExtensionImpl::preprocessProtrusionExtrude_Extrude( tag_t feature )
{
    UF_MODL_mswp_extrude_t extrude;
    char* feature_name;

    UF_CALL( ::UF_MODL_ask_feat_name( feature, &feature_name ) );

    QString str_name = QString( feature_name );
    str_name.remove( str_name.indexOf( "(" ), 1 );
    str_name.remove( str_name.indexOf( ")" ), 1 );

    UF_CALL( ::UF_MODL_mswp_ask_extrude( feature, &extrude ) );

    content += indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"%1\", %3, %4);\n" )
               .arg( str_name )
               .arg( count_feature_sketch )
               .arg( extrude.limits.start_limit.limit_data.distance_data.string )
               .arg( extrude.limits.end_limit.limit_data.distance_data.string );
}
