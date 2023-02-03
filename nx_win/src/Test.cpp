#include "Stable.h"
#include "NXExtensionImpl.h"
#include "nxUtils.h"

QString NXExtensionImpl::Test()
{
    const char* part_name = "T:\\paratran\\trunk\\system\\test\\standartModel.prt";
    tag_t part = NULL_TAG;
    UF_PART_load_status_t error_status;

    UF_PART_open( part_name, &part, &error_status );

    QString testPre = PreprocessModel();

    return testPre;
}

/*void NXExtensionImpl::testCsysPre()
{
    tag_t tagBaseCsys = NULL_TAG;
    double point_coords[3];
    double origin_point[3];
    double plane_normal[3];
    double direction[3];
    int parentCount;
    int childCount;
    int type, subtype;
    int count;
    tag_t* tagsParents;
    tag_t* tagsChilds;
    tag_t* tagFeatureObjects;
    char *feature_name;
    QVector<tag_t> tagsCsys;

    content.clear();

    UF_CALL(::UF_MODL_ask_object(UF_coordinate_system_type, UF_csys_normal_subtype, &tagBaseCsys));

    do
    {
        tagsCsys.push_back(tagBaseCsys);
        UF_CALL(::UF_MODL_ask_object(UF_coordinate_system_type, UF_csys_normal_subtype, &tagBaseCsys));
    } while ( tagBaseCsys != NULL_TAG );

    UF_CALL(::UF_SO_ask_children(tagsCsys.last(), UF_SO_ASK_ALL_CHILDREN, &childCount, &tagsChilds));

    UF_CALL(::UF_MODL_ask_feat_name(tagsChilds[0], &feature_name));

    UF_CALL(::UF_MODL_ask_feat_object(tagsChilds[0],
        &count,
        &tagFeatureObjects));

    for (int i = 0; i < count; i++)
    {
        UF_CALL(::UF_OBJ_ask_type_and_subtype(tagFeatureObjects[i],
            &type,
            &subtype));

        if ( type == UF_point_type )
        {
            ::UF_CURVE_ask_point_data(tagFeatureObjects[i], point_coords);
        }

        if ( type == UF_datum_plane_type )
        {
            ::UF_MODL_ask_datum_plane(tagFeatureObjects[i], origin_point, plane_normal);
        }
    }

    UF_CALL(::UF_SO_ask_parents(tagsCsys.last(), UF_SO_ASK_ALL_PARENTS, &parentCount, &tagsParents));

    ::UF_SO_ask_x_direction_of_xform(tagsParents[0], direction);

    ::UF_SO_ask_y_direction_of_xform(tagsParents[0], direction);

    ::UF_SO_ask_z_direction_of_xform(tagsParents[0], direction);
}*/
