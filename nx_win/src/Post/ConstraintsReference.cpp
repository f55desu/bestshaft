#include "../Stable.h"
#include "../NXExtensionImpl.h"

QScriptValue NXExtensionImpl::createDatumBaseCsys(double matrix[9])
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    tag_t xform = NULL_TAG,
            datum_csys = NULL_TAG,
            point = NULL_TAG,
            dir_x = NULL_TAG, dir_y = NULL_TAG,
            scalar_x = NULL_TAG, scalar_y = NULL_TAG, scalar_z = NULL_TAG,
            scalar_csys = NULL_TAG;
    double x_dir[3], y_dir[3];

    for (int i = 0; i < 3; i++)
    {
        x_dir[i] = matrix[i];
        y_dir[i] = matrix[i+3];
    }

    result = UF_SO_create_dirr_doubles(global_tag_part, update_option, x_dir, &dir_x);
    UF_CALL(result);

    result = UF_SO_create_dirr_doubles(global_tag_part, update_option, y_dir, &dir_y);
    UF_CALL(result);

    result = UF_SO_create_scalar_double(global_tag_part, update_option, 0.0, &scalar_x);
    UF_CALL(result);

    result = UF_SO_create_scalar_double(global_tag_part, update_option, 0.0, &scalar_y);
    UF_CALL(result);

    result = UF_SO_create_scalar_double(global_tag_part, update_option, 0.0, &scalar_z);
    UF_CALL(result);

    result = UF_SO_create_scalar_double(global_tag_part, update_option, 1.0, &scalar_csys);
    UF_CALL(result);

    tag_t scale[3] = {scalar_x, scalar_y, scalar_z};

    result = UF_SO_create_point_3_scalars(global_tag_part, update_option, scale, &point);
    UF_CALL(result);

    result = UF_SO_create_xform_pnt_xy_dirs (global_tag_part, update_option, point, dir_x, dir_y, scalar_csys, &xform);
    UF_CALL(result);

    result = UF_MODL_create_datum_csys(global_tag_part, xform, true, &datum_csys);
    UF_CALL(result);

    return QScriptValue(datum_csys);
}

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_Base_CoorSys(QVariantList matr)
{
    global_tag_part = ::UF_PART_ask_display_part();
    flag_firs_solid = false;

    int result;
    tag_t existing_sys = NULL_TAG,
            matrix_exist_csys = NULL_TAG;
    double matrix_exist[9],
            origin_point_exist[3],
            matrix[9];

    for (int i = 0; i < 9; i++)
    {
        matrix[i] = matr.at(i).toDouble();
    }

    result = UF_OBJ_cycle_objs_in_part(global_tag_part, UF_coordinate_system_type, &existing_sys);
    UF_CALL(result);

    if (existing_sys != NULL_TAG)
    {
        result = UF_CSYS_ask_csys_info(existing_sys, &matrix_exist_csys, origin_point_exist);
        UF_CALL(result);

        result = UF_CSYS_ask_matrix_values(matrix_exist_csys, matrix_exist);
        UF_CALL(result);

        for (int i = 0; i < 9; i++)
        {
            if (matrix_exist[i] != matrix[i])
            {
                return QScriptValue(createDatumBaseCsys(matrix).toInteger());
            }
            if (i == 8 && matrix_exist[8] == matrix[8])
            {
                return QScriptValue(existing_sys);
            }
        }
    }
    else
    {
        return QScriptValue(createDatumBaseCsys(matrix).toInteger());
    }
	return 0;
}

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point(QScriptValue base_plane, 
                                                                                       QScriptValue base_axis,
                                                                                       QScriptValue base_point)
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;

    tag_t /*scalar_point_normal_1 = NULL_TAG,	scalar_point_normal_2 = NULL_TAG,*/
            scalar_xform = NULL_TAG, xform = NULL_TAG,
            datum_csys = NULL_TAG, /*so_origin_point = NULL_TAG,*/
            dirr_normal_surface = NULL_TAG,	dirr_axis = NULL_TAG,
            csys_plane = NULL_TAG, csys_axis = NULL_TAG, csys_point = NULL_TAG;

    double direction_value[3];

    csys_plane = base_plane.toInteger();
    csys_axis = base_axis.toInteger();
    csys_point = base_point.toInteger();

    /*result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 0.50, dim_option, &scalar_point_normal_1);
    UF_CALL(result);
    result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 0.50, dim_option, &scalar_point_normal_2);
    UF_CALL(result);
    // center point surface
    result =  UF_SO_create_point_on_surface(global_tag_part, update_option, csys_plane,
                                            scalar_point_normal_1,
                                            scalar_point_normal_2,
                                            &so_origin_point);
    UF_CALL(result);

    result = UF_SO_create_dirr_normal_to_surface_point(global_tag_part, update_option, csys_plane, so_origin_point, false, &dirr_normal_surface);
    UF_CALL(result);*/

    result =  UF_SO_create_dirr_plane(global_tag_part, update_option, csys_plane, false, &dirr_normal_surface);
    UF_CALL(result);

    result = UF_SO_create_dirr_line(global_tag_part, update_option, csys_axis, false, &dirr_axis);
    UF_CALL(result);

    result = UF_SO_ask_direction_of_dirr(dirr_axis, direction_value);
    UF_CALL(result);

    // check OY direction: normal or reverse
    if (direction_value[0] == -1)
    {
        result = UF_SO_create_dirr_line(global_tag_part, update_option, csys_axis, true, &dirr_axis);
        UF_CALL(result);
    }

    result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 1.0, dim_option, &scalar_xform);
    UF_CALL(result);

    result = UF_SO_create_xform_pnt_xz_dirs(global_tag_part, update_option, csys_point, dirr_axis, dirr_normal_surface, scalar_xform, &xform);
    UF_CALL(result);

    result = UF_MODL_create_datum_csys(global_tag_part, xform, true, &datum_csys);
    UF_CALL(result);

    return QScriptValue(datum_csys);
}

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir(QScriptValue point, 
                                                                                   QScriptValue x_dir,
                                                                                   QScriptValue y_dir)
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG,
            xform = NULL_TAG,
            datum_csys = NULL_TAG,
            x_direction = NULL_TAG,
            y_direction = NULL_TAG,
            line_x = NULL_TAG,
            line_y = NULL_TAG,
            origin_point = NULL_TAG;
    double dir_X_value[3],
            dir_Y_value[3],
            Z_value;

    line_x = x_dir.toInteger();
    line_y = y_dir.toInteger();
    origin_point = point.toInteger();

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_x, false, &x_direction);
    UF_CALL(result);

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_y, false , &y_direction);
    UF_CALL(result);

    result = UF_SO_ask_direction_of_dirr(x_direction, dir_X_value);
    UF_CALL(result);

    result = UF_SO_ask_direction_of_dirr(y_direction, dir_Y_value);
    UF_CALL(result);

    Z_value = dir_X_value[0] * dir_Y_value[1] - dir_X_value[1] * dir_Y_value[0];

    if (Z_value < 0)
    {
        result = UF_SO_create_dirr_line(global_tag_part, update_option, line_y, true , &y_direction);
        UF_CALL(result);
    }

    result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 1.0, dim_option, &scalar_xform);
    UF_CALL(result);

    result = UF_SO_create_xform_pnt_xy_dirs(global_tag_part, update_option, origin_point, x_direction, y_direction, scalar_xform, &xform);
    UF_CALL(result);

    result = UF_MODL_create_datum_csys(global_tag_part, xform, true, &datum_csys);
    UF_CALL(result);

    return QScriptValue(datum_csys);
}

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir(QScriptValue point, 
                                                                                   QScriptValue y_dir,
                                                                                   QScriptValue z_dir)
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG,
            xform = NULL_TAG,
            datum_csys = NULL_TAG,
            y_direction = NULL_TAG,
            z_direction = NULL_TAG,
            line_z = NULL_TAG,
            line_y = NULL_TAG,
            origin_point = NULL_TAG;

    line_z = z_dir.toInteger();
    line_y = y_dir.toInteger();
    origin_point = point.toInteger();

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_z, false, &z_direction);
    UF_CALL(result);

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_y, false, &y_direction);
    UF_CALL(result);

    result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 1.0, dim_option, &scalar_xform);
    UF_CALL(result);

    result = UF_SO_create_xform_pnt_yz_dirs(global_tag_part, update_option, origin_point, y_direction, z_direction, scalar_xform, &xform);
    UF_CALL(result);

    result = UF_MODL_create_datum_csys(global_tag_part, xform, true, &datum_csys);
    UF_CALL(result);

    return QScriptValue(datum_csys);
}

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir(QScriptValue point, 
                                                                                   QScriptValue x_dir,
                                                                                   QScriptValue z_dir)
{
    int result;
    UF_SO_update_option_e update_option = UF_SO_update_within_modeling;
    UF_SO_scalar_dim_option_t  dim_option = UF_SO_scalar_dimensionality_length;
    tag_t scalar_xform = NULL_TAG, xform = NULL_TAG,
            datum_csys = NULL_TAG,
            x_direction = NULL_TAG, z_direction = NULL_TAG,
            line_x = NULL_TAG, line_z = NULL_TAG,
            origin_point = NULL_TAG;

    line_x = x_dir.toInteger();
    line_z = z_dir.toInteger();
    origin_point = point.toInteger();

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_x, false, &x_direction);
    UF_CALL(result);

    result = UF_SO_create_dirr_line(global_tag_part, update_option, line_z, false, &z_direction);
    UF_CALL(result);

    result =  UF_SO_create_scalar_double_dim(global_tag_part, update_option, 1.0, dim_option, &scalar_xform);
    UF_CALL(result);

    result = UF_SO_create_xform_pnt_xz_dirs(global_tag_part, update_option, origin_point, x_direction, z_direction, scalar_xform, &xform);
    UF_CALL(result);

    result = UF_MODL_create_datum_csys(global_tag_part, xform, true, &datum_csys);
    UF_CALL(result);

    return QScriptValue(datum_csys);
}