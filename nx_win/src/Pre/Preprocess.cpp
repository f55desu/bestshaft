#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

QString NXExtensionImpl::PreprocessModel()
{
    Category& logger = BaseExtension::GetLogger();
    logger.info( "Start preprocessing." );

    if ( NULL_TAG == ::UF_PART_ask_display_part() )
    {
        QString mes = tr( "Model doesn't exist." );
        logger.info( mes.toUtf8().constData() );

        BaseExtension::EnterModalLoop();
        QMessageBox::critical( GetTopWindow(), tr( "Paratran preprocessing" ), mes );
        BaseExtension::ExitModalLoop();

        return QString( "" );
    }

    int count = 0;
    tag_t tagSolidBody = NULL_TAG, tagFirstSolidBody = NULL_TAG;

    ::UF_MODL_ask_object( UF_solid_type, UF_solid_body_subtype, &tagSolidBody );

    if ( tagSolidBody == NULL_TAG )
    {
        QString mes = tr( "No solid body to proceed." );
        logger.info( mes.toUtf8().constData() );

        BaseExtension::EnterModalLoop();
        QMessageBox::critical( GetTopWindow(), tr( "Paratran preprocessing" ), mes );
        BaseExtension::ExitModalLoop();

        return QString( "" );
    }
    else
    {
        tagFirstSolidBody = tagSolidBody;

        do
        {
            count++;
            ::UF_MODL_ask_object( UF_solid_type, UF_solid_body_subtype, &tagSolidBody );
        } while ( tagSolidBody != NULL_TAG );
    }

    if ( count > 1 )
        logger.info( "More than one solid bodies exist in the part. The first in the list is accepted." );

    return visitor.PreprocessBody( tagFirstSolidBody );

    uf_list_p_t featureList;

    UF_CALL( ::UF_MODL_ask_body_feats( tagFirstSolidBody, &featureList ) );

    name_csys.clear();
    name_edge.clear();
    name_face.clear();
    name_point.clear();
    count_feature_sketch = 0;
    count_new_csys = 0;
    count_edge = 0;
    count_face = 0;
    count_point_on_edge = 0;
    //isSoDirection = true;
    allFeature.clear();
    numberCurrentFeature = 0;

    content.clear();
    visitedFeaturesSet.clear();
    visitedSmartObjectsSet.clear();

    for ( uf_list_p_t feature = featureList; feature != NULL; feature = feature->next )
        allFeature.push_back( feature->eid );

    logger.debug( "%d features", allFeature.size() );

    for ( uf_list_p_t feature = featureList; feature != NULL; feature = feature->next, numberCurrentFeature++ )
    {
        if ( ::UF_OBJ_ask_status( feature->eid ) == UF_OBJ_ALIVE )
        {
            int suppressed;
            UF_CALL( ::UF_MODL_ask_suppress_feature( feature->eid, &suppressed ) );

            if ( suppressed )
            {
                char* feature_name;
                UF_CALL( ::UF_MODL_ask_feat_name( feature->eid, &feature_name ) );
                logger.debug( "Feature suppresed: tag: %d, name: %s", feature->eid, feature_name );
                ::UF_free( feature_name );
                continue;
            }

            TraverseFeatureRelatives( feature->eid, QString() );
        }
    }

    UF_CALL( ::UF_MODL_delete_list( &featureList ) );

    logger.info( "End preprocessing." );

    return content;
}
