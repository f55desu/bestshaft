#include "../stdafx.h"
#include "../SolidEdgeExtensionImpl.h"
#include "../commands.h"
#include "../util.h"

void SolidEdgeExtensionImpl::preprocessSketch( const SketchPtr& sketch )
{
    QString name = GET_QSTRING_FROM_BSTR( sketch->Name );
    m_content += m_indent + QString( "var sketch%1 = SKETCH_Open(\"%2\", %3);\n" )
                 .arg( ++m_countFeatureSketch ).arg( name ).arg( name_csys );
}

QString SolidEdgeExtensionImpl::preprocessSketchOpen()
{
    return QString();
}
