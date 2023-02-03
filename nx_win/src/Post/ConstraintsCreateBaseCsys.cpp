#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::CONSTRAINTS_Create_Base_CoorSys( QVariantList matr )
{
    CheckPostprocess();

    /*int result;
    tag_t existing_csys = NULL_TAG,
            matrix_exist_csys = NULL_TAG;*/
    double /*matrix_exist[9],
            origin_point_exist[3],*/
    matrix[12];

    /*for (int i = 0; i < 9; i++)
    {
        matrix[i] = matr.at(i).toDouble();
    }*/
    matrix[0] = matr.at( 0 ).toDouble();
    matrix[1] = matr.at( 3 ).toDouble();
    matrix[2] = matr.at( 6 ).toDouble();
    matrix[3] = matr.at( 1 ).toDouble();
    matrix[4] = matr.at( 4 ).toDouble();
    matrix[5] = matr.at( 7 ).toDouble();
    matrix[6] = matr.at( 2 ).toDouble();
    matrix[7] = matr.at( 5 ).toDouble();
    matrix[8] = matr.at( 8 ).toDouble();
    matrix[9] = matr.at( 9 ).toDouble();
    matrix[10] = matr.at( 10 ).toDouble();
    matrix[11] = matr.at( 11 ).toDouble();

    /*tag_t  matrix_id;
    UF_CALL(::UF_CSYS_create_matrix(matrix, &matrix_id));

    tag_t csys_id;
    UF_CALL(::UF_CSYS_create_csys(&matrix[9],matrix_id,&csys_id));

    return QJSValue(csys_id);*/

    /*result = UF_OBJ_cycle_objs_in_part(globalTagPart, UF_coordinate_system_type, &existing_csys);
    UF_CALL(result);

    if (existing_csys != NULL_TAG)
    {
        result = UF_CSYS_ask_csys_info(existing_csys, &matrix_exist_csys, origin_point_exist);
        UF_CALL(result);

        result = UF_CSYS_ask_matrix_values(matrix_exist_csys, matrix_exist);
        UF_CALL(result);

        for (int i = 0; i < 9; i++)
        {
            if (matrix_exist[i] != matrix[i])
            {
                return QJSValue(createBaseCsys(matrix).toInt());
            }
            else if (i == 8 )
            {
                return QJSValue(existing_csys);
            }
        }
    }
    else*/
    //{
    return QJSValue( createBaseCsys( matrix ).toInt() );
    //}
    //return QJSValue();
}

QJSValue NXExtensionImpl::createBaseCsys( double matrix[12] )
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

    /*for (int i = 0; i < 3; i++)
    {
        x_dir[i] = matrix[i];
        y_dir[i] = matrix[i+3];
    }*/
    x_dir[0] = matrix[0];
    x_dir[1] = matrix[1];
    x_dir[2] = matrix[2];
    y_dir[0] = matrix[3];
    y_dir[1] = matrix[4];
    y_dir[2] = matrix[5];

    result = UF_SO_create_dirr_doubles( globalTagPart, update_option, x_dir, &dir_x );
    UF_CALL( result );

    result = UF_SO_create_dirr_doubles( globalTagPart, update_option, y_dir, &dir_y );
    UF_CALL( result );

    result = UF_SO_create_scalar_double( globalTagPart, update_option, matrix[9], &scalar_x );
    UF_CALL( result );

    result = UF_SO_create_scalar_double( globalTagPart, update_option, matrix[10], &scalar_y );
    UF_CALL( result );

    result = UF_SO_create_scalar_double( globalTagPart, update_option, matrix[11], &scalar_z );
    UF_CALL( result );

    result = UF_SO_create_scalar_double( globalTagPart, update_option, 1.0, &scalar_csys );
    UF_CALL( result );

    tag_t scale[3] = {scalar_x, scalar_y, scalar_z};

    result = UF_SO_create_point_3_scalars( globalTagPart, update_option, scale, &point );
    UF_CALL( result );

    result = UF_SO_create_xform_pnt_xy_dirs ( globalTagPart,
                                              update_option,
                                              point,
                                              dir_x,
                                              dir_y,
                                              scalar_csys,
                                              &xform );
    UF_CALL( result );

    result = UF_MODL_create_datum_csys( globalTagPart, xform, true, &datum_csys );
    UF_CALL( result );

    return QJSValue( datum_csys );
}
