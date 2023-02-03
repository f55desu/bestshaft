#include "../stdafx.h"
#include "../SolidEdgeExtensionImpl.h"
#include "../commands.h"
#include "../util.h"

QString SolidEdgeExtensionImpl::PreprocessModel()
{
    m_logger.info( "Preprocessing model" );

    try
    {
        if ( CCommands::GetApplicationPtr()->Documents->Count == 0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QString errorMessage = "There no document or current document is not part document";
            m_logger.error( errorMessage.toStdString() );
            return errorMessage;
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        ModelsPtr modelsCollection = partDocument->Models;
        ModelPtr model = modelsCollection->Item( 1 );

        if ( !model )
        {
            QString errorMessage = "There no models in document";
            m_logger.error( errorMessage.toStdString() );
            return errorMessage;
        }

        BodyPtr body = model->GetBody();

        if ( !body )
        {
            QString errorMessage = "There no body in model to proceed";
            m_logger.error( errorMessage.toStdString() );
            return errorMessage;
        }

        name_csys.clear();
        name_edge.clear();
        name_face.clear();
        name_point.clear();
        m_countFeatureSketch = 0;
        m_countNewCsys = 0;
        count_edge = 0;
        count_face = 0;
        count_point_on_edge = 0;
        //isSoDirection = true;
        allFeature.clear();
        numberCurrentFeature = 0;

        m_content.clear();
        visitedFeaturesSet.clear();
        visitedSmartObjectsSet.clear();
        m_indent = "   ";

        //m_content += "with (Modeler)\n{\n";

        /*SketchsPtr sketches = partDocument->Sketches;
        for (int i = 1; i <= sketches->Count; i++)
        {
          preprocessSketch(sketches->Item(i));
        }*/

        FeaturesPtr features = model->GetFeatures();

        for ( long i = 1; i <= features->Count; i++ )
            allFeature.push_back( features->Item( i ) );

        for ( long i = 1; i <= features->Count; i++, numberCurrentFeature++ )
        {
            IDispatchPtr feature = features->Item( i );
            bool supressed = false;

            if ( SUCCEEDED( ComGetProperty( feature, L"Suppress", VT_I4, ( void* )&supressed ) ) )
            {
                if ( supressed )
                    continue;

                TraverseFeatureRelatives( feature );
            }
        }

//    UF_CALL(::UF_MODL_delete_list(&featureList));
        //m_content += "}";

        m_logger.info( "End preprocessing." );

        return m_content;
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = "COM error in the extension: " + QString::fromLocal8Bit( e.ErrorMessage() );
        m_logger.error( errorMessage.toStdString() );
        return errorMessage;
    }
    catch ( ... )
    {
        QString errorMessage = "Unknown exeption in the extension.";
        m_logger.error( errorMessage.toStdString() );
        return errorMessage;
    }

    return m_content;
}

void SolidEdgeExtensionImpl::TraverseFeatureRelatives( const IDispatchPtr& feature )
{
    try
    {
        if ( visitedFeaturesSet.find( feature ) == visitedFeaturesSet.end() )
        {
            FeatureTypeConstants featType;

            if ( SUCCEEDED( ComGetProperty( feature, L"Type", VT_I4, ( void* )&featType ) ) )
            {
                if ( featType == igExtrudedProtrusionFeatureObject )
                    preprocessProtrusionExtrude_Extrude( feature );
                else
                    m_logger.warn( "Unknown feature type" );
            }
            else
                m_logger.warn( "Can not get feature type" );
        }
        else
            visitedFeaturesSet.insert( feature );
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = "COM error in the extension: " + QString::fromLocal8Bit( e.ErrorMessage() );
        m_logger.error( errorMessage.toStdString() );
        return;
    }
    catch ( ... )
    {
        QString errorMessage = "Unknown exeption in the extension.";
        m_logger.error( errorMessage.toStdString() );
        return;
    }
}

void SolidEdgeExtensionImpl::sketch( const ProfilePtr& profile )
{
    try
    {
        SAFEARRAY* matrix = NULL;
        HRESULT hr = profile->GetMatrix( &matrix );
        Q_ASSERT( SUCCEEDED( hr ) );

        m_content += m_indent + QString( "var matrix = [1, 0, 0, 0, 1, 0, 0, 0, 1];\n" );
        long elementsCount = 0;
        ::SafeArrayGetUBound( matrix, 1, &elementsCount );

        for ( long i = 0; i < elementsCount; i++ )
        {
            double element;
            ::SafeArrayGetElement( matrix, &i, &element );
        }

        ::SafeArrayDestroy( matrix );

        m_content += m_indent + QString( "var sketch%1 = SKETCH_Open(\"%2\", %3);\n" )
                     .arg( ++m_countFeatureSketch ).arg( "name" ).arg( name_csys );

        Lines2dPtr lines = profile->GetLines2d();

        for ( long i = 1; i <= lines->Count; i++ )
        {
            Line2dPtr line = lines->Item( i );
            QString lineName = GET_QSTRING_FROM_BSTR( line->GetName( false ) );
            double startPointX, startPointY, endPointX, endPointY;
            hr = line->GetStartPoint( &startPointX, &startPointY );
            Q_ASSERT( SUCCEEDED( hr ) );
            hr = line->GetEndPoint( &endPointX, &endPointY );
            Q_ASSERT( SUCCEEDED( hr ) );

            m_content += m_indent + QString( "SKETCH_Create_2D_Line_2Points(sketch%1, \"%2\", %3, %4, %5, %6);\n" )
                         .arg( m_countFeatureSketch ).arg( lineName ).arg( startPointX ).arg( startPointY ).arg( endPointX ).arg( endPointY );
        }

        m_content += m_indent + QString( "SKETCH_Close(sketch%1)\n" ).arg( m_countFeatureSketch );
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = "COM error in the extension: " + QString::fromLocal8Bit( e.ErrorMessage() );
        m_logger.error( errorMessage.toStdString() );
        return;
    }
    catch ( ... )
    {
        QString errorMessage = "Unknown exeption in the extension.";
        m_logger.error( errorMessage.toStdString() );
        return;
    }
}

void SolidEdgeExtensionImpl::preprocessProtrusionExtrude_Extrude( const IDispatchPtr& feature )
{
    try
    {
        ExtrudedProtrusionPtr protrusion = ( ExtrudedProtrusionPtr )feature;

        long numProfiles = 0;
        SAFEARRAY* profiles = NULL;
        HRESULT hr = protrusion->GetProfiles( &numProfiles, &profiles );
        Q_ASSERT( SUCCEEDED( hr ) );

        for ( long i = 0; i < numProfiles; i++ )
        {
            ProfilePtr profile;
            ::SafeArrayGetElement( profiles, &i, &profile );
            sketch( profile );
        }

        ::SafeArrayDestroy( profiles );

        QString name = GET_QSTRING_FROM_BSTR( protrusion->GetName() );
        int indexOf = -1;

        if ( ( indexOf = name.indexOf( "(" ) ) != -1 )
            name.remove( indexOf, 1 );

        if ( ( indexOf = name.indexOf( ")" ) ) != -1 )
            name.remove( indexOf, 1 );

        m_content += m_indent + QString( "SOLID_Create_Protrusion_Extrude(sketch%2, \"%1\", %3, %4);\n" )
                     .arg( name )
                     .arg( m_countFeatureSketch )
                     .arg( 0 )
                     .arg( protrusion->GetDepth() );
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = "COM error in the extension: " + QString::fromLocal8Bit( e.ErrorMessage() );
        m_logger.error( errorMessage.toStdString() );
        return;
    }
    catch ( ... )
    {
        QString errorMessage = "Unknown exeption in the extension.";
        m_logger.error( errorMessage.toStdString() );
        return;
    }
}

void dumpBoundary( const Boundary2dPtr& boundary, QTextStream& ts )
{
    ts << "          Boundary \"" << GET_QSTRING_FROM_BSTR( boundary->GetName( false ) )
       << "\" Type: " << boundary->GetType() << "\n";

    double x = 0.0, y = 0.0;
    boundary->GetCentroid( &x, &y );
    ts << "            Centroid coordinates: " << x << ", " << y << "\n";

    boundary->GetFloodPoint( &x, &y );
    ts << "            Flood point coordinates: " << x << ", " << y << "\n";

    long keyPointsCount = boundary->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        boundary->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i + 1 << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    ts << "            Key: " << GET_QSTRING_FROM_BSTR( boundary->Key[false] ) << "\n";
    ts << "            State: " << boundary->State << "\n";
}

void dumpArc( const Arc2dPtr& arc, QTextStream& ts )
{
    ts << "          Arc \"" << GET_QSTRING_FROM_BSTR( arc->GetName( false ) ) << "\"\n";

    double x = 0.0, y = 0.0;
    arc->GetStartPoint( &x, &y );
    ts << "            Start point coordinates: " << x << ", " << y << "\n";
    arc->GetEndPoint( &x, &y );
    ts << "            End point coordinates: " << x << ", " << y << "\n";
    arc->GetCenterPoint( &x, &y );
    ts << "            Center point coordinates: " << x << ", " << y << "\n";
    ts << "            Lenght: " << arc->Length << "\n"
       << "            Radius: " << arc->Radius << "\n"
       << "            Start angle: " << arc->StartAngle << "\n"
       << "            Sweep angle: " << arc->SweepAngle << "\n"
       << "            Key: " << GET_QSTRING_FROM_BSTR( arc->Key ) << "\n"
       << "            Is fillet: " << ( ( arc->IsFillet == VARIANT_TRUE ) ? "true\n" : "false\n" );

    long keyPointsCount = arc->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        arc->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i + 1 << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    // TODO: dump relationships
}

void dumpBSplineCurve( const BSplineCurve2dPtr& curve, QTextStream& ts )
{
    ts << "          BSpline curve \"" << GET_QSTRING_FROM_BSTR( curve->GetName( false ) ) << "\"\n";

    double x = 0.0, y = 0.0;
    curve->GetCentroid( &x, &y );
    ts << "            Centroid coordinates: " << x << ", " << y << "\n";

    VARIANT nodesNumber, polesNumber, form, scope, rational, degree;
    curve->GetData( &nodesNumber, &vtMissing, &polesNumber, &vtMissing, &vtMissing, &rational, &vtMissing, &degree, &form,
                    &scope );

    /*ts << "            Number of nodes: " << V_INT(nodesNumber) << "\n"
       << "            Number of poles: " << V_INT(polesNumber) << "\n"
       << "            Rational: " << ((V_BOOL(rational) == VARIANT_TRUE) ? "true\n" : "false\n")
       << "            Degree: " << V_R8(degree) << "\n"
       << "            Form: " << V_INT(form) << "\n"
       << "            Scope: " << V_INT(scope) << "\n"
       << "            Key: " << GET_QSTRING_FROM_BSTR(curve->Key) << "\n";*/

    long keyPointsCount = curve->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        curve->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i + 1 << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    // TODO: dump relationships
}

void dumpCircle( const Circle2dPtr& circle, QTextStream& ts )
{
    ts << "          Circle \"" << GET_QSTRING_FROM_BSTR( circle->GetName( false ) ) << "\"\n";

    double x = 0.0, y = 0.0;
    circle->GetCenterPoint( &x, &y );
    ts << "            Center point coordinates: " << x << ", " << y << "\n";

    ts << "            Area: " << circle->Area << "\n"
       << "            Circumference: " << circle->Circumference << "\n"
       << "            Diameter: " << circle->Diameter << "\n"
       << "            Length: " << circle->Length << "\n"
       << "            Radius: " << circle->Radius << "\n"
       << "            Key: " << GET_QSTRING_FROM_BSTR( circle->Key ) << "\n";

    long keyPointsCount = circle->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        circle->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i + 1 << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    // TODO: dump relationships
}

void dumpLine( const Line2dPtr& line, QTextStream& ts )
{
    ts << "          Line \"" << GET_QSTRING_FROM_BSTR( line->GetName( false ) ) << "\"\n";

    double x = 0.0, y = 0.0;
    line->GetStartPoint( &x, &y );
    ts << "            Start point coordinates: " << x << ", " << y << "\n";
    line->GetEndPoint( &x, &y );
    ts << "            End point coordinates: " << x << ", " << y << "\n";

    ts << "            Angle: " << line->Angle << "\n"
       << "            Is chamfer: " << ( ( line->IsChamfer == VARIANT_TRUE ) ? "true\n" : "false\n" )
       << "            Length: " << line->Length << "\n"
       << "            Is projection: " << ( ( line->Projection == VARIANT_TRUE ) ? "true\n" : "false\n" );

    if ( line->Projection == VARIANT_TRUE )
        ts << "            Projection dash name: " << GET_QSTRING_FROM_BSTR( line->ProjectionDashName ) << "\n";

    ts << "            Key: " << GET_QSTRING_FROM_BSTR( line->Key ) << "\n";

    long keyPointsCount = line->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        line->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i + 1 << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    // TODO: dump relationships
}

void dumpPoint( const Point2dPtr& point, QTextStream& ts )
{
    ts << "          Point \"" << GET_QSTRING_FROM_BSTR( point->GetName( false ) ) << "\"\n";

    double x = 0.0, y = 0.0;
    point->GetPoint( &x, &y );
    ts << "            Point coordinates: " << x << ", " << y << "\n"
       << "            Key: " << GET_QSTRING_FROM_BSTR( point->Key ) << "\n";

    long keyPointsCount = point->GetKeyPointCount();
    ts << "            Key points count: " << keyPointsCount << "\n";

    for ( long i = 0; i < keyPointsCount; i++ )
    {
        double keyX = 0.0, keyY = 0.0, keyZ = 0.0;
        KeyPointType keyType = igKeyPointNonDefining;
        long handleType = ( long )igHandleNone;

        point->GetKeyPoint( i, &keyX, &keyY, &keyZ, &keyType, &handleType );
        ts << "              Key point number " << i << " coordinates: " << keyX << ", " << keyY << ", " << keyZ
           << " key type: " << keyType << " handle type: " << handleType << "\n";
    }

    // TODO: dump relationships
}

void dumpProfile( const ProfilePtr& profile, QTextStream& ts )
{
    ts << "      Profile: name \""
       << GET_QSTRING_FROM_BSTR( profile->GetName() )
       << "\" DisplayName: \"" << GET_QSTRING_FROM_BSTR( profile->DisplayName )
       << "\" SystemName: \"" << GET_QSTRING_FROM_BSTR( profile->SystemName )
       << "\"\n";

    RefPlanePtr plane = profile->Plane;
    ts << "        RefPlane: \"" << GET_QSTRING_FROM_BSTR( plane->GetName() ) << "\"\n";
    SAFEARRAY* saNormal = 0x0;
    plane->GetNormal( &saNormal );
    SAFEARRAY* saRootPoint = 0x0;
    plane->GetRootPoint( &saRootPoint );

    double* normal = new double[3];
    double* rootPoint = new double[3];

    for ( long l = 0; l < 3; l++ )
    {
        ::SafeArrayGetElement( saNormal, &l, &normal[l] );
        ::SafeArrayGetElement( saRootPoint, &l, &rootPoint[l] );
    }

    ts << QString( "          Normal: (%1; %2; %3)" ).arg( normal[0] ).arg( normal[1] ).arg( normal[2] ) << "\n";
    ts << QString( "          Root point: (%1; %2; %3)" ).arg( rootPoint[0] ).arg( rootPoint[1] ).arg(
           rootPoint[2] ) << "\n";
    ts.flush();

    delete[] normal;
    delete[] rootPoint;
    ::SafeArrayDestroy( saNormal );
    ::SafeArrayDestroy( saRootPoint );

    Arcs2dPtr arcs = profile->Arcs2d;
    ts << "        Arcs count: " << arcs->Count << "\n";

    for ( long i = 1; i <= arcs->Count; i++ )
    {
        Arc2dPtr arc = arcs->Item( i );
        dumpArc( arc, ts );
    }

    AreaPropertiesCollectionPtr areaProperties = profile->AreaPropertiesCollection;
    ts << "        Area Properties count: " << areaProperties->Count << "\n";
    // TODO: dump area properties

    Boundaries2dPtr boundaries = profile->GetBoundaries2d();
    ts << "        Boundaries count: " << boundaries->Count << "\n";

    for ( long i = 1; i <= boundaries->Count; i++ )
    {
        Boundary2dPtr boundary = boundaries->Item( i );
        dumpBoundary( boundary, ts );
    }

    BalloonsPtr balloons = profile->Balloons;
    ts << "        Balloons count: " << balloons->Count << "\n";
    // TODO: dump balloons

    BSplineCurves2dPtr curves = profile->BSplineCurves2d;
    ts << "        BSpline curves count: " << curves->Count << "\n";

    for ( long i = 1; i <= curves->Count; i++ )
    {
        BSplineCurve2dPtr curve = curves->Item( i );
        dumpBSplineCurve( curve, ts );
    }

    Circles2dPtr circles = profile->Circles2d;
    ts << "        Circles count: " << curves->Count << "\n";

    for ( long i = 1; i <= circles->Count; i++ )
    {
        Circle2dPtr circle = circles->Item( i );
        dumpCircle( circle, ts );
    }

    CircularPatterns2dPtr circularPatterns = profile->CircularPatterns2d;
    ts << "        Circular patterns count: " << circularPatterns->Count << "\n";

    for ( long i = 1; i <= circularPatterns->Count; i++ )
    {
        CircularPattern2dPtr circularPattern = circularPatterns->Item( i );
        ts << "        Circular pattern:\n"
           << "          Angular spacing: " << circularPattern->AngularSpacing << "\n"
           << "          Count: " << circularPattern->Count << "\n"
           << "          Reference occurence: " << circularPattern->ReferenceOccurrence << "\n";
    }

    /*ComponentImages2dPtr componentImages = profile->ComponentImages2d;
    ts << "        Component images count: " << componentImages->Count << "\n";*/ //Returns 0x0, there nio description in documentation
    // TODO: dump component images

    CornerAnnotationsPtr cornerAnnotations = profile->CornerAnnotations;
    ts << "        Corner annotations count: " << cornerAnnotations->Count << "\n";
    // TODO: dump corner annotations

    DatumFramesPtr datumFrames = profile->DatumFrames;
    ts << "        Datum frames count: " << datumFrames->Count << "\n";
    // TODO: dump datum frames

    DatumPointsPtr datumPoints = profile->DatumPoints;
    ts << "        Datum points count: " << datumPoints->Count << "\n";
    // TODO: dump datum points

    DatumTargetsPtr datumTargets = profile->DatumTargets;
    ts << "        Datum targets count: " << datumTargets->Count << "\n";
    // TODO: dump datum targets

    DimensionsPtr dimensions = profile->Dimensions;
    ts << "        Dimensions count: " << dimensions->Count << "\n";
    // TODO: dump dimensions

    DrawingObjectsPtr drawingObjects = profile->DrawingObjects;
    ts << "        Drawing objects count: " << drawingObjects->Count << "\n";
    // TODO: dump drawing objects

    Ellipses2dPtr ellipses = profile->Ellipses2d;
    ts << "        Ellipses images count: " << ellipses->Count << "\n";
    // TODO: dump ellipses

    EllipticalArcs2dPtr ellipticalArcs = profile->EllipticalArcs2d;
    ts << "        Elliptical arcs count: " << ellipticalArcs->Count << "\n";
    // TODO: dump elliptical arcs

    FeatureControlFramesPtr featureControlFrames = profile->FeatureControlFrames;
    ts << "        Feature control frames count: " << featureControlFrames->Count << "\n";
    // TODO: dump feature control frames

    GostWeldSymbolsPtr gostWeldSymbols = profile->GostWeldSymbols;
    ts << "        Gost weld symbols count: " << gostWeldSymbols->Count << "\n";
    // TODO: dump gost weld symbols

    Holes2dPtr holes = profile->Holes2d;
    ts << "        Holes count: " << holes->Count << "\n";
    // TODO: dump holes

    Images2dPtr images = profile->Images2d;
    ts << "        Images count: " << images->Count << "\n";
    // TODO: dump images

    LeadersPtr leaders = profile->Leaders;
    ts << "        Leaders count: " << leaders->Count << "\n";
    // TODO: dump leaders

    Lines2dPtr lines = profile->Lines2d;
    ts << "        Lines count: " << lines->Count << "\n";

    for ( long i = 1; i <= lines->Count; i++ )
    {
        Line2dPtr line = lines->Item( i );
        dumpLine( line, ts );
    }

    MountingBoss2dCollectionPtr mountingBossCollection = profile->MountingBoss2dCollection;
    ts << "        Mounting boss count: " << mountingBossCollection->Count << "\n";
    // TODO: dump mounting boss collection

    Points2dPtr points = profile->Points2d;
    ts << "        Points count: " << points->Count << "\n";

    for ( long i = 1; i <= points->Count; i++ )
    {
        Point2dPtr point = points->Item( i );
        dumpPoint( point, ts );
    }

    RectangularPatterns2dPtr rectangularPatterns = profile->RectangularPatterns2d;
    ts << "        Rectangular patterns count: " << rectangularPatterns->Count << "\n";
    // TODO: dump rectangular patterns

    Relations2dPtr relations = profile->Relations2d;
    ts << "        Relations count: " << relations->Count << "\n";
    // TODO: dump relations

    SurfaceFinishSymbolsPtr surfaceFinishSymbols = profile->SurfaceFinishSymbols;
    ts << "        Surface finish symbols count: " << surfaceFinishSymbols->Count << "\n";
    // TODO: dump Surface finish symbols

    /*TextProfilesPtr textProfiles = profile->TextProfiles;
    ts << "        Text profiles count: " << textProfiles->Count << "\n";*/
    // TODO: dump text profiles

    WeldSymbolsPtr weldSymbols = profile->WeldSymbols;
    ts << "        Weld symbols count: " << weldSymbols->Count << "\n";
    // TODO: dump weld symbols

    IDispatchPtr parent = profile->Parent;
    long type;
    ComGetProperty( parent, L"Type", VT_I4, ( void* )&type );
    ts << "        Profile parent type: " << type << "\n";
}

void SolidEdgeExtensionImpl::dumpAll()
{
    try
    {
        if ( CCommands::GetApplicationPtr()->Documents->Count == 0 ||
                CCommands::GetApplicationPtr()->ActiveDocumentType != igPartDocument )
        {
            QString errorMessage = "There no document or current document is not part document";
            m_logger.error( errorMessage.toStdString() );
            return;
        }

        PartDocumentPtr partDocument = CCommands::GetApplicationPtr()->ActiveDocument;
        QFile file( "D://" + GET_QSTRING_FROM_BSTR( partDocument->GetName() ) );
        file.open( QFile::Text | QFile::WriteOnly );
        QTextStream ts( &file );

        // dump features
        ModelsPtr modelsCollection = partDocument->Models; // Model consists features and body

        for ( long i = 1; i <= modelsCollection->Count; i++ )
        {
            ModelPtr model = modelsCollection->Item( i );
            ts << "Model \"" << GET_QSTRING_FROM_BSTR( model->GetName() ) << "\"\n";
            FeaturesPtr featuresCollection = model->GetFeatures();

            for ( long j = 1; j <= featuresCollection->Count; j++ )
            {
                IDispatchPtr feature = featuresCollection->Item( j );
                FeatureTypeConstants featType;
                _bstr_t featureName;
                ComGetProperty( feature, L"DisplayName", VT_I4, ( void* )&featureName );
                QString name = GET_QSTRING_FROM_BSTR( featureName );
                ComGetProperty( feature, L"Type", VT_I4, ( void* )&featType );

                ts << "  Feature \"" << name << "\" type is " << ( long )featType << "\n";

                if ( featType == igExtrudedProtrusionFeatureObject )
                {
                    ExtrudedProtrusionPtr protrusion = ( ExtrudedProtrusionPtr )feature;
                    long numProfiles = 0;
                    SAFEARRAY* saProfiles = 0x0;
                    protrusion->GetProfiles( &numProfiles, &saProfiles );

                    ts << "    Profiles count: " << numProfiles << "\n";

                    for ( long k = 0; k < numProfiles; k++ )
                    {
                        ProfilePtr profile;
                        ::SafeArrayGetElement( saProfiles, &k, &profile );
                        dumpProfile( profile, ts );
                    }

                    ::SafeArrayDestroy( saProfiles );

                    /*ProfilePtr profile = protrusion->GetProfile(); // NOTE: fail if more than one profile in feauture
                    dumpProfile(profile, ts);*/
                }
                else if ( featType == igExtrudedCutoutFeatureObject )
                {
                    ExtrudedCutoutPtr cutout = ( ExtrudedCutoutPtr )feature;
                    long numProfiles = 0;
                    SAFEARRAY* saProfiles = 0x0;
                    cutout->GetProfiles( &numProfiles, &saProfiles );

                    ts << "    Profiles count: " << numProfiles << "\n";

                    for ( long k = 0; k < numProfiles; k++ )
                    {
                        ProfilePtr profile;
                        ::SafeArrayGetElement( saProfiles, &k, &profile );
                        dumpProfile( profile, ts );
                    }

                    ::SafeArrayDestroy( saProfiles );

                    /*ProfilePtr profile = cutout->GetProfile();
                    dumpProfile(profile, ts);*/
                }
            }

            ts.flush();
        }

        // dump sketchs
        SketchsPtr sketchesCollection = partDocument->GetSketches();

        for ( long i = 1; i <= sketchesCollection->Count; i++ )
        {
            SketchPtr sketch = sketchesCollection->Item( i );
            ts << "Sketch name: \"" << GET_QSTRING_FROM_BSTR( sketch->GetName() ) << "\""
               << " DisplayName: \"" << GET_QSTRING_FROM_BSTR( sketch->DisplayName ) << "\""
               << " SystemName: \"" << GET_QSTRING_FROM_BSTR( sketch->SystemName ) << "\""
               << "\n";

            ProfilesPtr profiles = sketch->GetProfiles();

            ts << "  Profiles count: " << profiles->Count << "\n";

            for ( long k = 1; k < profiles->Count; k++ )
            {
                ProfilePtr profile = profiles->Item( k );
                dumpProfile( profile, ts );
            }

            IDispatchPtr profilesParent = profiles->Parent;
            long type;
            ComGetProperty( profilesParent, L"Type", VT_I4, ( void* )&type );
            ts << "  Profiles collection parent type: " << type << "\n";

            ProfilePtr profile = sketch->GetProfile();
            dumpProfile( profile, ts );

            IDispatchPtr sketchParent = sketch->Parent;
            ComGetProperty( sketchParent, L"Type", VT_I4, ( void* )&type );
            ts << "  Sketch parent type: " << type << "\n";
        }

        // dump constrainsts
        ConstraintsPtr constraints = partDocument->Constraints;
        ts << "Constraints count: " << constraints->Count << "\n";

        for ( long i = 1; i <= constraints->Count; i++ )
        {
            ConstraintPtr constraint = constraints->Item( i );
            int constraintType = ( int )constraint->Type;
            ts << "  Constraint type: " << constraintType << "\n";
            long geometryCount = constraint->GeometryCount;
            ts << "  Geometry count: " << geometryCount << "\n";
            ts << "  Constraint index: " << constraint->Index << "\n";
            ts << "  Constraint offset value: " << constraint->OffsetValue << "\n";
            IDispatchPtr parent = constraint->Parent;
            long type;
            ComGetProperty( parent, L"Type", VT_I4, ( void* )&type );
            ts << "  Constraint parent type: " << type << "\n";
        }
    }
    catch ( const _com_error& e )
    {
        QString errorMessage = "COM error in the extension: " + QString::fromLocal8Bit( e.ErrorMessage() );
        m_logger.error( errorMessage.toStdString() );
        return;
    }
    catch ( ... )
    {
        QString errorMessage = "Unknown exeption in the extension.";
        m_logger.error( errorMessage.toStdString() );
        return;
    }
}
