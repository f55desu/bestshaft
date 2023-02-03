#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                                    QJSValue base_axis,
                                                                                    QJSValue base_point )
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;

    tag_t /*scalar_point_normal_1 = NULL_TAG,    scalar_point_normal_2 = NULL_TAG,*/
    scalar_xform = NULL_TAG, xform = NULL_TAG,
    datum_csys = NULL_TAG, /*so_origin_point = NULL_TAG,*/
    dirr_normal_surface = NULL_TAG,    dirr_axis = NULL_TAG,
    csys_plane = NULL_TAG, csys_axis = NULL_TAG, csys_point = NULL_TAG;

    double direction_value[3];

    csys_plane = base_plane.toInt();
    csys_axis = base_axis.toInt();
    csys_point = base_point.toInt();

    result =  UF_SO_create_dirr_plane( globalTagPart, update_option, csys_plane, false, &dirr_normal_surface );
    UF_CALL( result );

    result = UF_SO_create_dirr_line( globalTagPart, update_option, csys_axis, false, &dirr_axis );
    UF_CALL( result );

    result = UF_SO_ask_direction_of_dirr( dirr_axis, direction_value );
    UF_CALL( result );

    // check OY direction: normal or reverse
    if ( direction_value[0] == -1 )
    {
        result = UF_SO_create_dirr_line( globalTagPart, update_option, csys_axis, true, &dirr_axis );
        UF_CALL( result );
    }

    result =  UF_SO_create_scalar_double_dim( globalTagPart, update_option, 1.0, dim_option, &scalar_xform );
    UF_CALL( result );

    result = UF_SO_create_xform_pnt_xz_dirs( globalTagPart, update_option, csys_point, dirr_axis, dirr_normal_surface,
                                             scalar_xform, &xform );
    UF_CALL( result );

    result = UF_MODL_create_datum_csys( globalTagPart, xform, true, &datum_csys );
    UF_CALL( result );

    return QJSValue( datum_csys );
}

/*result =  UF_SO_create_scalar_double_dim(globalTagPart, update_option, 0.50, dim_option, &scalar_point_normal_1);
UF_CALL(result);
result =  UF_SO_create_scalar_double_dim(globalTagPart, update_option, 0.50, dim_option, &scalar_point_normal_2);
UF_CALL(result);
// center point surface
result =  UF_SO_create_point_on_surface(globalTagPart, update_option, csys_plane,
                                        scalar_point_normal_1,
                                        scalar_point_normal_2,
                                        &so_origin_point);
UF_CALL(result);

result = UF_SO_create_dirr_normal_to_surface_point(globalTagPart, update_option, csys_plane, so_origin_point, false, &dirr_normal_surface);
UF_CALL(result);*/
