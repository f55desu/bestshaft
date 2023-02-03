#include "../stdafx.h"
#include "../command.h"
#include "../SolidEdgeExtensionImpl.h"


QJSValue SolidEdgeExtensionImpl::SKETCH_Create_2D_Line_2Points( QJSValue sketch,
                                                                QString line_name,
                                                                double x1,
                                                                double y1,
                                                                double x2,
                                                                double y2,
                                                                bool flag )
{
    try
    {
        m_logger.info( "SKETCH_Create_2D_Line_2Points: %s, %f, %f, %f, %f", line_name.toStdString(), x1, y1, x2, y2 );
        SketchPtr pSketch;
        GET_COM_FROM_QSCRIPTVALUE( pSketch, sketch, Sketch );

        if ( !pSketch )
            QSCRIPT_ERROR( "There no active sketch" );

        ProfilePtr pProfile = pSketch->Profile;

        if ( !pProfile )
            QSCRIPT_ERROR( "There no profile" );

        Line2dPtr pLine = pProfile->Lines2d->AddBy2Points( x1, y1, x2, y2 );

        if ( !pLine )
            QSCRIPT_ERROR( "Cannot create line" );
    }
    catch ( const _com_error& e )
    {
        QSCRIPT_COM_ERROR( e );
    }
    catch ( ... )
    {
        QSCRIPT_ERROR( "Unknown exeption in the extension." );
    }

    return QJSValue( 0 );
}

QJSValue SolidEdgeExtensionImpl::SOLID_Create_Block( QJSValue sketch,
                                                     QString name,
                                                     double xOrigin,
                                                     double yOrigin,
                                                     double zOrigin,
                                                     double xLen,
                                                     double yLen,
                                                     double zLen )
{
    try
    {
        m_logger.info( "SOLID_Create_Block: %s, %f, %f, %f, %f, %f, %f",
                       name.toStdString(), xOrigin, yOrigin, zOrigin, xLen, yLen, zLen );
        SketchPtr pSketchPtr;
        GET_COM_FROM_QSCRIPTVALUE( pSketchPtr, sketch, Sketch );

        if ( !pSketchPtr )
            QSCRIPT_ERROR( "There no sketch." );

        ProfilePtr pProfile = pSketchPtr->Profile;

        if ( !pProfile )
            QSCRIPT_ERROR( "Cannot get profile." );

        Lines2dPtr lines2d = pProfile->Lines2d;
        lines2d->AddBy2Points( xOrigin, yOrigin, xOrigin + xLen, yOrigin );
        lines2d->AddBy2Points( xOrigin + xLen, yOrigin, xOrigin + xLen, yOrigin + yLen );
        lines2d->AddBy2Points( xOrigin + xLen, yOrigin + yLen, xOrigin, yOrigin + yLen );
        lines2d->AddBy2Points( xOrigin, yOrigin, xOrigin, yOrigin + yLen );

        if ( pProfile->End( igProfileClosed ) != 0 )
            QSCRIPT_ERROR( "Cannot close profile." );

        long numberOfProfiles = 1;
        SAFEARRAYBOUND sabProfiles;
        sabProfiles.lLbound = 0;
        sabProfiles.cElements = numberOfProfiles;
        SAFEARRAY* saProfiles = SafeArrayCreate( VT_DISPATCH, 1, &sabProfiles );

        if ( !saProfiles )
            QSCRIPT_ERROR( "Cannot create safearray." );

        long index = 0;
        HRESULT hr = SafeArrayPutElement( saProfiles, &index, pProfile );

        if ( !SUCCEEDED( hr ) )
            QSCRIPT_ERROR( "Cannot put element in safearray." );

        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "Cannot get active document." );

        ModelPtr pBlock = pPartDocument->Models->AddFiniteExtrudedProtrusion( numberOfProfiles,
                                                                              &saProfiles,
                                                                              igRight,
                                                                              zLen );

        if ( !pBlock )
            QSCRIPT_ERROR( "Cannot create block." );

        pBlock->Name = qPrintable( name );
    }
    catch ( const _com_error& e )
    {
        QSCRIPT_COM_ERROR( e );
    }
    catch ( ... )
    {
        QSCRIPT_ERROR( "Unknown exeption in the extension." );
    }

    return QJSValue();
}

QJSValue SolidEdgeExtensionImpl::SOLID_Create_Protrusion_Extrude( QJSValue sketch,
                                                                  QString extrude_name,
                                                                  double depth_start,
                                                                  double depth_end, bool sign )
{
    try
    {
        m_logger.info( "SOLID_Create_Protrusion_Extrude: %s, %f, %f", extrude_name.toStdString(), depth_start, depth_end );
        SketchPtr pSketch;
        GET_COM_FROM_QSCRIPTVALUE( pSketch, sketch, Sketch );

        if ( !pSketch )
            QSCRIPT_ERROR( "There no sketch." );

        ProfilePtr pProfile = pSketch->Profile;

        if ( !pProfile )
            QSCRIPT_ERROR( "There no profile." );

        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "Cannot get part document." );

        long numberOfProfiles = 1;
        SAFEARRAYBOUND sabProfiles;
        sabProfiles.lLbound = 0;
        sabProfiles.cElements = numberOfProfiles;
        SAFEARRAY* saProfiles = SafeArrayCreate( VT_DISPATCH, 1, &sabProfiles );

        if ( !saProfiles )
            QSCRIPT_ERROR( "Cannot create safearray." );

        long index = 0;
        HRESULT hr = SafeArrayPutElement( saProfiles, &index, pProfile );

        if ( !SUCCEEDED( hr ) )
        {
            SafeArrayDestroy( saProfiles );
            QSCRIPT_ERROR( "Cannot create safearray" );
        }

        ModelPtr pModel = pPartDocument->Models->
                          AddFiniteExtrudedProtrusion( numberOfProfiles,
                                                       &saProfiles,
                                                       igRight,
                                                       ( depth_end - depth_start ) );

        if ( !pModel )
        {
            SafeArrayDestroy( saProfiles );
            QSCRIPT_ERROR( "Cannot create Extruded Protrusion." );
        }

        pModel->Name = qPrintable( extrude_name );

        SafeArrayDestroy( saProfiles );
    }
    catch ( const _com_error& e )
    {
        QSCRIPT_COM_ERROR( e );
    }
    catch ( ... )
    {
        QSCRIPT_ERROR( "Unknown exeption in the extension." );
    }

    return QJSValue( 0 );
}

void SolidEdgeExtensionImpl::SOLID_Create_Block_WDH( QJSValue sketch,
                                                     double x, double y, double z,
                                                     double width, double depth,
                                                     double height )
{
    m_logger.info( "SOLID_Create_Block_WDH: %f, %f, %f, %f, %f, %f",
                   x, y, z, width, depth, height );
    SOLID_Create_Block( sketch, "", x, y, z, width, depth, height );
}

void SolidEdgeExtensionImpl::SOLID_Create_Block_2Points( QJSValue sketch,
                                                         double x1, double y1, double z1,
                                                         double x2, double y2, double z2 )
{
    m_logger.info( "SOLID_Create_Block_2Points: %f, %f, %f, %f, %f, %f",
                   x1, y1, z1, x2, y2, z2 );
}

QJSValue SolidEdgeExtensionImpl::createPointOnCurve( QJSValue curve, double t )
{
    m_logger.info( "Creating point on curve" );
    return QJSValue();
}
