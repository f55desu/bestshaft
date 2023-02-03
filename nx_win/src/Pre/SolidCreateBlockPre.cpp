#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

QString blockPre( char* size[3], double location[3],
                  QString name, QString name_csys, int count_feature_sketch )
{
    int numLine = 1;
    QString content;
    QString indent = "   ";
    content += indent + QString( "var sketch%3 = SKETCH_Open(\"sketch_%1\", %2);\n" )
               .arg( name )
               .arg( name_csys )
               .arg( ++count_feature_sketch );

    content += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%7,\"line%1_%6\", %2, %3, %4, %5);\n" )
               .arg( numLine )
               .arg( location[0] )
               .arg( location[1] )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[0]/*).remove(0, 3)*/ )
               .arg( location[0] )
               .arg( name )
               .arg( count_feature_sketch );

    content += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%7,\"line%1_%6\", %2, %3, %4, %5);\n" )
               .arg( ++numLine )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[0]/*).remove(0, 3)*/ )
               .arg( location[0] )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[0]/*).remove(0, 3)*/ )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[1]/*).remove(0, 3)*/ )
               .arg( name )
               .arg( count_feature_sketch );

    content += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%7,\"line%1_%6\", %2, %3, %4, %5);\n" )
               .arg( ++numLine )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[0]/*).remove(0, 3)*/ )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[1]/*).remove(0, 3)*/ )
               .arg( location[0] )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[1]/*).remove(0, 3)*/ )
               .arg( name )
               .arg( count_feature_sketch );

    content += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%7,\"line%1_%6\", %2, %3, %4, %5);\n" )
               .arg( ++numLine )
               .arg( location[0] )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[1]/*).remove(0, 3)*/ )
               .arg( location[0] )
               .arg( location[1] )
               .arg( name )
               .arg( count_feature_sketch );

    content += indent + "SKETCH_Close();\n";

    content += indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"Extrude_%1\", 0, %3);\n" )
               .arg( name )
               .arg( count_feature_sketch )
               .arg( /*QTextCodec::codecForLocale()->toUnicode(*/size[2]/*).remove(0, 3)*/ );

    return content;
}

void NXExtensionImpl::preprocessBlock( tag_t feature )
{
    char* size[3];
    QString str_name;
    tag_t feature_array[1] = {feature};
    int num_features = 1,
        num_parents;
    tag_t* parents;
    char** parent_names;
    double origin[3];
    int type, subtype;
    char name[UF_OBJ_NAME_LEN + 1];
    char* feature_name;

    UF_CALL( ::UF_MODL_ask_block_parms( feature, 1, size ) );
    UF_CALL( ::UF_MODL_ask_feat_location( feature, origin ) );

    UF_CALL( ::UF_MODL_ask_references_of_features( feature_array, num_features,
                                                   &parents, &parent_names, &num_parents ) );

    if ( num_parents > 0 )
    {
        UF_CALL( ::UF_OBJ_ask_type_and_subtype( parents[0],
                                                &type,
                                                &subtype ) );
    }

    if ( !::UF_OBJ_ask_name( feature, name ) )
        str_name = name;
    else
    {
        UF_CALL( ::UF_MODL_ask_feat_name( feature, &feature_name ) );
        str_name = QString( feature_name );
        str_name.remove( str_name.indexOf( "(" ), 1 );
        str_name.remove( str_name.indexOf( ")" ), 1 );
        ::UF_free( feature_name );
    }

    /*content += QString("SOLID_Create_Block(\"%1\", %5, %6, %7, %2, %3, %4);\n")
    .arg(str_name)
    .arg(QTextCodec::codecForLocale()->toUnicode(size[0]).remove(0, 3))
    .arg(QTextCodec::codecForLocale()->toUnicode(size[1]).remove(0, 3))
    .arg(QTextCodec::codecForLocale()->toUnicode(size[2]).remove(0, 3))
    .arg(origin[0]).arg(origin[1]).arg(origin[2]);*/

    if ( num_parents == 0 || type == UF_point_type )
    {
        double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
        preprocessBaseCSYS( matrix );

        content += blockPre( size, origin, str_name, name_csys, count_feature_sketch );
    }

    if ( num_parents >= 1 )
    {
        ::UF_free( parents );
        ::UF_free_string_array( num_parents, parent_names );
    }
}
