#include "Stable.h"
#include "NXExtensionImpl.h"
#include "../../utils/Point.h"

double Point3D::tolerance = 1e-12;

NXExtensionImpl NXExtensionImpl::m_instance;
HHOOK NXExtensionImpl::m_hHook = 0x0;

UF_MB_action_t NXExtensionImpl::m_actionTable[] =
{
    {( char* )"RUN_EDITOR_ACTION", &NXExtensionImpl::RunEditorAction, 0x0},
    {0x0, 0x0, 0x0}
};

/* The report function handles an error code from Open C and C++ API routines
* that return an int error code. The function passes the error code
* to UF_get_fail_message to get the message text associated with the
* error code, then prints the results.
*/
int NXExtensionImpl::Report( const char* file, int line, const char* call, int irc )
{
    if ( irc )
    {
        char messg[133], num[10], num2[10];
        BaseExtension::GetLogger().error(
            string( "NX API Error: " ) + file + ", line " +
            itoa( line, num, 10 ) + ": " + call +
            ( ( UF_get_fail_message( irc, messg ) ) ?
              ( string( " returned a " ) +  itoa( irc, num2, 10 ) ) :
              ( string( " returned error: " ) + messg ) ) );
    }

    return ( irc );
}

NXExtensionImpl::NXExtensionImpl() :
    BaseExtension(), m_registered( 0 )
{
}


NXExtensionImpl::~NXExtensionImpl()
{
}

void NXExtensionImpl::Initialize()
{
    //Call base class initialization first
    BaseExtension::Initialize();

    // Initialize the Open C API environment
    if ( UF_CALL( ::UF_initialize() ) )
        return;

    if ( m_registered == 0 )
    {
        UF_CALL( ::UF_MB_add_actions( m_actionTable ) );
        m_registered = 1;
    }
}

BaseExtension::Variant NXExtensionImpl::ExtractVariant()
{
    Variant variant;

    if ( ::UF_ASSEM_ask_work_part() == NULL_TAG )
    {
        QMessageBox::warning( GetTopWindow(), qApp->applicationName(),
                              QString( "Work part doesn't exist. Create new or open existing part model." ),
                              QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel );
        return Variant();
    }

    tag_t tagDisplayPart = ::UF_PART_ask_display_part();

    int number_of_exps;
    tag_t* exps = nullptr;
    UF_CALL( ::UF_MODL_ask_exps_of_part( tagDisplayPart, &number_of_exps, &exps ) );

    char* name = nullptr;
    double value = 0.0;

    for ( int i = 0; i < number_of_exps; i++ )
    {
        value = 0.0;
        UF_CALL( ::UF_MODL_ask_exp_tag_string( exps[i], &name ) );
        QString string = QString( name );
        QString before;
        size_t pos = string.indexOf( "=" );

        if ( pos != -1 )
            before = string.left( pos );
        else
            before = string;

        UF_CALL( ::UF_MODL_ask_exp_tag_value( exps[i], &value ) );
        variant.insert( before, value );
    }

    ::UF_free( name );
    ::UF_free( exps );

    return variant;



//    tag_t tagExpression = NULL_TAG;
//    UF_CALL( ::UF_MODL_ask_object( UF_solid_type,
//                                   UF_solid_body_subtype,
//                                   &tagExpression ) );

//    if ( tagExpression == NULL_TAG )
//        return;
//    else
//    {
//        do
//        {
//            int bodyType;
//            UF_CALL( ::UF_MODL_ask_body_type( tagExpression, &bodyType ) );

//            if ( bodyType == UF_MODL_SOLID_BODY )
//                break;

//            ::UF_MODL_ask_object( UF_solid_type,
//                                  UF_solid_body_subtype,
//                                  &tagExpression );
//        } while ( tagExpression != NULL_TAG );
//    }

//    if ( tagExpression == NULL_TAG )
    //        return;
}

void NXExtensionImpl::ApplyVariant( BaseExtension::Variant variant )
{
    for ( auto& var : variant.keys() )
    {
        // Set new value for expression by providing an entire expression like 'exp=value'
        QString exp = QString( var + "=" + QString::number( ( variant[var] ) ) );
        QByteArray byteArray = exp.toStdString().c_str();
        char* expC = byteArray.data();
        UF_CALL( ::UF_MODL_edit_exp( expC ) );
    }

    // You call this routine after you use UF_MODL_import_exp, UF_MODL_edit_exp, and UF_MODL_move_feature.
    UF_CALL( ::UF_MODL_update() );
}

void NXExtensionImpl::WriteWavefrontObjFile( const QString& fileName,
                                             const std::set<TetgenPoint3D>& points,
                                             const std::vector<TetgenFacet>& facets )
{
    std::ofstream out( fileName.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + fileName.toStdString() );

    std::map<TetgenPoint3D, unsigned long long> vertex_indices;
    unsigned long long vertex_index = 1;

    for ( const auto& point : points )
    {
        out << "v " << point.point.x << " " << point.point.y << " " << point.point.z << std::endl;
        vertex_indices[point] = vertex_index++;
    }

    // Write the facet data
    for ( const auto& facet : facets )
    {
        const unsigned long long p1_index = vertex_indices[facet.p1];
        const unsigned long long p2_index = vertex_indices[facet.p2];
        const unsigned long long p3_index = vertex_indices[facet.p3];

        if ( p1_index > 0 && p2_index > 0 && p3_index > 0 )
            out << "f " << p1_index << " " << p2_index << " " << p3_index << std::endl;
    }

    out.close();
}

void NXExtensionImpl::WriteStlFile( const QString& fileName,
                                    const std::vector<TetgenFacet>& facets )
{
    std::ofstream out( fileName.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + fileName.toStdString() );

    out << "solid my_solid\n";
    std::vector<TetgenFacet>::const_iterator it, end;

    for ( it = facets.begin(), end = facets.end(); it != end; ++it )
    {
        out << "  facet normal 0 0 0\n"
            << "    outer loop\n"
            << "      vertex " << it->p1.point.x << " " << it->p1.point.y << " " << it->p1.point.z << "\n"
            << "      vertex " << it->p2.point.x << " " << it->p2.point.y << " " << it->p2.point.z << "\n"
            << "      vertex " << it->p3.point.x << " " << it->p3.point.y << " " << it->p3.point.z << "\n"
            << "    endloop\n"
            << "  endfacet\n";
    }

    out << "endsolid my_solid\n";
    out.close();
}

void NXExtensionImpl::WritePolyFile( const QString& fileName,
                                     const std::set<TetgenPoint3D>& points,
                                     const std::vector<TetgenFacet>& facets )
{
    std::ofstream out( fileName.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + fileName.toStdString() );

    // Write the number of points and dimensions to the file
    out << points.size() << " 3 0 1" << std::endl;

    unsigned long long index = 1;
    std::map<TetgenPoint3D, unsigned long long> point_index_map;

    // Write vertices section
    for ( const auto& point : points )
    {
        out << index++ << " " << point.point.x << " " << point.point.y << " " <<  point.point.z << " ";

        if ( point.marker == BoundaryMarker::DEFAULT )
            out << BoundaryMarker::DEFAULT;
        else if ( point.marker == BoundaryMarker::CONSTRAINT )
            out << BoundaryMarker::CONSTRAINT;
        else if ( point.marker == BoundaryMarker::FORCE )
            out << BoundaryMarker::FORCE;
        else if ( point.marker == BoundaryMarker::MORE_DETAILED )
            out << BoundaryMarker::MORE_DETAILED;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_TOP )
            out << BoundaryMarker::INTERMEDIATE_TOP;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_MIDDLE )
            out << BoundaryMarker::INTERMEDIATE_MIDDLE;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_BOTTOM )
            out << BoundaryMarker::INTERMEDIATE_BOTTOM;

        out << std::endl;
        point_index_map[point] = index - 1;
    }

    // Write facets section
    out << facets.size() << " 1\n";
    index = 1;

    for ( const auto& facet : facets )
    {
        out << "1 0 ";

        if ( facet.marker == BoundaryMarker::DEFAULT )
            out << BoundaryMarker::DEFAULT;
        else if ( facet.marker == BoundaryMarker::CONSTRAINT )
            out << BoundaryMarker::CONSTRAINT;
        else if ( facet.marker == BoundaryMarker::FORCE )
            out << BoundaryMarker::FORCE;
        else if ( facet.marker == BoundaryMarker::MORE_DETAILED )
            out << BoundaryMarker::MORE_DETAILED;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_TOP )
            out << BoundaryMarker::INTERMEDIATE_TOP;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_MIDDLE )
            out << BoundaryMarker::INTERMEDIATE_MIDDLE;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_BOTTOM )
            out << BoundaryMarker::INTERMEDIATE_BOTTOM;

        out << "\n";
        index++;

        out << "3 " << point_index_map[facet.p1] << " "
            << point_index_map[facet.p2] << " "
            << point_index_map[facet.p3] << "\n";
    }

    // part 3 and part 4
    out << "0\n0" << std::endl;
    out.close();
}

void NXExtensionImpl::WriteSmeshFile( const QString& fileName,
                                      const std::set<TetgenPoint3D>& points,
                                      const std::vector<TetgenFacet>& facets )
{
    std::ofstream out( fileName.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + fileName.toStdString() );

    // Write the number of points and dimensions to the file
    out << points.size() << " 3 0 1" << std::endl;

    unsigned long long index = 1;
    std::map<TetgenPoint3D, unsigned long long> point_index_map;

    // Write vertices section
    for ( const auto& point : points )
    {
        out << index++ << " " << point.point.x << " " << point.point.y << " " <<  point.point.z << " ";

        if ( point.marker == BoundaryMarker::DEFAULT )
            out << BoundaryMarker::DEFAULT;
        else if ( point.marker == BoundaryMarker::CONSTRAINT )
            out << BoundaryMarker::CONSTRAINT;
        else if ( point.marker == BoundaryMarker::FORCE )
            out << BoundaryMarker::FORCE;
        else if ( point.marker == BoundaryMarker::MORE_DETAILED )
            out << BoundaryMarker::MORE_DETAILED;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_TOP )
            out << BoundaryMarker::INTERMEDIATE_TOP;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_MIDDLE )
            out << BoundaryMarker::INTERMEDIATE_MIDDLE;
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_BOTTOM )
            out << BoundaryMarker::INTERMEDIATE_BOTTOM;

        out << std::endl;
        point_index_map[point] = index - 1;
    }

    // Write facets section
    out << facets.size() << " 1\n";
    index = 1;

    for ( const auto& facet : facets )
    {
        index++;

        out << "3 " << point_index_map[facet.p1] << " "
            << point_index_map[facet.p2] << " "
            << point_index_map[facet.p3] << " ";

        if ( facet.marker == BoundaryMarker::DEFAULT )
            out << BoundaryMarker::DEFAULT;
        else if ( facet.marker == BoundaryMarker::CONSTRAINT )
            out << BoundaryMarker::CONSTRAINT;
        else if ( facet.marker == BoundaryMarker::FORCE )
            out << BoundaryMarker::FORCE;
        else if ( facet.marker == BoundaryMarker::MORE_DETAILED )
            out << BoundaryMarker::MORE_DETAILED;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_TOP )
            out << BoundaryMarker::INTERMEDIATE_TOP;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_MIDDLE )
            out << BoundaryMarker::INTERMEDIATE_MIDDLE;
        else if ( facet.marker == BoundaryMarker::INTERMEDIATE_BOTTOM )
            out << BoundaryMarker::INTERMEDIATE_BOTTOM;

        out << "\n";
    }

    // part 3 and part 4
    out << "0\n0" << std::endl;
    out.close();
}

void NXExtensionImpl::WriteMtrFile( const QString& fileName,
                                    const std::set<TetgenPoint3D>& points,
                                    const double& bounding_box_diagonal_size )
{
    std::ofstream out( fileName.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + fileName.toStdString() );

    out << points.size() << " 1\n";

    // for max_facet_size == 0.016
//    double force_length = bounding_box_diagonal_size
//                          * 0.011/*parameter to be configurable*/;
//    double more_detailed_length = bounding_box_diagonal_size
//                                  * 0.005/*parameter to be configurable*/;
//    double intermediate_top_length = bounding_box_diagonal_size
//                                     * 0.01070/*parameter to be configurable*/;
//    double intermediate_middle_length = bounding_box_diagonal_size
//                                        * 0.0099/*parameter to be configurable*/;
//    double intermediate_bottom_length = bounding_box_diagonal_size
//                                        * 0.0107/*parameter to be configurable*/;
//    double constraint_length = bounding_box_diagonal_size
//                               * 0.0097/*parameter to be configurable*/;

    double force_length = bounding_box_diagonal_size
                          * 0.031/*parameter to be configurable*/;
    double more_detailed_length = bounding_box_diagonal_size
                                  * 0.0069/*parameter to be configurable*/;
    double intermediate_top_length = bounding_box_diagonal_size
                                     * 0.01376/*parameter to be configurable*/;
    double intermediate_middle_length = bounding_box_diagonal_size
                                        * 0.01975/*parameter to be configurable*/;
    double intermediate_bottom_length = bounding_box_diagonal_size
                                        * 0.01950/*parameter to be configurable*/;
    double constraint_length = bounding_box_diagonal_size
                               * 0.01985/*parameter to be configurable*/;

    // Write vertices mtr
    for ( const auto& point : points )
        if ( point.marker == BoundaryMarker::FORCE )
            out << force_length << "\n";
        else if ( point.marker == BoundaryMarker::MORE_DETAILED )
            out << more_detailed_length << "\n";
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_TOP )
            out << intermediate_top_length << "\n";
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_MIDDLE )
            out << intermediate_middle_length << "\n";
        else if ( point.marker == BoundaryMarker::INTERMEDIATE_BOTTOM )
            out << intermediate_bottom_length << "\n";
        else if ( point.marker == BoundaryMarker::CONSTRAINT )
            out << constraint_length << "\n";

    out.close();
}

void NXExtensionImpl::GetMeshData( std::set<TetgenPoint3D>& mesh_points,
                                   std::vector<TetgenFacet>& mesh_facets,
                                   double& mesh_bounding_box_diagonal_size,
                                   double& mesh_max_facet_size )
{
    // UF_PART_ask_diplay_part
    tag_t tag_display_part = NULL_TAG;
    tag_display_part = ::UF_PART_ask_display_part();

    if ( tag_display_part == NULL_TAG )
        throw std::exception( "::UF_PART_ask_display_part() returned NULL part tag" );

    // UF_OBJ_cycle_objs_in_part
    tag_t tag_solid_body = NULL_TAG;
    UF_CALL( ::UF_OBJ_cycle_objs_in_part( tag_display_part,
                                          UF_solid_type,
                                          &tag_solid_body ) );

    // UF_FACET_parameters_t
    UF_FACET_parameters_t faceting_parameters;
    UF_FACET_INIT_PARAMETERS( &faceting_parameters );
    UF_CALL( ::UF_FACET_ask_default_parameters( &faceting_parameters ) );

    faceting_parameters.max_facet_edges = 3; // сетка из треугольников

//    faceting_parameters.specify_surface_tolerance = true;
//    faceting_parameters.surface_dist_tolerance = 3.0;
//    faceting_parameters.surface_angular_tolerance = 0.0;

//    faceting_parameters.specify_curve_tolerance = true;
//    faceting_parameters.curve_dist_tolerance = 3.0;
//    faceting_parameters.curve_angular_tolerance =  0.0;
//    faceting_parameters.curve_max_length = DBL_MAX;

    double box[6];
    UF_CALL( ::UF_MODL_ask_bounding_box( tag_solid_body, box ) );

    Point3D corner_point[2] = {&box[0], &box[3]};

    // return bounding box size
    mesh_bounding_box_diagonal_size = corner_point[0].DistanceTo( corner_point[1] );
    mesh_max_facet_size = mesh_bounding_box_diagonal_size * 0.073/*parameter to be configurable*/;

    faceting_parameters.specify_max_facet_size = true;
    faceting_parameters.max_facet_size = mesh_max_facet_size;

    // UF_FACET_facet_solid
    tag_t tag_faceted_model = NULL_TAG;
    UF_CALL( ::UF_FACET_facet_solid( tag_solid_body,
                                     &faceting_parameters,
                                     &tag_faceted_model ) );

    // запросить количество вершин фасетной модели
    int facet_id = UF_FACET_NULL_FACET_ID;
    UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );

    // каждый фасет (полигон) всегда состоит из 3-х вершин
    // двумерный массив под вершины фасетной модели
    double facet_vertices[3][3];

    tag_t saved_face_tag = NULL_TAG;

    while ( facet_id != UF_FACET_NULL_FACET_ID )
    {
        int verts_in_facet = 0;
        UF_CALL( ::UF_FACET_ask_vertices_of_facet( tag_faceted_model,
                                                   facet_id,
                                                   &verts_in_facet,
                                                   facet_vertices ) );

        tag_t face_tag = NULL_TAG;
        UF_CALL( ::UF_FACET_ask_solid_face_of_facet( tag_faceted_model,
                                                     facet_id,
                                                     &face_tag ) );

        if ( saved_face_tag != face_tag )
            saved_face_tag = face_tag;

        // check constraint
        const char* title_constraint = "CONST";
        char* string_value_constraint = 0;
        logical has_attribute_constraint = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_constraint,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_constraint,
                                                      &has_attribute_constraint ) );

        // check force
        const char* title_force = "FORCE";
        char* string_value_force = 0;
        logical has_attribute_force = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_force,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_force,
                                                      &has_attribute_force ) );

        // check more detailed
        const char* title_more_detailed = "MORE_DETAILED";
        char* string_value_more_detailed = 0;
        logical has_attribute_more_detailed = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_more_detailed,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_more_detailed,
                                                      &has_attribute_more_detailed ) );

        // check intermediate top
        const char* title_intermediate_top = "INTERMEDIATE_TOP";
        char* string_value_intermediate_top = 0;
        logical has_attribute_intermediate_top = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_intermediate_top,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_intermediate_top,
                                                      &has_attribute_intermediate_top ) );

        // check intermediate middle
        const char* title_intermediate_middle = "INTERMEDIATE_MIDDLE";
        char* string_value_intermediate_middle = 0;
        logical has_attribute_intermediate_middle = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_intermediate_middle,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_intermediate_middle,
                                                      &has_attribute_intermediate_middle ) );

        // check intermediate bottom
        const char* title_intermediate_bottom = "INTERMEDIATE_BOTTOM";
        char* string_value_intermediate_bottom = 0;
        logical has_attribute_intermediate_bottom = false;
        UF_CALL( ::UF_ATTR_get_string_user_attribute( face_tag,
                                                      title_intermediate_bottom,
                                                      UF_ATTR_NOT_ARRAY,
                                                      &string_value_intermediate_bottom,
                                                      &has_attribute_intermediate_bottom ) );

        TetgenFacet tetgen_facet;
        tetgen_facet.marker = BoundaryMarker::DEFAULT;

        for ( int i = 0; i < verts_in_facet; i++ )
        {
            Point3D current_point( facet_vertices[i][0], facet_vertices[i][1], facet_vertices[i][2] );
            TetgenPoint3D current_tet_point{ current_point, BoundaryMarker::DEFAULT, 0.0 };

            if ( has_attribute_constraint )
                current_tet_point.marker = BoundaryMarker::CONSTRAINT;

            if ( has_attribute_force )
                current_tet_point.marker = BoundaryMarker::FORCE;

            if ( has_attribute_more_detailed )
                current_tet_point.marker = BoundaryMarker::MORE_DETAILED;

            if ( has_attribute_intermediate_top )
                current_tet_point.marker = BoundaryMarker::INTERMEDIATE_TOP;

            if ( has_attribute_intermediate_middle )
                current_tet_point.marker = BoundaryMarker::INTERMEDIATE_MIDDLE;

            if ( has_attribute_intermediate_bottom )
                current_tet_point.marker = BoundaryMarker::INTERMEDIATE_BOTTOM;

            switch ( i )
            {
                case 0:
                    // first point in facet
                    tetgen_facet.p1 = current_tet_point;
                    break;

                case 1:
                    tetgen_facet.p2 = current_tet_point;
                    break;

                case 2:
                    // last point in facet
                    tetgen_facet.p3 = current_tet_point;
                    break;
            }

            // add point
            mesh_points.insert( current_tet_point );
        }

        if ( has_attribute_constraint )
        {
            // set facet boundary marker
            tetgen_facet.marker = BoundaryMarker::CONSTRAINT;

            // free memory
            ::UF_free( string_value_constraint );
        }

        if ( has_attribute_force )
        {
            // set facet boundary marker
            tetgen_facet.marker = BoundaryMarker::FORCE;

            // free memory
            ::UF_free( string_value_force );
        }

        if ( has_attribute_more_detailed )
        {
            tetgen_facet.marker = BoundaryMarker::MORE_DETAILED;
            ::UF_free( string_value_more_detailed );
        }

        if ( has_attribute_intermediate_top )
        {
            tetgen_facet.marker = BoundaryMarker::INTERMEDIATE_TOP;
            ::UF_free( string_value_intermediate_top );
        }

        if ( has_attribute_intermediate_middle )
        {
            tetgen_facet.marker = BoundaryMarker::INTERMEDIATE_MIDDLE;
            ::UF_free( string_value_intermediate_middle );
        }

        if ( has_attribute_intermediate_bottom )
        {
            tetgen_facet.marker = BoundaryMarker::INTERMEDIATE_BOTTOM;
            ::UF_free( string_value_intermediate_bottom );
        }

        // add facet
        mesh_facets.push_back( tetgen_facet );

        // next facet
        UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );
    }
}

void NXExtensionImpl::SaveMeshDatabase( const QString& wavefront_obj_file_path,
                                        const QString& stl_file_path,
                                        const QString& tetgen_input_poly_file_path,
                                        const QString& tetgen_input_smesh_file_path,
                                        const QString& tetgen_input_mtr_file_path,
                                        double& max_facet_size )
{
    std::set<TetgenPoint3D> points;
    std::vector<TetgenFacet> facets;

    double bounding_box_diagonal_size = -1;

    // get points, facets and bounding box size
    GetMeshData( points, facets, bounding_box_diagonal_size, max_facet_size );

    // write files
    WriteWavefrontObjFile( wavefront_obj_file_path, points, facets );
    WriteStlFile( stl_file_path, facets );
    WritePolyFile( tetgen_input_poly_file_path, points, facets );
    WriteSmeshFile( tetgen_input_smesh_file_path, points, facets );
    WriteMtrFile( tetgen_input_mtr_file_path, points, bounding_box_diagonal_size );
}
