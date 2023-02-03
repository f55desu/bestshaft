#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

QString preprocessSketchOpen( QString content, QString indent, QString name, QString name_csys, int num )
{
    return content += indent + QString( "var sketch%3 = SKETCH_Open(\"%1\", %2);\n" )
                      .arg( name )
                      .arg( name_csys )
                      .arg( num );
}

QString preprocessSketchClose( QString content, QString indent, int num )
{
    return content += indent + QString( "SKETCH_Close(sketch%3);\n" ).arg( num );
}

struct SketchObject
{
    tag_t tagObject;
    QString name;
};

bool compareString ( const SketchObject& sk1, const SketchObject& sk2 )
{
    return sk1.name.toLower() < sk2.name.toLower();
}

void NXExtensionImpl::preprocessSketch( tag_t feature )
{
    uf_list_p_t sketchList;
    UF_SKET_info_t sketchInfo;
    tag_t* tagFeatureObjects;
    int type = 0, subtype = 0, count;

    SketchObject sketchObject;
    QVector<SketchObject> sketchObjectVector;
    char name[UF_OBJ_NAME_LEN + 1];
    //  int sketchCount;

    UF_CALL( ::UF_SKET_ask_feature_sketches( feature, &sketchList ) );

    /* UF_CALL(::UF_MODL_ask_list_count(sketchList, &sketchCount));

    for (uf_list_p_t isketch = sketchList; isketch != NULL; isketch = isketch->next)
    {
    UF_CALL(::UF_SKET_ask_sketch_info(isketch->eid, &sketchInfo));

    content = preprocessSketchOpen(content, indent,
    QTextCodec::codecForLocale()->toUnicode(sketchInfo.name),
    name_csys,
    ++count_feature_sketch);
    }

    UF_CALL(::UF_MODL_delete_list(&sketchList));*/

    UF_CALL( ::UF_SKET_ask_sketch_info( sketchList->eid, &sketchInfo ) );

    content = preprocessSketchOpen( content, indent,
                                    /*QTextCodec::codecForLocale()->toUnicode(*/sketchInfo.name/*)*/,
                                    name_csys,
                                    ++count_feature_sketch );

    UF_CALL( ::UF_MODL_ask_feat_object( feature,
                                        &count,
                                        &tagFeatureObjects ) );

    for ( int i = 0; i < count; i++ )
    {
        UF_CALL( ::UF_OBJ_ask_type_and_subtype( tagFeatureObjects[i],
                                                &type,
                                                &subtype ) );

        if ( type != UF_sketch_type && type != UF_sketch_tol_csys_type )
        {
            ::UF_OBJ_ask_name( tagFeatureObjects[i], name );
            sketchObject.name = name;
            sketchObject.tagObject = tagFeatureObjects[i];

            sketchObjectVector.push_back( sketchObject );
        }
    }

    std::sort( sketchObjectVector.begin(), sketchObjectVector.end(), &compareString );

    for ( int i = 0; i < sketchObjectVector.size(); i++ )
    {
        UF_CALL( ::UF_OBJ_ask_type_and_subtype( sketchObjectVector.at( i ).tagObject,
                                                &type,
                                                &subtype ) );

        if ( type == UF_line_type )
            preprocessLine2Points( sketchObjectVector.at( i ).tagObject, sketchInfo.csys, count_feature_sketch );
    }

    content = preprocessSketchClose( content, indent, count_feature_sketch );
}

void NXExtensionImpl::MapToSketch( double sket_csys[12], double point[3] )
{
    /*double new_point[3] = {0.0, 0.0, 0.0};
    double origin[3] = {0.0, 0.0, 0.0};

    origin[0] = sket_csys[0] * sket_csys[9] + sket_csys[1] * sket_csys[10] + sket_csys[2] * sket_csys[11];
    origin[1] = sket_csys[3] * sket_csys[9] + sket_csys[4] * sket_csys[10] + sket_csys[5] * sket_csys[11];
    origin[2] = sket_csys[6] * sket_csys[9] + sket_csys[7] * sket_csys[10] + sket_csys[8] * sket_csys[11];

    new_point[0] = sket_csys[0] * point[0] + sket_csys[1] * point[1] + sket_csys[2] * point[2] - origin[0];
    new_point[1] = sket_csys[3] * point[0] + sket_csys[4] * point[1] + sket_csys[5] * point[2] - origin[1];
    new_point[2] = sket_csys[6] * point[0] + sket_csys[7] * point[1] + sket_csys[8] * point[2] - origin[2];

    for (int i = 0; i < 3; i++)
    {
        if (new_point[i] < 0.01) new_point[i] = 0.0;
        point[i] = qRound(new_point[i] * 100) / 100.;
    }*/

    double new_point[3] = {0.0, 0.0, 0.0};

    new_point[0] = sket_csys[0] * point[0] + sket_csys[3] * point[1] + sket_csys[6] * point[2] + sket_csys[9];
    new_point[1] = sket_csys[1] * point[0] + sket_csys[4] * point[1] + sket_csys[7] * point[2] + sket_csys[10];
    new_point[2] = sket_csys[2] * point[0] + sket_csys[5] * point[1] + sket_csys[8] * point[2] + sket_csys[11];

    for ( int i = 0; i < 3; i++ )
        point[i] = new_point[i];
}
