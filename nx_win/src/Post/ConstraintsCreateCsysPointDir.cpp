#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                                QJSValue x_dir,
                                                                                QJSValue y_dir )
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG, xform = NULL_TAG,
          datum_csys = NULL_TAG,
          x_direction = NULL_TAG, y_direction = NULL_TAG,
          line_x = NULL_TAG, line_y = NULL_TAG,
          origin_point = NULL_TAG;
    double dir_X_value[3], dir_Y_value[3],
           Z_value;

    line_x = x_dir.toInt();
    line_y = y_dir.toInt();
    origin_point = point.toInt();

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_x, false, &x_direction );
    UF_CALL( result );

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_y, false, &y_direction );
    UF_CALL( result );

    result = UF_SO_ask_direction_of_dirr( x_direction, dir_X_value );
    UF_CALL( result );

    result = UF_SO_ask_direction_of_dirr( y_direction, dir_Y_value );
    UF_CALL( result );

    Z_value = dir_X_value[0] * dir_Y_value[1] - dir_X_value[1] * dir_Y_value[0];

    if ( Z_value < 0 )
    {
        result = UF_SO_create_dirr_line( globalTagPart, update_option, line_y, true, &y_direction );
        UF_CALL( result );
    }

    result =  UF_SO_create_scalar_double_dim( globalTagPart,
                                              update_option,
                                              1.0,
                                              dim_option,
                                              &scalar_xform );
    UF_CALL( result );

    result = UF_SO_create_xform_pnt_xy_dirs( globalTagPart,
                                             update_option,
                                             origin_point,
                                             x_direction,
                                             y_direction,
                                             scalar_xform,
                                             &xform );
    UF_CALL( result );

    result = UF_MODL_create_datum_csys( globalTagPart, xform, true, &datum_csys );
    UF_CALL( result );

    return QJSValue( datum_csys );
}

QJSValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                                QJSValue y_dir,
                                                                                QJSValue z_dir )
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG, xform = NULL_TAG,
          datum_csys = NULL_TAG,
          y_direction = NULL_TAG, z_direction = NULL_TAG,
          line_z = NULL_TAG, line_y = NULL_TAG,
          origin_point = NULL_TAG;

    line_z = z_dir.toInt();
    line_y = y_dir.toInt();
    origin_point = point.toInt();

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_z, false, &z_direction );
    UF_CALL( result );

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_y, false, &y_direction );
    UF_CALL( result );

    result =  UF_SO_create_scalar_double_dim( globalTagPart,
                                              update_option,
                                              1.0,
                                              dim_option,
                                              &scalar_xform );
    UF_CALL( result );

    result = UF_SO_create_xform_pnt_yz_dirs( globalTagPart,
                                             update_option,
                                             origin_point,
                                             y_direction,
                                             z_direction,
                                             scalar_xform,
                                             &xform );
    UF_CALL( result );

    result = UF_MODL_create_datum_csys( globalTagPart, xform, true, &datum_csys );
    UF_CALL( result );

    return QJSValue( datum_csys );
}

QJSValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                                QJSValue x_dir,
                                                                                QJSValue z_dir )
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG, xform = NULL_TAG,
          datum_csys = NULL_TAG,
          x_direction = NULL_TAG, z_direction = NULL_TAG,
          line_x = NULL_TAG, line_z = NULL_TAG,
          origin_point = NULL_TAG;

    line_x = x_dir.toInt();
    line_z = z_dir.toInt();
    origin_point = point.toInt();

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_x, false, &x_direction );
    UF_CALL( result );

    result = UF_SO_create_dirr_line( globalTagPart, update_option, line_z, false, &z_direction );
    UF_CALL( result );

    result =  UF_SO_create_scalar_double_dim( globalTagPart,
                                              update_option,
                                              1.0,
                                              dim_option,
                                              &scalar_xform );
    UF_CALL( result );

    result = UF_SO_create_xform_pnt_xz_dirs( globalTagPart,
                                             update_option,
                                             origin_point,
                                             x_direction,
                                             z_direction,
                                             scalar_xform,
                                             &xform );
    UF_CALL( result );

    result = UF_MODL_create_datum_csys( globalTagPart, xform, true, &datum_csys );
    UF_CALL( result );

    return QJSValue( datum_csys );
}
