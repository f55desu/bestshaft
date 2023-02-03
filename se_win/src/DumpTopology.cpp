#include "stdafx.h"
#include "SolidEdgeExtensionImpl.h"
#include "commands.h"
#include "Utils.h"

void SolidEdgeExtensionImpl::DumpTopology()
{
    QFileDialog* fileDialog = new QFileDialog( 0x0, Qt::WindowSystemMenuHint | Qt::WindowTitleHint );
    fileDialog->setFileMode( QFileDialog::AnyFile );
    fileDialog->setAcceptMode( QFileDialog::AcceptSave );
    fileDialog->setOption( QFileDialog::DontUseNativeDialog, true );
    fileDialog->setDefaultSuffix( "txt" );
    fileDialog->setNameFilter( "*.txt" );
    fileDialog->setViewMode( QFileDialog::List );
    QDialog::DialogCode dialogResult = ( QDialog::DialogCode )fileDialog->exec();

    if ( dialogResult != QDialog::Accepted )
    {
        delete fileDialog;
        return;
    }

    QStringList selectedFiles = fileDialog->selectedFiles();
    delete fileDialog;
    QString outputFileName = selectedFiles.at( 0 );
    m_logger.info( "Writing topology dump to %s", outputFileName.toStdString().c_str() );
    QFile outputFile( outputFileName );

    if ( !outputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QSHOW_ERROR( "Can not open output file" );
        return;
    }

    QTextStream outStream( &outputFile );

    try
    {
        if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QSHOW_ERROR( "There no document or active document is not part document" );
            return;
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;

        for ( int modelNumber = 1; modelNumber <= modelsCollection->Count; modelNumber++ )
        {
            ModelPtr model = modelsCollection->Item( modelNumber );
            BodyPtr body = model->GetBody();
            outStream << "Body tag = " << body->GetTag() << "\n";

            FacesPtr facesCollection = body->GetFaces( igQueryAll );

            for ( int faceNumber = 1; faceNumber <= facesCollection->Count; faceNumber++ )
            {
                FacePtr face = facesCollection->Item( faceNumber );
                int faceId = face->GetID();
                outStream << "\tFace name = " << m_topologyObjectsNames.value( faceId )
                          << ", id = " << faceId
                          << ", x = " << m_topologyObjectCoordinates.value( faceId ).x()
                          << ", y = " << m_topologyObjectCoordinates.value( faceId ).y()
                          << ", z = " << m_topologyObjectCoordinates.value( faceId ).z()
                          << "\n";

                LoopsPtr loopsCollection = face->GetLoops();

                for ( int loopNumber = 1; loopNumber <= loopsCollection->Count; loopNumber++ )
                {
                    LoopPtr loop = loopsCollection->Item( loopNumber );

                    EdgesPtr edgesCollection = loop->GetEdges();

                    for ( int edgeNumber = 1; edgeNumber <= edgesCollection->Count; edgeNumber++ )
                    {
                        EdgePtr edge = edgesCollection->Item( edgeNumber );

                        if ( edge )
                        {
                            int edgeId = edge->GetID();
                            outStream << "\t\t\tEdge name = " << m_topologyObjectsNames.value( edgeId )
                                      << ", id = " << edgeId
                                      << ", x = " << m_topologyObjectCoordinates.value( edgeId ).x()
                                      << ", y = " << m_topologyObjectCoordinates.value( edgeId ).y()
                                      << ", z = " << m_topologyObjectCoordinates.value( edgeId ).z() << "\n";

                            VertexPtr startVertex = edge->GetStartVertex();

                            if ( startVertex != 0x0 )
                            {
                                int startVertexId = startVertex->GetID();
                                outStream << "\t\t\t\tVertex name = " << m_topologyObjectsNames.value( startVertexId )
                                          << ", id = " << startVertexId
                                          << ", x = " << m_topologyObjectCoordinates.value( startVertexId ).x()
                                          << ", y = " << m_topologyObjectCoordinates.value( startVertexId ).y()
                                          << ", z = " << m_topologyObjectCoordinates.value( startVertexId ).z() << "\n";
                            }

                            VertexPtr endVertex = edge->GetEndVertex();

                            if ( endVertex != 0x0 )
                            {
                                int endVertexId = endVertex->GetID();
                                outStream << "\t\t\t\tVertex name = " << m_topologyObjectsNames.value( endVertexId )
                                          << ", id = " << endVertexId
                                          << ", x = " << m_topologyObjectCoordinates.value( endVertexId ).x()
                                          << ", y = " << m_topologyObjectCoordinates.value( endVertexId ).y()
                                          << ", z = " << m_topologyObjectCoordinates.value( endVertexId ).z() << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    catch ( _com_error exception )
    {
        QSHOW_ERROR( QString( "COM error in extension: %s" )
                     .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
    }
    catch ( ... )
    {
        QSHOW_ERROR( "Unknown error in extension" );
    }

    outStream.flush();
    outputFile.close();
}

void SolidEdgeExtensionImpl::DumpTopologyToObj()
{
    QFileDialog* fileDialog = new QFileDialog( 0x0, Qt::WindowSystemMenuHint | Qt::WindowTitleHint );
    fileDialog->setFileMode( QFileDialog::AnyFile );
    fileDialog->setAcceptMode( QFileDialog::AcceptSave );
    fileDialog->setOption( QFileDialog::DontUseNativeDialog, true );
    fileDialog->setDefaultSuffix( "obj" );
    fileDialog->setNameFilter( "*.obj" );
    fileDialog->setViewMode( QFileDialog::List );
    QDialog::DialogCode dialogResult = ( QDialog::DialogCode )fileDialog->exec();

    if ( dialogResult != QDialog::Accepted )
    {
        delete fileDialog;
        return;
    }

    QStringList selectedFiles = fileDialog->selectedFiles();
    delete fileDialog;
    QString outputFileName = selectedFiles.at( 0 );
    m_logger.info( "Writing topology dump to %s", outputFileName.toStdString().c_str() );
    QFile outputFile( outputFileName );

    if ( !outputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QSHOW_ERROR( "Can not open output file" );
        return;
    }

    QTextStream objFileStream( &outputFile );
    QStringList facets;

    try
    {
        if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QSHOW_ERROR( "There no document or active document is not part document" );
            return;
        }

        QList<QVector3D> facetVertices;

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;

        for ( int modelNumber = 1; modelNumber <= modelsCollection->Count; modelNumber++ )
        {
            ModelPtr model = modelsCollection->Item( modelNumber );
            BodyPtr body = model->GetBody();

            FacesPtr facesCollection = body->GetFaces( igQueryAll );

            for ( int faceNumber = 1; faceNumber <= facesCollection->Count; faceNumber++ )
            {
                FacePtr face = facesCollection->Item( faceNumber );
                facets.append( QString( "g %1\n" ).arg( face->GetID() ) );

                long facetsCount = 0;
                SAFEARRAY* saFacetCoordinates = NULL;

                HRESULT hr = face->GetFacetData( m_tolerance, &facetsCount, &saFacetCoordinates );

                if ( SUCCEEDED( hr ) )
                {
                    for ( int facetNumber = 0; facetNumber < facetsCount; facetNumber++ )
                    {
                        QString facet = "f ";

                        for ( long pointNumber = 0; pointNumber < 3; pointNumber++ )
                        {
                            double points[3] = {0.0};

                            for ( long coordinateNumber = 0; coordinateNumber < 3; coordinateNumber++ )
                            {
                                long coordinateIndex = facetNumber * 9 + pointNumber * 3 + coordinateNumber;
                                hr = SafeArrayGetElement( saFacetCoordinates, &coordinateIndex, &points[coordinateNumber] );

                                if ( !SUCCEEDED( hr ) )
                                    break;
                            }

                            QVector3D facetVertex( points[0], points[1], points[2] );
                            int i = 0;

                            for ( i; i < facetVertices.count(); i++ )
                            {
                                if ( facetVertices[i] == facetVertex )
                                    break;
                            }

                            if ( i == facetVertices.count() ) // vertex not found, add new
                            {
                                facetVertices.append( facetVertex );
                                objFileStream << QString( "v %1 %2 %3\n" ).arg( facetVertex.x() ).arg( facetVertex.y() ).arg( facetVertex.z() );
                                objFileStream.flush();
                            }

                            facet.append( QString( "%1 " ).arg( i + 1 ) ); // in .obj numbering starts with 1 instead of 0
                        }

                        facet.append( "\n" );
                        facets.append( facet );
                    }
                }
            }
        }

    }
    catch ( _com_error exception )
    {
        QSHOW_ERROR( QString( "COM error in extension: %s" )
                     .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ).toStdString().c_str() );
    }
    catch ( ... )
    {
        QSHOW_ERROR( "Unknown error in extension" );
    }

    foreach ( QString facet, facets )
    {
        objFileStream << facet;
        objFileStream.flush();
    }

    outputFile.close();
}

void SolidEdgeExtensionImpl::DumpTangents()
{
//#define USE_EDGE_USES
    try
    {
        if ( CCommands::GetApplicationPtr()->ActiveDocument == 0x0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QSHOW_ERROR( "There no document or active document is not part document" );
            return;
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );
        BodyPtr body = model->GetBody();

        SAFEARRAYBOUND sabCoordinates;
        sabCoordinates.lLbound = 0;
        sabCoordinates.cElements = 3;

        SAFEARRAYBOUND sabParams;
        sabParams.lLbound = 0;
        sabParams.cElements = 1;

        FacesPtr faces = body->GetFaces( igQueryAll );

        for ( int i = 1; i <= faces->Count; i++ )
        {
            FacePtr face = faces->Item( i );
            LoopsPtr loops = face->Loops;

            for ( int j = 1; j <= loops->Count; j++ )
            {
                LoopPtr loop = loops->Item( j );
                VerticesPtr loopVertices = loop->Vertices;
                QFile loopOrderFile( QString( "D:\\loop%1_vertises_order.txt" ).arg( loop->GetID() ) );
                loopOrderFile.open( QFile::WriteOnly | QFile::Text );
                QTextStream ts_l( &loopOrderFile );

                for ( int k = 1; k <= loopVertices->Count; k++ )
                {
                    VertexPtr vertex = loopVertices->Item( k );
                    QVector3D vertexCoordinates;
                    SAFEARRAY* saCoordinates = NULL;
                    vertex->GetPointData( &saCoordinates );
                    Utils::getCoordinatesFromSafeArray( saCoordinates, vertexCoordinates );

                    ts_l << QString( "v %1 %2 %3" ).arg( vertexCoordinates.x() ).arg( vertexCoordinates.y() ).arg(
                             vertexCoordinates.z() ) << endl;
                    ts_l.flush();
                }

                loopOrderFile.close();

#ifdef USE_EDGE_USES
                EdgeUsesPtr edges = loop->EdgeUses;
#else
                EdgesPtr edges = loop->Edges;
#endif
                QFile edgeOrderFile( QString( "D:\\loop%1_edges_vertises_order.txt" ).arg( loop->GetID() ) );
                edgeOrderFile.open( QFile::WriteOnly | QFile::Text );
                QTextStream ts_o( &edgeOrderFile );

                for ( int k = 1; k <= edges->Count; k++ )
                {
#ifdef USE_EDGE_USES
                    EdgeUsePtr edgeUse = edges->Item( k );
                    EdgePtr edge = edgeUse->Edge;
#else
                    EdgePtr edge = edges->Item( k );
#endif
                    VertexPtr startVertex = edge->GetStartVertex();
                    VertexPtr endVertex = edge->GetEndVertex();

                    SAFEARRAY* saStartPoint = SafeArrayCreate( VT_R8, 1, &sabCoordinates );
                    SAFEARRAY* saEndPoint = SafeArrayCreate( VT_R8, 1, &sabCoordinates );;
                    edge->GetEndPoints( &saStartPoint, &saEndPoint );

                    QVector3D startPoint, endPoint;
                    Utils::getCoordinatesFromSafeArray( saStartPoint, startPoint );
                    Utils::getCoordinatesFromSafeArray( saEndPoint, endPoint );

                    ts_o << QString( "g face%1edge%2" ).arg( face->GetID() ).arg( edge->GetID() ) << endl;
                    ts_o << QString( "v %1 %2 %3" ).arg( startPoint.x() ).arg( startPoint.y() ).arg( startPoint.z() ) << endl;
                    ts_o << QString( "v %1 %2 %3" ).arg( endPoint.x() ).arg( endPoint.y() ).arg( endPoint.z() ) << endl;

                    SAFEARRAY* saStartParam = SafeArrayCreate( VT_R8, 1, &sabParams );
                    SAFEARRAY* saEndParam = SafeArrayCreate( VT_R8, 1, &sabParams );
                    SAFEARRAY* saGuessParam = NULL;
                    SAFEARRAY* saMaxDeviations = NULL;
                    SAFEARRAY* saFlags = NULL;
                    edge->GetParamAtPoint( 1, &saStartPoint, &saGuessParam, &saMaxDeviations, &saStartParam, &saFlags );
                    edge->GetParamAtPoint( 1, &saEndPoint, &saGuessParam, &saMaxDeviations, &saEndParam, &saFlags );

                    SAFEARRAY* saStartTangent = SafeArrayCreate( VT_R8, 1, &sabCoordinates );
                    SAFEARRAY* saEndTangent = SafeArrayCreate( VT_R8, 1, &sabCoordinates );
#ifdef USE_EDGE_USES
                    edge->GetTangent( 1, &saStartParam, &saStartTangent );
                    edge->GetTangent( 1, &saEndParam, &saEndTangent );
#else
                    edge->GetTangent( 1, &saStartParam, &saStartTangent );
                    edge->GetTangent( 1, &saEndParam, &saEndTangent );
#endif

                    QVector3D startTangentVector, endTangentVector;
                    Utils::getCoordinatesFromSafeArray( saStartTangent, startTangentVector );
                    Utils::getCoordinatesFromSafeArray( saEndTangent, endTangentVector );
#ifdef USE_EDGE_USES

//          if (edgeUse->IsParamReversed == VARIANT_TRUE)
                    if ( edgeUse->IsOpposedToEdge == VARIANT_TRUE )
                    {
                        startTangentVector *= -1;
                        endTangentVector *= -1;
                    }

#else

                    if ( edge->IsParamReversed == VARIANT_TRUE )
                    {
                        startTangentVector *= -1;
                        endTangentVector *= -1;
                    }

#endif

                    QVector3D edgeDirectionVector = endPoint - startPoint;
                    edgeDirectionVector *= 1.5;
                }

                edgeOrderFile.close();
            }
        }
    }
    catch ( const _com_error& exception )
    {
        m_logger.error( qPrintable( QString( "COM error in extension: %1" )
                                    .arg( QString::fromLocal8Bit( exception.ErrorMessage() ) ) ) );
        return;
    }
    catch ( ... )
    {
        m_logger.error( "Filling hash of primitives coordinates: unknown exception" );
        return;
    }
}
