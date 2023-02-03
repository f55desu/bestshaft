#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::SOLID_Create_Block( QString name,
                                              /*  QJSValue base_csys,*/
                                              double xOrigin,
                                              double yOrigin,
                                              double zOrigin,
                                              double xLen,
                                              double yLen,
                                              double zLen )
{
    int result;
    tag_t tagBlock = NULL_TAG;

    double block_orig[3] = {xOrigin, yOrigin, zOrigin};
    std::string xLenStr = QString::number( xLen ).toStdString();
    std::string yLenStr = QString::number( yLen ).toStdString();
    std::string zLenStr = QString::number( zLen ).toStdString();
    char* block_len[3] = {( char* )xLenStr.c_str(),
                          ( char* )yLenStr.c_str(),
                          ( char* )zLenStr.c_str()
                         };

    CheckFirstSolidInPart();

    if ( firstSolidFlag )
    {
        result = UF_MODL_create_block1( UF_POSITIVE, block_orig, block_len, &tagBlock );
        UF_CALL( result );
    }
    else
    {
        result = UF_MODL_create_block1( UF_NULLSIGN, block_orig, block_len, &tagBlock );
        UF_CALL( result );
    }

    std::string strName = qPrintable( name );
    result = UF_OBJ_set_name( tagBlock, strName.c_str() );
    UF_CALL( result );

    return QJSValue( result );
}

void NXExtensionImpl::SOLID_Create_Block_WDH( double x,
                                              double y,
                                              double z,
                                              double width,
                                              double depth,
                                              double height )
{
    char wd[10] = "";
    strcpy( wd, QString::number( width ).toStdString().c_str() );

    char dp[10] = "";
    strcpy( dp, QString::number( depth ).toStdString().c_str() );

    char hg[10] = "";
    strcpy( hg, QString::number( height ).toStdString().c_str() );

    UF_FEATURE_SIGN sign = UF_NULLSIGN;
    double block_orig[3] = {x, y, z};

    char* block_len[3] = {wd, dp, hg};
    tag_t blk_obj;

    UF_CALL( UF_MODL_create_block1( sign, block_orig, block_len, &blk_obj ) );
}

void NXExtensionImpl::SOLID_Create_Block_2Points( double x1,
                                                  double y1,
                                                  double z1,
                                                  double x2,
                                                  double y2,
                                                  double z2 )
{
    double width, depth, height;
    width = x2 - x1;
    depth = y2 - y1;
    height = z2 - z1;

    char wd[10] = "";
    strcpy( wd, QString::number( width ).toStdString().c_str() );

    char dp[10] = "";
    strcpy( dp, QString::number( depth ).toStdString().c_str() );

    char hg[10] = "";
    strcpy( hg, QString::number( height ).toStdString().c_str() );

    UF_FEATURE_SIGN sign = UF_NULLSIGN;
    double block_orig[3] = {x1, y1, z1};

    char* block_len[3] = {wd, dp, hg};
    tag_t blk_obj;

    UF_CALL( UF_MODL_create_block1( sign, block_orig, block_len, &blk_obj ) );
}

/*QJSValue NXExtensionImpl::SOLID_Create_Block(QScriptContext *pcontext, QScriptEngine *pengine)
{
    if ( pcontext->argumentCount() < 6)
    {
        pengine->abortEvaluation(
            pcontext->throwError(QScriptContext::SyntaxError,
                                 "Arguments of function mismatch."));
    }

    double x = pcontext->argument(0).toNumber();
    double y = pcontext->argument(1).toNumber();
    double z = pcontext->argument(2).toNumber();

    UF_FEATURE_SIGN sign = UF_NULLSIGN;
    //UF_FEATURE_SIGN sign1 = UF_POSITIVE;
    double block_orig[3] = {x,y,z};
    char *block_len[3] = {"1","2","3"};
    tag_t blk_obj;

    UF_CALL(UF_MODL_create_block1(sign, block_orig, block_len, &blk_obj));
    return QJSValue(0);
}*/
