#include "../Stable.h"
#include "../NXExtensionImpl.h"

QScriptValue NXExtensionImpl::SKETCH_Open(QString sketchName)
{
    if ( m_selectManager.size() < 2 )
        return m_scriptEngine->currentContext()->throwError(
            "At least two selected objects are required to create the sketch.");

    std::string strName = qPrintable(sketchName);
    strName.resize(MAX_ENTITY_NAME_SIZE+1);
    tag_t tagNewSketch = NULL_TAG;
    UF_CALL(::UF_SKET_initialize_sketch(&strName[0],&tagNewSketch));

    /*tag_t tagDatumAxis = NULL_TAG;
    UF_CALL(::UF_OBJ_cycle_objs_in_part(::UF_ASSEM_ask_work_part(),
                                        UF_datum_axis_type,&tagDatumAxis));
    tag_t object[2] = {tagPlane,tagDatumAxis};*/
/*UF_MODL_create_datum_csys
UF_MODL_set_datum_csys_visibility
UF_SO_set_visibility_option
UF_SKET_attach_to_face*/
    tag_t object[2] = {m_selectManager[0],m_selectManager[1]};
    int reference[2] = {1,1};
    UF_CALL(::UF_SKET_create_sketch(&strName[0],
                                    1,
                                    NULL,
                                    object,
                                    reference,
                                    1,
                                    &tagNewSketch));


    m_selectManager.clear();
    m_selectManager.push_back(tagNewSketch);

    return QScriptValue(tagNewSketch);
}

void NXExtensionImpl::SKETCH_Close()
{
    UF_CALL(::UF_SKET_terminate_sketch());
    m_selectManager.clear();
}
