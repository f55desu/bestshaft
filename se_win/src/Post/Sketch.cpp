#include "../stdafx.h"
#include "../commands.h"
#include "../SolidEdgeExtensionImpl.h"

QJSValue SolidEdgeExtensionImpl::SKETCH_Open( QString skt_name, QJSValue csys )
{
    try
    {
        m_logger.info( "Open sketch %s", skt_name.toStdString() );
        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "Cannot get part document." );

        SketchsPtr pSketchsCollection = pPartDocument->Sketches;

        if ( !pSketchsCollection )
            QSCRIPT_ERROR( "Cannot get sketches collection." );

        for ( int i = 1; i <= pSketchsCollection->Count; i++ )
        {
            SketchPtr pSketch = pSketchsCollection->Item( i );

            if ( !pSketch )
                QSCRIPT_ERROR( qPrintable( QString( "Cannot get sketch %1 from part document" ).arg( i ) ) );

            if ( QString( pSketch->Name ) == skt_name )
                GET_QSCRIPTVALUE_FROM_COM( pSketch );
        }

        QJSValue plane = getPlaneXOY( csys );
        RefPlanePtr pPlane;
        GET_COM_FROM_QSCRIPTVALUE( pPlane, plane, RefPlane );

        if ( !pPlane )
            QSCRIPT_ERROR( "Cannot add plane to sketch." );

        SketchPtr pNewSketch = pSketchsCollection->AddByPlane( pPlane );
        pNewSketch->Name = qPrintable( skt_name );

        return GET_QSCRIPTVALUE_FROM_COM( pNewSketch );
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

QJSValue SolidEdgeExtensionImpl::SKETCH_Close( QJSValue sketch )
{
    try
    {
        m_logger.info( "Closing sketch" );
        SketchPtr pSketch;
        GET_COM_FROM_QSCRIPTVALUE( pSketch, sketch, Sketch );

        if ( !pSketch )
            QSCRIPT_ERROR( "There no sketch." );

        ProfilePtr pProfile = pSketch->Profile;

        if ( !pProfile )
            QSCRIPT_ERROR( "There no profile in sketch." );

        int result = pProfile->End( igProfileClosed );

        if ( result != 0 )
            QSCRIPT_ERROR( "The profile is not closed." );
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

QJSValue SolidEdgeExtensionImpl::getActiveSketch()
{
    try
    {
        m_logger.info( "Getting active sketch" );
        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "getActiveSketch: Cannot get part document." );

        SketchPtr pSketch = pPartDocument->ActiveSketch;
        return GET_QSCRIPTVALUE_FROM_COM( pSketch );
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
