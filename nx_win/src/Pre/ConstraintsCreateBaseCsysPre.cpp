#include "../Stable.h"
#include "../NXExtensionImpl.h"
#include "../nxUtils.h"

void NXExtensionImpl::preprocessBaseCSYS( double matrix[9] )
{
    name_csys = "base_csys";
    content += indent + QString( "var matrix = [%1, %2, %3, %4, %5, %6, %7, %8, %9];\n" )
               .arg( matrix[0] ).arg( matrix[1] ).arg( matrix[2] )
               .arg( matrix[3] ).arg( matrix[4] ).arg( matrix[5] )
               .arg( matrix[6] ).arg( matrix[7] ).arg( matrix[8] );
    content += indent + QString( "var %1 = CONSTRAINTS_Create_Base_CoorSys(matrix);\n" )
               .arg( name_csys );
}
