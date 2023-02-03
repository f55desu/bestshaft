#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

void NXExtensionImpl::preprocessLine2Points( tag_t feature, double sketchCSYS[12], int num )
{
    UF_CURVE_line_t line_coords;
    char name[UF_OBJ_NAME_LEN + 1];

    UF_CALL( ::UF_CURVE_ask_line_data( feature, &line_coords ) );

    ::UF_OBJ_ask_name( feature, name );

    MapToSketch( sketchCSYS, line_coords.start_point );
    MapToSketch( sketchCSYS, line_coords.end_point );

    content += indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%6, \"%1\", %2, %3, %4, %5);\n" )
               .arg( name )
               .arg( line_coords.start_point[0] )
               .arg( line_coords.start_point[1] )
               .arg( line_coords.end_point[0] )
               .arg( line_coords.end_point[1] )
               .arg( num );
}
