#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

QJSValue NXExtensionImpl::DUMP_All()
{
    tag_t tagPart = ::UF_ASSEM_ask_work_part();

    QString list;
    tag_t object = NULL_TAG;
    int type, subtype;
    object = ::UF_OBJ_cycle_all( tagPart, object );

    while ( object != NULL_TAG )
    {
        string name( UF_OBJ_NAME_LEN + 1, '\0' );
        UF_CALL( ::UF_OBJ_ask_type_and_subtype( object, &type, &subtype ) );
        UF_CALL( ::UF_OBJ_ask_name( object, &name[0] ) );
        list += QString( "tag=%1,name=\"%2\",type=%3,subtype=%4\n" )
                .arg( object ).arg( name.c_str() ).arg( GetTypeName( type ) ).arg( subtype );
        object = ::UF_OBJ_cycle_all( tagPart, object );
    }

    return list;
}
