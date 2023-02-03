#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

NXExtensionImpl::information_xform NXExtensionImpl::info_xform;
bool NXExtensionImpl::isSoDirection = true;

double* InvertMatrix ( double* tm, double* m )
{
    double det;

    /*det = 1.0/(tm[0]*(tm[5]*tm[10]-tm[9]*tm[6])-
          tm[1]*(tm[4]*tm[10]-tm[8]*tm[6])+
          tm[2]*(tm[4]*tm[9]-tm[5]*tm[8]));

    m[0] = (tm[5]*tm[10] - tm[6]*tm[9])*det;
    m[1] = (tm[2]*tm[9] - tm[1]*tm[10])*det;
    m[2] = (tm[1]*tm[6] - tm[2]*tm[5])*det;
    m[4] = (tm[6]*tm[8] - tm[4]*tm[10])*det;
    m[5] = (tm[0]*tm[10] - tm[2]*tm[8])*det;
    m[6] = (tm[2]*tm[4] - tm[0]*tm[6])*det;
    m[8] = (tm[4]*tm[9] - tm[5]*tm[8])*det;
    m[9] = (tm[1]*tm[8] - tm[0]*tm[9])*det;
    m[10] = (tm[0]*tm[5] - tm[1]*tm[4])*det;

    m[3 ] = -(tm[3]*m[0]+tm[7]*m[1]+tm[11]*m[2]);
    m[7 ] = -(tm[3]*m[4]+tm[7]*m[5]+tm[11]*m[6]);
    m[11] = -(tm[3]*m[8]+tm[7]*m[9]+tm[11]*m[10]);*/

    det = 1.0 / ( tm[0] * ( tm[4] * tm[8] - tm[5] * tm[7] ) -
                  tm[3] * ( tm[1] * tm[8] - tm[2] * tm[7] ) +
                  tm[6] * ( tm[1] * tm[5] - tm[4] * tm[2] ) );

    m[0] = ( tm[4] * tm[8] - tm[7] * tm[5] ) * det;
    m[3] = ( tm[6] * tm[5] - tm[3] * tm[8] ) * det;
    m[6] = ( tm[3] * tm[7] - tm[6] * tm[4] ) * det;
    m[1] = ( tm[7] * tm[2] - tm[1] * tm[8] ) * det;
    m[4] = ( tm[0] * tm[8] - tm[6] * tm[2] ) * det;
    m[7] = ( tm[6] * tm[1] - tm[0] * tm[7] ) * det;
    m[2] = ( tm[1] * tm[5] - tm[4] * tm[2] ) * det;
    m[5] = ( tm[3] * tm[2] - tm[0] * tm[5] ) * det;
    m[8] = ( tm[0] * tm[4] - tm[3] * tm[1] ) * det;

    m[9 ] = -( tm[9] * m[0] + tm[10] * m[3] + tm[11] * m[6] );
    m[10 ] = -( tm[9] * m[1] + tm[10] * m[4] + tm[11] * m[7] );
    m[11] = -( tm[9] * m[2] + tm[10] * m[5] + tm[11] * m[8] );

    return m;
}

void NXExtensionImpl::Visitor::TraverseSmartObjects( tag_t object )
{
    int typeSmart, subtypeSmart;

//    if ( visitedSmartObjectsSet.find(object) != visitedSmartObjectsSet.end() )
//    {
//        return;
//    }
//    else
//    {
//        visitedSmartObjectsSet.insert(object);

    UF_CALL( ::UF_OBJ_ask_type_and_subtype( object,
                                            &typeSmart,
                                            &subtypeSmart ) );

    if ( typeSmart == UF_coordinate_system_type )
    {
        if ( !isSoDirection )
        {
            tag_t matrix;
            double csys_origin[3], matrix_value[9];

            UF_CALL( ::UF_CSYS_ask_csys_info( object, &matrix, csys_origin ) );
            UF_CALL( ::UF_CSYS_ask_matrix_values( matrix, matrix_value ) );

            //preprocessBaseCSYS(matrix_value);
        }

        if ( info_xform.face != NULL_TAG )
        {
            double scalar;
            uf_list_p_t suppressFeature;
            UF_CALL( ::UF_MODL_create_list( &suppressFeature ) );

            if ( info_xform.status_edge == UF_OBJ_CONDEMNED ||
                    info_xform.status_face == UF_OBJ_CONDEMNED ||
                    info_xform.status_edgeForPoint == UF_OBJ_CONDEMNED )
            {
                /*for (int i = allFeature.size() - 1; i >= numberCurrentFeature; i--)
                {
                    UF_CALL(::UF_MODL_put_list_item(suppressFeature, allFeature.at(i)));
                }*/

                UF_CALL( ::UF_MODL_suppress_feature( suppressFeature ) );
            }

            UF_CALL( ::UF_SO_ask_double_of_scalar( info_xform.point_scalar, &scalar ) );

            //preprocessCSYSPlaneAxisPoint(info_xform.face, info_xform.edge, info_xform.edgeForPoint, scalar);

            if ( info_xform.status_edge == UF_OBJ_CONDEMNED ||
                    info_xform.status_face == UF_OBJ_CONDEMNED ||
                    info_xform.status_edgeForPoint == UF_OBJ_CONDEMNED )
                UF_CALL( ::UF_MODL_unsuppress_feature( suppressFeature ) );

            ::UF_free( suppressFeature );
        }

        isSoDirection = true;
        info_xform.edge = NULL_TAG;
        info_xform.face = NULL_TAG;
        info_xform.edgeForPoint = NULL_TAG;
        info_xform.point_scalar = NULL_TAG;
        info_xform.status_edge = 0;
        info_xform.status_face = 0;
        info_xform.status_edgeForPoint = 0;
    }

//    }

    int parentCount;
    tag_t* tagsParents;
    int type, subtype, typeObj, subtypeObj;

    UF_CALL( ::UF_SO_ask_parents( object, UF_SO_ASK_ALL_PARENTS, &parentCount, &tagsParents ) );

    UF_CALL( ::UF_OBJ_ask_type_and_subtype( object,
                                            &typeObj,
                                            &subtypeObj ) );

    for ( int j = 0; j < parentCount; j++ )
    {
        UF_CALL( ::UF_OBJ_ask_type_and_subtype( tagsParents[j],
                                                &type,
                                                &subtype ) );

        // adding information about the xform

        if ( typeObj == UF_direction_type && subtype == UF_solid_edge_subtype )
        {
            info_xform.status_edge = ::UF_OBJ_ask_status( tagsParents[j] );
            info_xform.edge = tagsParents[j];
        }

        if ( ( typeObj == UF_direction_type || typeObj == UF_xform_type )
                && subtype == UF_solid_face_subtype )
        {
            info_xform.status_face = ::UF_OBJ_ask_status( tagsParents[j] );
            info_xform.face = tagsParents[j];
        }

        if ( typeObj == UF_point_type && type == UF_scalar_type )
            info_xform.point_scalar = tagsParents[j];

        if ( typeObj == UF_point_type && subtype == UF_solid_edge_subtype )
        {
            info_xform.status_edgeForPoint = ::UF_OBJ_ask_status( tagsParents[j] );
            info_xform.edgeForPoint = tagsParents[j];
        }

        logical isSOObject;
        UF_CALL( ::UF_SO_is_so( tagsParents[j], &isSOObject ) );

        if ( isSOObject )
            TraverseSmartObjects( tagsParents[j] );
        else if ( type == UF_direction_type )
            isSoDirection = false;
    }

    ::UF_free( tagsParents );

    int childCount;
    tag_t* tagsChilds;

    UF_CALL( ::UF_SO_ask_children( object, UF_SO_ASK_ALL_CHILDREN, &childCount, &tagsChilds ) );

    for ( int j = 0; j < childCount; j++ )
    {
        logical isSOObject;
        UF_CALL( ::UF_SO_is_so( tagsChilds[j], &isSOObject ) );

        if ( isSOObject )
            TraverseSmartObjects( tagsChilds[j] );
    }

    ::UF_free( tagsChilds );
}

#if 0
void NXExtensionImpl::TraverseFeatureRelatives( tag_t feature, QString dumpIndent )
{
    Category& logger = BaseExtension::GetLogger();

    int objCountParents, objCountChild;
    tag_t* tagsObjFeatParents, *tagsObjFeatChild;
    char* feat_type/*, *feature_name*/;
    //  char name[UF_OBJ_NAME_LEN+1];

    if ( visitedFeaturesSet.find( feature ) != visitedFeaturesSet.end() )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( feature, &feat_type ) );

        /*if( !strcmp(feat_type,"BLEND") )
        {
        UF_MODL_edge_blend_data_t blend_data;
        UF_CALL(::UF_MODL_ask_edge_blend(feature, &blend_data));
        content += indentLocal +
        QString("UF_MODL_ask_edge_blend called for tag=%1\n").arg(feature);

        content += indentLocal +
        QString("type=%1,radius=%2,edgecount=%3,etags=")
        .arg(blend_data.blend_type)
        .arg(blend_data.blend_radius)
        .arg(blend_data.number_edges);

        for(int i = 0; i < blend_data.number_edges; i++)
        {
        //All edges have condemned status
        /*int status = ::UF_OBJ_ask_status(blend_data.edge_data[i].edge);

        int edge_type;
        UF_CALL(::UF_MODL_ask_edge_type(blend_data.edge_data[i].edge,&edge_type));

        ::UF_OBJ_ask_name(blend_data.edge_data[i].edge, name);*/

        /*content += QString("%1 ").arg(blend_data.edge_data[i].edge);
        }

        content += "\n";
        }*/

        if ( !strcmp( feat_type, "SKETCH" ) )
            preprocessSketch( feature );
        else if ( !strcmp( feat_type, "SWP104" ) )
            preprocessProtrusionExtrude_SWP( feature );

        if ( !strcmp( feat_type, "EXTRUDE" ) )
            preprocessProtrusionExtrude_Extrude( feature );

        ::UF_free( feat_type );
        return;
    }
    else
        visitedFeaturesSet.insert( feature );

    UF_CALL( ::UF_MODL_ask_feat_type( feature, &feat_type ) );

    logger.debug( qPrintable( ( dumpIndent + QString( "Processed feature: tag: %d, name: %s" ) ) ), feature, feat_type );

    if ( !strcmp( feat_type, "BLOCK" ) )
        preprocessBlock( feature );

    ::UF_free( feat_type );

    /*logical hidden_member;
    UF_CALL(::UF_MODL_is_feature_a_hidden_set_member(feature,&hidden_member));

    content += indentLocal +
    QString("UF_MODL_is_feature_a_hidden_set_member called for tag=%1, %2 returned\n")
    .arg(feature).arg(hidden_member);*/

    /*char** parents_name;
    UF_CALL(::UF_MODL_ask_references_of_features(&feature,1,&tagFeatureObjects,&parents_name,&count));

    content += indentLocal +
    QString("UF_MODL_ask_references_of_features called for tag=%1, %2 objects returned\n")
    .arg(feature).arg(count);

    for ( int i = 0; i < count; i++ )
    {
    UF_CALL(::UF_OBJ_ask_type_and_subtype(tagFeatureObjects[i],
    &type,
    &subtype));
    content += indentLocal + QString("objtag%5=%1,name=\"%2\",type=%3,subtype=%4\n")
    .arg(tagFeatureObjects[i]).arg(QTextCodec::codecForLocale()->toUnicode(parents_name[i]))
    .arg(GetTypeName(type)).arg(subtype).arg(i);
    }

    if ( count )
    ::UF_free_string_array(count,parents_name);

    UF_CALL(::UF_MODL_ask_feat_object(feature,
    &count,
    &tagFeatureObjects));*/
    logical isSOObject;

    UF_CALL( ::UF_MODL_ask_feat_relatives( feature,
                                           &objCountParents,
                                           &tagsObjFeatParents,
                                           &objCountChild,
                                           &tagsObjFeatChild ) );

    logger.debug( qPrintable( ( dumpIndent + QString( "Relatives: parents count: %d, childs count: %d" ) ) )
                  , objCountParents, objCountChild );
    dumpIndent += "    ";

    for ( int j = 0; j < objCountParents; j++ )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( tagsObjFeatParents[j], &feat_type ) );

        char* feature_name;
        UF_CALL( ::UF_MODL_ask_feat_name( tagsObjFeatParents[j], &feature_name ) );
        logger.debug( qPrintable( ( dumpIndent + QString( "Parent: tag: %d, name: %s, type: %s" ) ) ),
                      tagsObjFeatParents[j], feature_name, feat_type );
        ::UF_free( feature_name );

        if ( !strcmp( feat_type, "DATUM_CSYS" ) )
        {
            tag_t tagXform = ::UF_MODL_ask_xform_tag_of_datum_csys( tagsObjFeatParents[j] );

            UF_CALL( ::UF_SO_is_so( tagXform, &isSOObject ) );

            if ( isSOObject )
                TraverseSmartObjects( tagXform );
            else
            {
                double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
                preprocessBaseCSYS( matrix );
            }
        }

        if ( !strcmp( feat_type, "SWP104" ) )
            continue;

        if ( !strcmp( feat_type, "EXTRUDE" ) )
            continue;

        ::UF_free( feat_type );

        TraverseFeatureRelatives( tagsObjFeatParents[j], dumpIndent );
    }

    for ( int j = 0; j < objCountChild; j++ )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( tagsObjFeatChild[j], &feat_type ) );

        char* feature_name;
        UF_CALL( ::UF_MODL_ask_feat_name( tagsObjFeatChild[j], &feature_name ) );
        logger.debug( qPrintable( ( dumpIndent + QString( "Child: tag: %d, name: %s, type: %s" ) ) ),
                      tagsObjFeatChild[j], feature_name, feat_type );
        ::UF_free( feature_name );

        if ( !strcmp( feat_type, "DATUM_CSYS" ) )
            continue;

        if ( !strcmp( feat_type, "EXTRUDE" ) && allFeature.at( numberCurrentFeature ) != tagsObjFeatChild[j] )
            continue;

        if ( !strcmp( feat_type, "BLOCK" ) )
            continue;

        ::UF_free( feat_type );

        TraverseFeatureRelatives( tagsObjFeatChild[j], dumpIndent );
    }

    ::UF_free( tagsObjFeatParents );
    ::UF_free( tagsObjFeatChild );
}
#else
void NXExtensionImpl::TraverseFeatureRelatives( tag_t feature, QString dumpIndent )
{
    Category& logger = BaseExtension::GetLogger();

    int objCountParents, objCountChild;
    tag_t* tagsObjFeatParents, *tagsObjFeatChild;
    char* feat_type/*, *feature_name*/;

    if ( visitedFeaturesSet.find( feature ) != visitedFeaturesSet.end() )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( feature, &feat_type ) );

        if ( !strcmp( feat_type, "SKETCH" ) )
            preprocessSketch( feature );
        else if ( !strcmp( feat_type, "SWP104" ) )
            preprocessProtrusionExtrude_SWP( feature );

        if ( !strcmp( feat_type, "EXTRUDE" ) )
            preprocessProtrusionExtrude_Extrude( feature );

        ::UF_free( feat_type );
        return;
    }
    else
        visitedFeaturesSet.insert( feature );

    UF_CALL( ::UF_MODL_ask_feat_type( feature, &feat_type ) );

    logger.debug( qPrintable( ( dumpIndent + QString( "Processed feature: tag: %d, name: %s" ) ) ), feature, feat_type );

    if ( !strcmp( feat_type, "BLOCK" ) )
        preprocessBlock( feature );

    ::UF_free( feat_type );

    logical isSOObject;

    UF_CALL( ::UF_MODL_ask_feat_relatives( feature,
                                           &objCountParents,
                                           &tagsObjFeatParents,
                                           &objCountChild,
                                           &tagsObjFeatChild ) );

    logger.debug( qPrintable( ( dumpIndent + QString( "Relatives: parents count: %d, childs count: %d" ) ) )
                  , objCountParents, objCountChild );
    dumpIndent += "    ";

    if ( objCountParents == 0 )
    {
        if ( !strcmp( feat_type, "DATUM_CSYS" ) )
        {
            double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
            preprocessBaseCSYS( matrix );
        }
    }

    for ( int j = 0; j < objCountParents; j++ )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( tagsObjFeatParents[j], &feat_type ) );

        char* feature_name;
        UF_CALL( ::UF_MODL_ask_feat_name( tagsObjFeatParents[j], &feature_name ) );
        logger.debug( qPrintable( ( dumpIndent + QString( "Parent: tag: %d, name: %s, type: %s" ) ) ),
                      tagsObjFeatParents[j], feature_name, feat_type );
        ::UF_free( feature_name );

        if ( !strcmp( feat_type, "DATUM_CSYS" ) )
        {
            tag_t tagXform = ::UF_MODL_ask_xform_tag_of_datum_csys( tagsObjFeatParents[j] );

            UF_CALL( ::UF_SO_is_so( tagXform, &isSOObject ) );

            if ( isSOObject )
            {
                //TraverseSmartObjects(tagXform);
            }
            else
            {
                double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
                preprocessBaseCSYS( matrix );
            }
        }

        if ( !strcmp( feat_type, "SWP104" ) )
            continue;

        if ( !strcmp( feat_type, "EXTRUDE" ) )
            continue;

        ::UF_free( feat_type );

        TraverseFeatureRelatives( tagsObjFeatParents[j], dumpIndent );
    }

    for ( int j = 0; j < objCountChild; j++ )
    {
        UF_CALL( ::UF_MODL_ask_feat_type( tagsObjFeatChild[j], &feat_type ) );

        char* feature_name;
        UF_CALL( ::UF_MODL_ask_feat_name( tagsObjFeatChild[j], &feature_name ) );
        logger.debug( qPrintable( ( dumpIndent + QString( "Child: tag: %d, name: %s, type: %s" ) ) ),
                      tagsObjFeatChild[j], feature_name, feat_type );
        ::UF_free( feature_name );

        if ( !strcmp( feat_type, "DATUM_CSYS" ) )
            continue;

        if ( !strcmp( feat_type, "EXTRUDE" ) && allFeature.at( numberCurrentFeature ) != tagsObjFeatChild[j] )
            continue;

        if ( !strcmp( feat_type, "BLOCK" ) )
            continue;

        ::UF_free( feat_type );

        TraverseFeatureRelatives( tagsObjFeatChild[j], dumpIndent );
    }

    ::UF_free( tagsObjFeatParents );
    ::UF_free( tagsObjFeatChild );
}

void NXExtensionImpl::Visitor::MapToSketch( double sket_csys[12], double point[3] )
{
    double new_point[3] = {0.0, 0.0, 0.0};
    //double origin[3] = {0.0, 0.0, 0.0};

    /*origin[0] = sket_csys[0] * sket_csys[9] + sket_csys[1] * sket_csys[10] + sket_csys[2] * sket_csys[11];
    origin[1] = sket_csys[3] * sket_csys[9] + sket_csys[4] * sket_csys[10] + sket_csys[5] * sket_csys[11];
    origin[2] = sket_csys[6] * sket_csys[9] + sket_csys[7] * sket_csys[10] + sket_csys[8] * sket_csys[11];*/

    /*new_point[0] = sket_csys[0] * point[0] + sket_csys[1] * point[1] + sket_csys[2] * point[2] + sket_csys[9];
    new_point[1] = sket_csys[3] * point[0] + sket_csys[4] * point[1] + sket_csys[5] * point[2] + sket_csys[10];
    new_point[2] = sket_csys[6] * point[0] + sket_csys[7] * point[1] + sket_csys[8] * point[2] + sket_csys[11];*/

    new_point[0] = sket_csys[0] * point[0] + sket_csys[3] * point[1] + sket_csys[6] * point[2] + sket_csys[9];
    new_point[1] = sket_csys[1] * point[0] + sket_csys[4] * point[1] + sket_csys[7] * point[2] + sket_csys[10];
    new_point[2] = sket_csys[2] * point[0] + sket_csys[5] * point[1] + sket_csys[8] * point[2] + sket_csys[11];

    for ( int i = 0; i < 3; i++ )
    {
        //if (new_point[i] < 0.01) new_point[i] = 0.0;
        point[i] = /*qRound(*/new_point[i]/* * 100) / 100.*/;
    }
}

tag_t NXExtensionImpl::Visitor::TraverseParent( tag_t tagParentFeature )
{
    tag_t tagFirstFeature = NULL_TAG;

    int countParent, countChild;
    tag_t* tagParentFeatures, *tagChildFeatures;
    UF_CALL( ::UF_MODL_ask_feat_relatives(
                 tagParentFeature, &countParent, &tagParentFeatures, &countChild, &tagChildFeatures ) );

    for ( int i = 0; i < countParent; i++ )
    {
        tagFirstFeature = TraverseParent( tagParentFeatures[i] );

        if ( tagFirstFeature != NULL_TAG )
            break;
    }

    if ( countParent == 0 )
        tagFirstFeature = tagParentFeature;

    ::UF_free( tagParentFeatures );
    ::UF_free( tagChildFeatures );

    return tagFirstFeature;
}

tag_t NXExtensionImpl::Visitor::ExtractFirstFeature( tag_t tagBody )
{
    Category& logger = BaseExtension::GetLogger();

    tag_t tagFirstFeature = NULL_TAG;

    uf_list_p_t featureList;

    UF_CALL( ::UF_MODL_ask_body_feats( tagBody, &featureList ) );

    for ( uf_list_p_t feature = featureList; feature != NULL; feature = feature->next )
    {
        char* feat_type;
        char* feat_name;
        UF_CALL( ::UF_MODL_ask_feat_type( feature->eid, &feat_type ) );
        UF_CALL( ::UF_MODL_ask_feat_name( feature->eid, &feat_name ) );
        logger.debug( "Processed feature: tag: %d, type: %s, name: %s", feature->eid, feat_type, feat_name );
        ::UF_free( feat_type );
    }

    for ( uf_list_p_t feature = featureList; feature != NULL; feature = feature->next )
    {
        tagFirstFeature = TraverseParent( feature->eid );

        if ( tagFirstFeature != NULL_TAG )
            break;
    }

    UF_CALL( ::UF_MODL_delete_list( &featureList ) );

    return tagFirstFeature;
}

void CollectParentFeatures( uf_list_p_t suppressFeature, tag_t eid, tag_t skip_eid )
{
    int objCountParents, objCountChild;
    tag_t* tagsObjFeatParents;
    tag_t* tagsObjFeatChild;

    UF_CALL( ::UF_MODL_ask_feat_relatives( eid, &objCountParents, &tagsObjFeatParents, &objCountChild,
                                           &tagsObjFeatChild ) );
    /*char *feature_name;
    UF_CALL(::UF_MODL_ask_feat_name(eid, &feature_name));
    ::UF_free(feature_name);*/

    int i = 0;

    for ( ; i < objCountParents; i++ )
    {
        if ( skip_eid == tagsObjFeatParents[i] )
            continue;

        /*UF_CALL(::UF_MODL_ask_feat_name(tagsObjFeatParents[i], &feature_name));
        ::UF_free(feature_name);*/

        CollectParentFeatures( suppressFeature, tagsObjFeatParents[i], skip_eid );

        UF_CALL( ::UF_MODL_put_list_item( suppressFeature, tagsObjFeatParents[i] ) );
    }

    ::UF_free( tagsObjFeatParents );
    ::UF_free( tagsObjFeatChild );
}

QString NXExtensionImpl::Visitor::PreprocessBody( tag_t tagBody )
{
    //Category& logger = BaseExtension::GetLogger();

    /*tag_t tagFirstFeature = ExtractFirstFeature(tagBody);

    if ( NULL_TAG == tagFirstFeature )
    {
        logger.crit("Unable to find first feature.");
        return;
    }*/

    uf_list_p_t featureList;

    UF_CALL( ::UF_MODL_ask_body_feats( tagBody, &featureList ) );

    /*for ( uf_list_p_t feature = featureList; feature != NULL; feature = feature->next )
    {
        char* feat_type;
        char* feat_name;
        UF_CALL(::UF_MODL_ask_feat_type(feature->eid, &feat_type));
        UF_CALL(::UF_MODL_ask_feat_name(feature->eid, &feat_name));
        logger.debug("Processed feature: tag: %d, type: %s, name: %s",feature->eid,feat_type,feat_name);
        ::UF_free(feat_type);
    }*/

    uf_list_p_t suppressFeature;

    if ( featureList != NULL )
    {
        scriptContent.clear();
        sketch_counter = 1;
        face_counter = 1;
        edge_counter = 1;
        point_counter = 1;
        csys_counter = 1;

        UF_CALL( ::UF_MODL_create_list( &suppressFeature ) );

        //Suppress all the next features
        for ( uf_list_p_t feature = featureList->next; feature != NULL; feature = feature->next )
        {
            CollectParentFeatures( suppressFeature, feature->eid, featureList->eid );
            UF_CALL( ::UF_MODL_put_list_item( suppressFeature, feature->eid ) );
        }

        UF_CALL( ::UF_MODL_suppress_feature( suppressFeature ) );

        UF_CALL( ::UF_MODL_delete_list( &suppressFeature ) );

        PreprocessFeature( featureList->eid );

        for ( uf_list_p_t feature = featureList->next; feature != NULL; feature = feature->next )
        {
            PreprocessFeature( feature->eid );

            UF_CALL( ::UF_MODL_create_list( &suppressFeature ) );

            CollectParentFeatures( suppressFeature, feature->eid, featureList->eid );
            UF_CALL( ::UF_MODL_put_list_item( suppressFeature, feature->eid ) );

            UF_CALL( ::UF_MODL_unsuppress_feature( suppressFeature ) );

            UF_CALL( ::UF_MODL_delete_list( &suppressFeature ) );
        }
    }

    UF_CALL( ::UF_MODL_delete_list( &featureList ) );

    return scriptContent;
}

QString NXExtensionImpl::Visitor::PreprocessFeature( tag_t tagFeature )
{
    Category& logger = BaseExtension::GetLogger();

    char* feat_type = 0x0;

    UF_CALL( ::UF_MODL_ask_feat_type( tagFeature, &feat_type ) );

    char objectName[UF_OBJ_NAME_LEN + 1];
    char* feature_name = 0x0, *name = objectName;

    if ( ::UF_OBJ_ask_name( tagFeature, objectName ) )
    {
        UF_CALL( ::UF_MODL_ask_feat_name( tagFeature, &feature_name ) );
        name = feature_name;
    }

    logger.debug( "Processed feature: tag: %d, type: %s, name: %s", tagFeature, feat_type, name );

    /*if( featureTypeName == "DATUM_CSYS" )
    {
        tag_t tagXform = ::UF_MODL_ask_xform_tag_of_datum_csys(tagFeature);

        logical isSOObject;
        UF_CALL(::UF_SO_is_so(tagXform, &isSOObject));

        if ( isSOObject )
        {
            //TraverseSmartObjects(tagXform);
        }
        else
        {
            //double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
            //preprocessBaseCSYS(matrix);
        }
    } else if( featureTypeName == "BLOCK" )
    {
        //preprocessBlock(feature);
        return;

    } else*/ if ( QString::fromLocal8Bit( feat_type ) == "SWP104" )
    {
        return scriptContent;
        return PreprocessExtrudeFeatureSWP( tagFeature );
    }
    else if ( QString::fromLocal8Bit( feat_type ) == "EXTRUDE" )
        return PreprocessExtrudeFeatureEXTRUDE( tagFeature );

    else
    {
        logger.crit( "Unable to recognize feature %s.", feat_type );
        return scriptContent;
    }

    ::UF_free( feat_type );
    ::UF_free( feature_name );

    /*int countParent, countChild;
    tag_t* tagParentFeatures,*tagChildFeatures;
    UF_CALL(::UF_MODL_ask_feat_relatives(
        tagFeature,&countParent,&tagParentFeatures,&countChild,&tagChildFeatures));

    int nextFeatureIndex = -1;
    for ( int i = 0; i < countChild; i++ )
    {
        UF_CALL(::UF_MODL_ask_feat_type(tagChildFeatures[i], &feat_type));
        featureTypeName = feat_type;
        ::UF_free(feat_type);

        if ( featureTypeName == "SKETCH" )
        {
            //preprocessSketch(tagChildFeatures[i]);
        }
        else if ( featureTypeName == "SWP104" ||
                  featureTypeName == "EXTRUDE" )
        {
            if ( -1 == nextFeatureIndex )
                nextFeatureIndex = i;
            else
            {
                logger.crit("More then one child solid feature exist.");
                nextFeatureIndex = -1;
                break;
            }
        }
    }

    if ( nextFeatureIndex != -1 )
        Preprocess(tagChildFeatures[nextFeatureIndex]);

    ::UF_free(tagParentFeatures);
    ::UF_free(tagChildFeatures);*/
}

QString NXExtensionImpl::Visitor::PreprocessExtrudeFeatureSWP( tag_t tagExtrudeFeature )
{
    Category& logger = BaseExtension::GetLogger();

    int countParent, countChild;
    tag_t* tagParentFeatures, *tagChildFeatures;
    UF_CALL( ::UF_MODL_ask_feat_relatives(
                 tagExtrudeFeature, &countParent, &tagParentFeatures, &countChild, &tagChildFeatures ) );
    ::UF_free( tagChildFeatures );

    char objectName[UF_OBJ_NAME_LEN + 1];
    UF_CALL( ::UF_OBJ_ask_name( tagExtrudeFeature, objectName ) );

    int i = 0;

    for ( ; i < countParent; i++ )
    {
        char* feat_type;
        UF_CALL( ::UF_MODL_ask_feat_type( tagParentFeatures[i], &feat_type ) );

        if ( QString::fromLocal8Bit( feat_type ) == "SKETCH" )
        {
            PreprocessSketch( tagParentFeatures[i] );

            int num_obj;
            tag_t* objects_tag;
            UF_MODL_SWEEP_TRIM_object_p_t trim_ptr;
            char* taper_angle;
            char* limit[2];
            char* offset[2];
            double point[3], direction[3];
            bool region_specified, solid_creation;

            UF_CALL( ::UF_MODL_ask_extrusion( tagExtrudeFeature,
                                              &num_obj, &objects_tag,
                                              &trim_ptr,
                                              &taper_angle,
                                              limit,
                                              offset,
                                              point,
                                              &region_specified, &solid_creation,
                                              direction ) );

            scriptContent += indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"%1\", %3, %4);\n" )
                             .arg( objectName )
                             .arg( sketch_counter++ )
                             .arg( limit[0] )
                             .arg( limit[1] );

            ::UF_free( objects_tag );
            ::UF_free( taper_angle );
            ::UF_free( limit[0] );
            ::UF_free( limit[1] );
            ::UF_free( offset[0] );
            ::UF_free( offset[1] );
            ::UF_free( trim_ptr );

            break;
        }

        ::UF_free( feat_type );
    }

    if ( i == countParent )
        logger.error( "ERROR: Required sketch for %s feature(%d) doesn't exist", objectName, tagExtrudeFeature );

    ::UF_free( tagParentFeatures );

    return scriptContent;
}

QString NXExtensionImpl::Visitor::PreprocessExtrudeFeatureEXTRUDE( tag_t tagExtrudeFeature )
{
    Category& logger = BaseExtension::GetLogger();

    int countParent, countChild;
    tag_t* tagParentFeatures, *tagChildFeatures;
    UF_CALL( ::UF_MODL_ask_feat_relatives(
                 tagExtrudeFeature, &countParent, &tagParentFeatures, &countChild, &tagChildFeatures ) );
    ::UF_free( tagChildFeatures );

    char objectName[UF_OBJ_NAME_LEN + 1];
    char* feature_name = 0x0, *name = objectName;

    if ( ::UF_OBJ_ask_name( tagExtrudeFeature, objectName ) )
    {
        UF_CALL( ::UF_MODL_ask_feat_name( tagExtrudeFeature, &feature_name ) );
        name = feature_name;
    }

    QString str_name = QString( name );
    ::UF_free( feature_name );

    int i = 0;

    for ( ; i < countParent; i++ )
    {
        char* feat_type;
        UF_CALL( ::UF_MODL_ask_feat_type( tagParentFeatures[i], &feat_type ) );

        if ( QString::fromLocal8Bit( feat_type ) == "SKETCH" )
        {
            PreprocessSketch( tagParentFeatures[i] );

            UF_MODL_mswp_extrude_t extrude;

            str_name.remove( str_name.indexOf( "(" ), 1 );
            str_name.remove( str_name.indexOf( ")" ), 1 );

            UF_CALL( ::UF_MODL_mswp_ask_extrude( tagExtrudeFeature, &extrude ) );

            scriptContent += indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"%1\", %3, %4" )
                             .arg( str_name )
                             .arg( sketch_counter++ )
                             .arg( extrude.limits.start_limit.limit_data.distance_data.string )
                             .arg( extrude.limits.end_limit.limit_data.distance_data.string )
                             + ( extrude.sign == UF_NULLSIGN || extrude.sign == UF_POSITIVE ? "" : ", false" ) + ");\n";

            break;
        }

        ::UF_free( feat_type );
    }

    if ( i == countParent )
        logger.error( "ERROR: Required sketch for %s feature(%d) doesn't exist", str_name, tagExtrudeFeature );

    ::UF_free( tagParentFeatures );

    return scriptContent;
}

extern int EvaluateFaceSnapPoint3( const tag_t tagFace, double* point );
extern void EvaluateEdgeSnapPoint( const tag_t tagEdge, double* point );

QString NXExtensionImpl::Visitor::PreprocessSketch( tag_t tagSketchFeature )
{
    Category& logger = BaseExtension::GetLogger();

    int countParent, countChild;
    tag_t* tagParentFeatures, *tagChildFeatures;
    UF_CALL( ::UF_MODL_ask_feat_relatives(
                 tagSketchFeature, &countParent, &tagParentFeatures, &countChild, &tagChildFeatures ) );
    ::UF_free( tagChildFeatures );

    char objectName[UF_OBJ_NAME_LEN + 1];
    char* feature_name = 0x0, *name = objectName;

    if ( ::UF_OBJ_ask_name( tagSketchFeature, objectName ) )
    {
        UF_CALL( ::UF_MODL_ask_feat_name( tagSketchFeature, &feature_name ) );
        name = feature_name;
    }

    uf_list_p_t sketchList;
    UF_CALL( ::UF_SKET_ask_feature_sketches( tagSketchFeature, &sketchList ) );

    if ( sketchList->next != NULL )
        logger.error( "ERROR: More than one sketch detected for %s feature(%d) doesn't exist", name, tagSketchFeature );

    UF_SKET_info_t sketchInfo;
    UF_CALL( ::UF_SKET_ask_sketch_info( sketchList->eid, &sketchInfo ) );
    UF_CALL( ::UF_MODL_delete_list( &sketchList ) );

    double inverted[12];
    InvertMatrix( sketchInfo.csys, inverted );
    double mtx_4D[16], inverted2[16];
    UF_MTX4_initialize( 1, &sketchInfo.csys[9], sketchInfo.csys, mtx_4D );
    UF_MTX4_invert( mtx_4D, inverted2 );

    double A[16] = {1, 3, 5, 1, 9, 2, 7, 2, 5, 7, 4, 1, 8, 8, 5, 3}, AA[16];
    UF_MTX4_invert( A, AA );

    double v[3] = {1, 2, 3}, vv[3];
    UF_MTX4_vec3_multiply( v, A, vv );

    int i = 0;

    for ( ; i < countParent; i++ )
    {
        char* feat_type;
        UF_CALL( ::UF_MODL_ask_feat_type( tagParentFeatures[i], &feat_type ) );

        if ( QString::fromLocal8Bit( feat_type ) == "DATUM_CSYS" )
        {
            PreprocessCSYS( tagParentFeatures[i] );

            scriptContent += indent + QString( "var sketch%3 = SKETCH_Open(\"%1\", %2);\n" )
                             .arg( sketchInfo.name )
                             .arg( name_csys )
                             .arg( sketch_counter );

            tag_t* tagFeatureObjects;
            int type = 0, subtype = 0, count;
            UF_CALL( ::UF_MODL_ask_feat_object( tagSketchFeature, &count, &tagFeatureObjects ) );
            //UF_CALL(::UF_SKET_ask_geoms_of_sketch(tagSketchFeature,&count,&tagSketchGeoms)); doesn't work

            tag_t tagSketch = NULL_TAG;
            UF_CALL( ::UF_SKET_initialize_sketch( sketchInfo.name, &tagSketch ) );

            for ( int i = 0; i < count; i++ )
            {
                UF_CALL( ::UF_OBJ_ask_type_and_subtype( tagFeatureObjects[i], &type, &subtype ) );

                QString t = GetTypeName( type );

                if ( type == UF_sketch_type || type == UF_sketch_tol_csys_type )
                    continue;

                switch ( type )
                {
                    case UF_line_type:
                        UF_CURVE_line_t line_coords;
                        double start[3], end[3];

                        UF_CALL( ::UF_CURVE_ask_line_data( tagFeatureObjects[i], &line_coords ) );

                        UF_SKET_reference_status_t status;
                        UF_CALL( ::UF_SKET_ask_reference_status( tagSketch, tagFeatureObjects[i], &status ) );

                        char objectName[UF_OBJ_NAME_LEN + 1];
                        ::UF_OBJ_ask_name( tagFeatureObjects[i], objectName );

                        start[0] = line_coords.start_point[0];
                        start[1] = line_coords.start_point[1];
                        start[2] = line_coords.start_point[2];
                        UF_MTX4_vec3_multiply( start, inverted2, line_coords.start_point );

                        end[0] = line_coords.end_point[0];
                        end[1] = line_coords.end_point[1];
                        end[2] = line_coords.end_point[2];
                        UF_MTX4_vec3_multiply( end, inverted2, line_coords.end_point );

                        //MapToSketch(inverted, line_coords.start_point);
                        MapToSketch( inverted, start );
                        //MapToSketch(inverted, line_coords.end_point);
                        MapToSketch( inverted, end );

                        scriptContent += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%6, \"%1\", %2, %3, %4, %5" )
                                         .arg( objectName )
                                         .arg( line_coords.start_point[0] )
                                         .arg( line_coords.start_point[1] )
                                         .arg( line_coords.end_point[0] )
                                         .arg( line_coords.end_point[1] )
                                         .arg( sketch_counter ) + ( status == UF_SKET_active ? "" : ", false" ) + ");\n";
                        break;

                    case UF_circle_type:
                        /*UF_CURVE_line_arc_t line_arc_info;
                        UF_CALL(::UF_CURVE_ask_line_arc_data(tagFeatureObjects[i], &line_arc_info)); not appropriate func*/

                        UF_CURVE_arc_t arc_coords;

                        UF_CALL( ::UF_CURVE_ask_arc_data( tagFeatureObjects[i], &arc_coords ) );

                        /*double mtx[9];
                        UF_CALL(::UF_CSYS_ask_matrix_values(arc_coords.matrix_tag, mtx));*/

                        UF_CALL( ::UF_SKET_ask_reference_status( tagSketch, tagFeatureObjects[i], &status ) );

                        ::UF_OBJ_ask_name( tagFeatureObjects[i], objectName );

                        MapToSketch( inverted, arc_coords.arc_center );
                        break;

                    default:
                        break;
                }
            }

            ::UF_free( tagFeatureObjects );

            scriptContent += indent + QString( "SKETCH_Close(sketch%3);\n" ).arg( sketch_counter );

            UF_CALL( ::UF_SKET_terminate_sketch() );

            break;
        }

        ::UF_free( feat_type );
    }

    ::UF_free( feature_name );

    if ( i == countParent )
    {
        //doesn't support sketch without CSYS
        logger.error( "ERROR: Required CSYS for %s feature(%d) doesn't exist", objectName, tagSketchFeature );

        //code outline for non CSYS sketch

        /*tag_t fts[1] = {tagSketchFeature};
        tag_t* ref;
        char** names;
        int cnt;
        UF_CALL(::UF_MODL_ask_references_of_features(fts,1,&ref,&names,&cnt));
        ::UF_free_string_array(cnt,names);

        tag_t face,edge;
        int type = 0, subtype = 0;
        for ( int i = 0; i < cnt; i++ )
        {
            UF_CALL(::UF_OBJ_ask_type_and_subtype(ref[i],&type,&subtype));
            QString t = GetTypeName(type);

            if ( type == UF_solid_type && subtype == UF_solid_face_subtype )
            {
                face = ref[i];
                continue;
            }
            if ( type == UF_solid_type && subtype == UF_solid_edge_subtype )
            {
                edge = ref[i];
                continue;
            }
        }

        ::UF_free(ref);

        if ( face && edge )
        {
            double face_point[3],edge_point1[3],edge_point2[3];
            EvaluateFaceSnapPoint3(face, face_point);

            name_face = "face" + QString::number(face_counter++);

            scriptContent += indent + QString("var %1 = SELECT_Object(%2, %3, %4);\n")
                    .arg(name_face)
                    .arg(QString::number(face_point[0],'f', 2))
                    .arg(QString::number(face_point[1],'f', 2))
                    .arg(QString::number(face_point[2],'f', 2));

            name_edge = "edge" + QString::number(edge_counter++);

            EvaluateEdgeSnapPoint(edge, edge_point1);

            scriptContent += indent + QString("var %4 = SELECT_Object(%1, %2, %3);\n")
                    .arg(QString::number(edge_point1[0],'f', 2))
                    .arg(QString::number(edge_point1[1],'f', 2))
                    .arg(QString::number(edge_point1[2],'f', 2))
                    .arg(name_edge);

            name_point = "point" + QString::number(point_counter++);

            int edge_vertex_count;
            UF_CALL(::UF_MODL_ask_edge_verts(edge,
                                             edge_point1,
                                             edge_point2,
                                             &edge_vertex_count));


            scriptContent += indent + QString("var %4 = SELECT_Object(%1, %2, %3);\n")
                    .arg(QString::number(edge_point1[0],'f', 2))
                    .arg(QString::number(edge_point1[1],'f', 2))
                    .arg(QString::number(edge_point1[2],'f', 2))
                    .arg(name_point);

            name_csys = "new_csys" + QString::number(csys_counter++);
            scriptContent += indent + QString("var %1 = CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point(%2, %3, %4);\n")
                    .arg(name_csys)
                    .arg(name_face)
                    .arg(name_edge)
                    .arg(name_point);
        }

        scriptContent += indent + QString("var sketch%3 = SKETCH_Open(\"%1\", %2);\n")
            .arg(sketchInfo.name)
            .arg(name_csys)
            .arg(sketch_counter);

        tag_t* tagFeatureObjects;
        int count;
        UF_CALL(::UF_MODL_ask_feat_object(tagSketchFeature,&count,&tagFeatureObjects));
        //UF_CALL(::UF_SKET_ask_geoms_of_sketch(tagSketchFeature,&count,&tagSketchGeoms)); doesn't work

        for ( int i = 0; i < count; i++ )
        {
            UF_CALL(::UF_OBJ_ask_type_and_subtype(tagFeatureObjects[i],&type,&subtype));

            if ( type == UF_line_type )
            {
                UF_CURVE_line_t line_coords;

                UF_CALL(::UF_CURVE_ask_line_data(tagFeatureObjects[i], &line_coords));

                char objectName[UF_OBJ_NAME_LEN+1];
                ::UF_OBJ_ask_name(tagFeatureObjects[i], objectName);

                MapToSketch(sketchInfo.csys, line_coords.start_point);
                MapToSketch(sketchInfo.csys, line_coords.end_point);

                scriptContent += indent + QString("SKETCH_Create_2D_Line_2Points(sketch%6, \"%1\", %2, %3, %4, %5);\n")
                        .arg(objectName)
                        .arg(line_coords.start_point[0])
                        .arg(line_coords.start_point[1])
                        .arg(line_coords.end_point[0])
                        .arg(line_coords.end_point[1])
                        .arg(sketch_counter);
            }
        }

        ::UF_free(tagFeatureObjects);

        scriptContent += indent + QString("SKETCH_Close(sketch%3);\n").arg(sketch_counter);*/

        //logger.error("ERROR: Required CSYS for %s feature(%d) doesn't exist",objectName,tagSketchFeature);
    }

    ::UF_free( tagParentFeatures );

    return scriptContent;
}

QString NXExtensionImpl::Visitor::PreprocessCSYS( tag_t tagCSYSFeature )
{
    tag_t tagXform = ::UF_MODL_ask_xform_tag_of_datum_csys( tagCSYSFeature );

    tag_t edge = NULL_TAG, face = NULL_TAG, point_scalar = NULL_TAG, edgeForPoint = NULL_TAG;

    logical isSOObject;
    UF_CALL( ::UF_SO_is_so( tagXform, &isSOObject ) );

    if ( isSOObject )
    {
        int parentCount;
        tag_t* tagsParents = 0x0;
        UF_CALL( ::UF_SO_ask_parents( tagXform, UF_SO_ASK_ALL_PARENTS, &parentCount, &tagsParents ) );

        for ( int i = 0; i < parentCount; i++ )
        {
            int typeSmart, subtypeSmart;
            UF_CALL( ::UF_OBJ_ask_type_and_subtype( tagsParents[i], &typeSmart, &subtypeSmart ) );

            QString t = GetTypeName( typeSmart );

            if ( typeSmart == UF_feature_type )
                continue;

            if ( typeSmart == UF_point_type && parentCount == 1 )
            {
                int childCount;
                tag_t* tagsChilds;
                UF_CALL( ::UF_SO_ask_children( tagXform, UF_SO_ASK_ALL_CHILDREN, &childCount, &tagsChilds ) );

                tag_t tagMatrix;
                double csys_origin[3], matrix[9];

                UF_CALL( ::UF_CSYS_ask_csys_info( tagsChilds[0], &tagMatrix, csys_origin ) );
                ::UF_free( tagsChilds );
                UF_CALL( ::UF_CSYS_ask_matrix_values( tagMatrix, matrix ) );

                name_csys = "base_csys";
                scriptContent += indent + QString( "var matrix = [%1, %2, %3, %4, %5, %6, %7, %8, %9];\n" )
                                 .arg( matrix[0] ).arg( matrix[1] ).arg( matrix[2] )
                                 .arg( matrix[3] ).arg( matrix[4] ).arg( matrix[5] )
                                 .arg( matrix[6] ).arg( matrix[7] ).arg( matrix[8] );
                scriptContent += indent + QString( "var %1 = CONSTRAINTS_Create_Base_CoorSys(matrix);\n" )
                                 .arg( name_csys );

                break;
            }

            if ( typeSmart == UF_solid_type && subtypeSmart == UF_solid_face_subtype )
            {
                face = tagsParents[i];
                continue;
            }

            //logical isSOObject;
            UF_CALL( ::UF_SO_is_so( tagsParents[i], &isSOObject ) );

            if ( isSOObject )
            {
                int parentCount2 = 0;
                tag_t* tagsParents2 = 0x0;
                UF_CALL( ::UF_SO_ask_parents( tagsParents[i], UF_SO_ASK_ALL_PARENTS, &parentCount2, &tagsParents2 ) );

                for ( int j = 0; j < parentCount2; j++ )
                {
                    int typeSmart2, subtypeSmart2;
                    UF_CALL( ::UF_OBJ_ask_type_and_subtype( tagsParents2[j], &typeSmart2, &subtypeSmart2 ) );

                    QString t = GetTypeName( typeSmart2 );

                    if ( typeSmart2 == UF_feature_type )
                        continue;

                    if ( typeSmart == UF_direction_type && subtypeSmart2 == UF_solid_edge_subtype )
                    {
                        double direction_value[3];
                        UF_CALL( ::UF_SO_ask_direction_of_dirr( tagsParents[i], direction_value ) );
                        edge = tagsParents2[j];
                        continue;
                    }

                    if ( typeSmart == UF_direction_type && subtypeSmart2 == UF_solid_face_subtype )
                    {
                        face = tagsParents2[j];
                        continue;
                    }

                    if ( typeSmart == UF_point_type && typeSmart2 == UF_scalar_type )
                    {
                        point_scalar = tagsParents2[j];
                        continue;
                    }

                    if ( typeSmart == UF_point_type && subtypeSmart2 == UF_solid_edge_subtype )
                    {
                        edgeForPoint = tagsParents2[j];
                        continue;
                    }

                    logical isSOObject2;
                    UF_CALL( ::UF_SO_is_so( tagsParents2[j], &isSOObject2 ) );

                    /*if ( isSOObject2 )
                    {
                    }*/
                }

                ::UF_free( tagsParents2 );
            }
        }

        ::UF_free( tagsParents );

        double face_point[3], edge_point1[3], edge_point2[3], *edge_point = NULL;

        if ( face && edge && point_scalar )
        {
            EvaluateFaceSnapPoint3( face, face_point );

            name_face = "face" + QString::number( face_counter++ );

            scriptContent += indent + QString( "var %1 = SELECT_Object(%2, %3, %4);\n" )
                             .arg( name_face )
                             .arg( QString::number( face_point[0], 'f', 2 ) )
                             .arg( QString::number( face_point[1], 'f', 2 ) )
                             .arg( QString::number( face_point[2], 'f', 2 ) );

            name_edge = "edge" + QString::number( edge_counter++ );

            EvaluateEdgeSnapPoint( edge, edge_point1 );

            scriptContent += indent + QString( "var %4 = SELECT_Object(%1, %2, %3);\n" )
                             .arg( QString::number( edge_point1[0], 'f', 2 ) )
                             .arg( QString::number( edge_point1[1], 'f', 2 ) )
                             .arg( QString::number( edge_point1[2], 'f', 2 ) )
                             .arg( name_edge );

            name_point = "point" + QString::number( point_counter++ );

            int edge_vertex_count;
            UF_CALL( ::UF_MODL_ask_edge_verts( edgeForPoint,
                                               edge_point1,
                                               edge_point2,
                                               &edge_vertex_count ) );

            double scalar;
            UF_CALL( ::UF_SO_ask_double_of_scalar( point_scalar, &scalar ) );

            if ( !scalar )
                edge_point = edge_point1;
            else
                edge_point = edge_point2;

            scriptContent += indent + QString( "var %4 = SELECT_Object(%1, %2, %3);\n" )
                             .arg( QString::number( edge_point[0], 'f', 2 ) )
                             .arg( QString::number( edge_point[1], 'f', 2 ) )
                             .arg( QString::number( edge_point[2], 'f', 2 ) )
                             .arg( name_point );

            name_csys = "new_csys" + QString::number( csys_counter++ );
            scriptContent += indent + QString( "var %1 = CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point(%2, %3, %4);\n" )
                             .arg( name_csys )
                             .arg( name_face )
                             .arg( name_edge )
                             .arg( name_point );

            return scriptContent;
        }
    }

    int childCount;
    tag_t* tagsChilds;
    UF_CALL( ::UF_SO_ask_children( tagXform, UF_SO_ASK_ALL_CHILDREN, &childCount, &tagsChilds ) );

    tag_t tagMatrix;
    double csys_origin[3], matrix[9];

    UF_CALL( ::UF_CSYS_ask_csys_info( tagsChilds[0], &tagMatrix, csys_origin ) );
    ::UF_free( tagsChilds );
    UF_CALL( ::UF_CSYS_ask_matrix_values( tagMatrix, matrix ) );

    name_csys = "base_csys";
    scriptContent += indent + QString( "var matrix = [%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12];\n" )
                     .arg( matrix[0] ).arg( matrix[3] ).arg( matrix[6] )
                     .arg( matrix[1] ).arg( matrix[4] ).arg( matrix[7] )
                     .arg( matrix[2] ).arg( matrix[5] ).arg( matrix[8] )
                     .arg( csys_origin[0] ).arg( csys_origin[1] ).arg( csys_origin[2] );
    scriptContent += indent + QString( "var %1 = CONSTRAINTS_Create_Base_CoorSys(matrix);\n" )
                     .arg( name_csys );

    return scriptContent;
}
#endif
