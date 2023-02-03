#include "../Stable.h"
#include "../NXExtensionImpl.h"

QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys(QString csysName)
{
    tag_t tagPart = ::UF_ASSEM_ask_work_part();

    tag_t tagCSYSFeature = NULL_TAG;

    /*int type = UF_coordinate_system_type;
    tag_t tagCSYS = NULL_TAG;
    UF_CALL(::UF_OBJ_cycle_objs_in_part(tagPart,type,&tagCSYS));
    if ( tagCSYS != NULL_TAG )
    {
        const double linear_offset[3] = {0.0};
        const double angular_offset[3] = {0.0};
        UF_CALL(::UF_MODL_create_datum_csys_offset(tagCSYS,
                                                   tagCSYS,
                                                   linear_offset,
                                                   angular_offset,
                                                   1,
                                                   &tagCSYSFeature));
    }
    else
    {*/
        /*tag_t tagMatrix = NULL_TAG;
        double matrix_values[9];
        ::UF_MTX3_identity(matrix_values);
        UF_CALL(::UF_CSYS_create_matrix(matrix_values,
                                       &tagMatrix));

        const double csys_origin[9] = {0.0};
        UF_CALL(::UF_CSYS_create_csys(csys_origin,
                                      tagMatrix,
                                      &tagCSYSFeature));*/

        tag_t tagScalar[3] = {NULL_TAG};
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[0]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[1]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[2]));

        tag_t tagPoint[3] = {NULL_TAG};
        UF_CALL(::UF_SO_create_point_3_scalars(tagPart,
                                               UF_SO_update_within_modeling,
                                               tagScalar,
                                               &tagPoint[0]));

        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             1.0,
                                             &tagScalar[0]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[1]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[2]));

        UF_CALL(::UF_SO_create_point_3_scalars(tagPart,
                                               UF_SO_update_within_modeling,
                                               tagScalar,
                                               &tagPoint[1]));

        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[0]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             1.0,
                                             &tagScalar[1]));
        UF_CALL(::UF_SO_create_scalar_double(tagPart,
                                             UF_SO_update_within_modeling,
                                             0.0,
                                             &tagScalar[2]));

        UF_CALL(::UF_SO_create_point_3_scalars(tagPart,
                                               UF_SO_update_within_modeling,
                                               tagScalar,
                                               &tagPoint[2]));

        tag_t tagXForm = NULL_TAG;
        UF_CALL(::UF_SO_create_xform_three_points(tagPart,
                                                  UF_SO_update_within_modeling,
                                                  tagPoint[0],
                                                  tagPoint[1],
                                                  tagPoint[2],
                                                  NULL_TAG,
                                                  &tagXForm));
        /*const double originPoint[3] = {0.0};
        const double x_direction[3] = {1.,0.,0.};
        const double y_direction[3] = {0.,1.,0.};
        UF_CALL(::UF_SO_create_xform_doubles(tagPart,
                                             UF_SO_update_within_modeling,
                                             originPoint,
                                             x_direction,
                                             y_direction,
                                             1.,
                                             &tagXForm));*/

        UF_CALL(::UF_MODL_create_datum_csys(tagPart,
                                            tagXForm,
                                            1,
                                            &tagCSYSFeature));
    //}


    UF_CALL(::UF_OBJ_set_name(tagCSYSFeature, qPrintable(csysName)));

    return QScriptValue(tagCSYSFeature);
}

/*QScriptValue NXExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys(QString csysName)
{
    tag_t tagXForm = NULL_TAG, tagCSYSFeature = NULL_TAG;

    UF_CALL(::UF_MODL_create_datum_csys(::UF_ASSEM_ask_work_part(),
                                        tagXForm,
                                        1,
                                        &tagCSYSFeature));

    UF_CALL(::UF_OBJ_set_name(tagCSYSFeature, qPrintable(csysName)));

    return QScriptValue(tagCSYSFeature);
}*/
