#include "../stdafx.h"
#include "../commands.h"
#include "../SolidEdgeExtensionImpl.h"

QJSValue SolidEdgeExtensionImpl::createBaseCsys( double matrix[9] )
{
    try
    {
        m_logger.info( "Creating base Coordinate System" );
        PartDocumentPtr pPartDoc = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDoc )
            QSCRIPT_ERROR( "There no document." );

        CoordinateSystemsPtr pCsysCollection = pPartDoc->CoordinateSystems;

        if ( !pCsysCollection )
            QSCRIPT_ERROR( "There no coordinate system collection." );

        const int COUNT_OF_ELEMENTS = 16;
        double datum_matrix[COUNT_OF_ELEMENTS];
        datum_matrix[0]  = matrix[0];
        datum_matrix[1]  = matrix[1];
        datum_matrix[2]  = matrix[2];
        datum_matrix[3]  = 0.0;
        datum_matrix[4]  = matrix[3];
        datum_matrix[5]  = matrix[4];
        datum_matrix[6]  = matrix[5];
        datum_matrix[7]  = 0.0;
        datum_matrix[8]  = matrix[6];
        datum_matrix[9]  = matrix[7];
        datum_matrix[10] = matrix[8];
        datum_matrix[11] = 0.0;
        datum_matrix[12] = 0.0;
        datum_matrix[13] = 0.0;
        datum_matrix[14] = 0.0;
        datum_matrix[15] = 1.0;

        SAFEARRAYBOUND sabDatumMatrix;
        sabDatumMatrix.lLbound = 0;
        sabDatumMatrix.cElements = COUNT_OF_ELEMENTS;
        SAFEARRAY* saDatumMatrix = SafeArrayCreate( VT_R8, 1, &sabDatumMatrix );

        if ( saDatumMatrix )
        {
            for ( long i = 0; i < COUNT_OF_ELEMENTS; i++ )
            {
                HRESULT hr = SafeArrayPutElement( saDatumMatrix, &i, &datum_matrix[i] );

                if ( !SUCCEEDED( hr ) )
                    QSCRIPT_ERROR( "Cannot create csys" );
            }
        }

        CoordinateSystemPtr pNewCsys = pCsysCollection->AddByMatrix( &saDatumMatrix );

        if ( !pNewCsys )
            QSCRIPT_ERROR( "Cannot create csys." );

        return GET_QSCRIPTVALUE_FROM_COM( pNewCsys );
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

QJSValue SolidEdgeExtensionImpl::getPlaneXOY( QJSValue csys )
{
    try
    {
        m_logger.info( "Getting plane XOY" );
        CoordinateSystemPtr pCoordinateSystem;
        GET_COM_FROM_QSCRIPTVALUE( pCoordinateSystem, csys, CoordinateSystem );

        if ( !pCoordinateSystem )
            QSCRIPT_ERROR( "Coordinate system doesn't exist." );

        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "There no document." );

        RefPlanePtr pXoyRefPlane =
            pPartDocument->RefPlanes->AddParallelByDistance( pCoordinateSystem->Plane[seCoordSysXYPlane], 0, igNormalSide );

        if ( !pXoyRefPlane )
            QSCRIPT_ERROR( "Cannon create ref plane." );

        return GET_QSCRIPTVALUE_FROM_COM( pXoyRefPlane );
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

QJSValue SolidEdgeExtensionImpl::getAxisOX( QJSValue csys )
{
    try
    {
        m_logger.info( "Getting axis OX" );

        // NOTE: We need to use RefAxis from PartDocument's RefPlanes collection
        // cause created in createBaseCsys() method csys does not contains any planes or axes
        PartDocumentPtr pPartDocument = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDocument )
            QSCRIPT_ERROR( "There no document." );

        return GET_QSCRIPTVALUE_FROM_COM( pPartDocument->RefAxes->Item( 1 ) );
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

QJSValue SolidEdgeExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point( QJSValue base_plane,
                                                                                           QJSValue base_axis,
                                                                                           QJSValue base_point )
{
    try
    {
        m_logger.info( "CONSTRAINTS_Create_3DReference_CoordSys_Plane_Axis_Point" );
        FacePtr pFace;
        EdgePtr pEdge;
        VertexPtr pVertex;

        if ( base_plane.isNull() || base_axis.isNull() || base_point.isNull() )
            QSCRIPT_ERROR( "Creating coordinate system by plane, axis, and point: wrong input data" );

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );
        BodyPtr body = model->GetBody();
        int faceId = base_plane.toInteger();
        int edgeId = base_axis.toInteger();
        int vertexId = base_point.toInteger();
        pFace = body->GetEntityByID( faceId );
        pEdge = body->GetEntityByID( edgeId );
        pVertex = body->GetEntityByID( vertexId );

        if ( !pFace || !pEdge || !pVertex )
            QSCRIPT_ERROR( "Creating coordinate system by plane, axis, and point: wrong input data" );

        SAFEARRAYBOUND sabVertexCoordinates;
        sabVertexCoordinates.lLbound = 0;
        sabVertexCoordinates.cElements = 3;
        SAFEARRAY* saVertexCoordinates = SafeArrayCreate( VT_R8, 1, &sabVertexCoordinates );

        if ( !saVertexCoordinates )
            QSCRIPT_ERROR( "Can not create safearray" );

        HRESULT hr = pVertex->GetPointData( &saVertexCoordinates );

        if ( !SUCCEEDED( hr ) )
            QSCRIPT_ERROR( "Can not get vertex coordinates" );

        double* coordinates = new double[3];

        for ( long i = 0; i < 3; i++ )
        {
            hr = SafeArrayGetElement( saVertexCoordinates, &i, &coordinates[i] );

            if ( !SUCCEEDED( hr ) )
                QSCRIPT_ERROR( "Can not get vertex coordinate" );
        }

        PartDocumentPtr pPartDoc = CCommands::GetApplicationPtr()->ActiveDocument;

        if ( !pPartDoc )
            QSCRIPT_ERROR( "There no document." );

        CoordinateSystemPtr pNewCsys =
            pPartDoc->CoordinateSystems->AddByGeometry( seCoordSysZAxis, pFace, igLeft,
                                                        seCoordSysXAxis, pVertex, igLeft,
                                                        pEdge, igKeyPointStart );

        if ( !pNewCsys )
            QSCRIPT_ERROR( "Creating coordinate system by plane, axis, and point: can not create csys" );

        return GET_QSCRIPTVALUE_FROM_COM( pNewCsys );
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

QJSValue SolidEdgeExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir( QJSValue point,
                                                                                       QJSValue x_dir,
                                                                                       QJSValue y_dir )
{
    m_logger.info( "CONSTRAINTS_Create_3DReference_CoordSys_Point_XY_dir" );
    return QJSValue();
}

QJSValue SolidEdgeExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir( QJSValue point,
                                                                                       QJSValue y_dir,
                                                                                       QJSValue z_dir )
{
    m_logger.info( "CONSTRAINTS_Create_3DReference_CoordSys_Point_YZ_dir" );
    return QJSValue();
}

QJSValue SolidEdgeExtensionImpl::CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir( QJSValue point,
                                                                                       QJSValue x_dir,
                                                                                       QJSValue z_dir )
{
    m_logger.info( "CONSTRAINTS_Create_3DReference_CoordSys_Point_XZ_dir" );
    return QJSValue();
}

QJSValue SolidEdgeExtensionImpl::CONSTRAINTS_Create_Base_CoorSys( QVariantList matr )
{
    m_logger.info( "CONSTRAINTS_Create_Base_CoorSys" );
    CheckPostprocess();

    double matrix[9];

    for ( int i = 0; i < 9; i++ )
        matrix[i] = matr.at( i ).toDouble();

    return createBaseCsys( matrix );
}
