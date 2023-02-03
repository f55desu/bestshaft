#include "../Stable.h"
#include "../NXExtensionImpl.h"

QJSValue NXExtensionImpl::SKETCH_Open( QString skt_name, QJSValue csys )
{
    if ( csys.toInt() == 0 )
    {
        m_scriptEngine->throwError( QString( "Coordinate system doesn't exist." ) );
        return QJSValue();
    }

    //std::string sketch_name = skt_name.toStdString();
    int result,
        sketch_option = 1,
        sketch_planeDir = 1,
        sketch_reference[2] = {1, 1};
    tag_t sketch_object[2],
          new_sketch = NULL_TAG;
    objectList = 0x0;

    sketch_object[0] = getPlaneXOY( csys ).toInt();
    sketch_object[1] = getAxisOX( csys ).toInt();

    result = UF_SKET_initialize_sketch( ( char* )qPrintable( skt_name ) /*(char*)sketch_name.c_str()*/, &new_sketch );
    UF_CALL( result );

    result = UF_SKET_create_sketch( ( char* )qPrintable( skt_name )/*(char*)sketch_name.c_str()*/,
                                    sketch_option,
                                    NULL,
                                    sketch_object,
                                    sketch_reference,
                                    sketch_planeDir,
                                    &new_sketch );
    UF_CALL( result );

    if ( result )
    {
        m_scriptEngine->throwError( QString( "Sketch's name already exists" ) );
        return QJSValue();
    }

    result = UF_MODL_create_list( &objectList );
    UF_CALL( result );

    return QJSValue( new_sketch );
}

QJSValue NXExtensionImpl::SKETCH_Close( QJSValue sketch )
{
    Q_UNUSED( sketch );

    int result = UF_SKET_terminate_sketch();
    UF_CALL( result );

    return QJSValue( result );
}

/*QJSValue NXExtensionImpl::getActiveSketch()
{
    tag_t active_sketch = NULL_TAG;
    int result = UF_SKET_ask_active_sketch (&active_sketch);
    UF_CALL(result);

    return QJSValue(active_sketch);
}*/

void NXExtensionImpl::MapToWorld( double sket_csys[12], double point[3] )
{
    double new_point[3] = {0.0, 0.0, 0.0};
    /*double origin[3] = {0.0, 0.0, 0.0};

    origin[0] = sket_csys[0] * sket_csys[9] + sket_csys[1] * sket_csys[10] + sket_csys[2] * sket_csys[11];
    origin[1] = sket_csys[3] * sket_csys[9] + sket_csys[4] * sket_csys[10] + sket_csys[5] * sket_csys[11];
    origin[2] = sket_csys[6] * sket_csys[9] + sket_csys[7] * sket_csys[10] + sket_csys[8] * sket_csys[11];

    new_point[0] = sket_csys[0] * point[0] + sket_csys[3] * point[1] + sket_csys[6] * point[2] + origin[0];
    new_point[1] = sket_csys[1] * point[0] + sket_csys[4] * point[1] + sket_csys[7] * point[2] + origin[1];
    new_point[2] = sket_csys[2] * point[0] + sket_csys[5] * point[1] + sket_csys[8] * point[2] + origin[2];*/

    new_point[0] = sket_csys[0] * point[0] + sket_csys[3] * point[1] + sket_csys[6] * point[2] + sket_csys[9];
    new_point[1] = sket_csys[1] * point[0] + sket_csys[4] * point[1] + sket_csys[7] * point[2] + sket_csys[10];
    new_point[2] = sket_csys[2] * point[0] + sket_csys[5] * point[1] + sket_csys[8] * point[2] + sket_csys[11];

    for ( int i = 0; i < 3; i++ )
        point[i] = new_point[i];
}
