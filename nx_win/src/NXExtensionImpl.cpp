#include "Stable.h"
#include "NXExtensionImpl.h"
#include "../../utils/Point.h"

double Point3D::tolerance = 1e-3;

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

int NXExtensionImpl::SaveSTL( const QString& variant_name, QString& returned_file_path,
                              double& returned_max_facet_size )
{
    // UF_PART_ask_diplay_part
    tag_t tag_display_part = NULL_TAG;
    tag_display_part = ::UF_PART_ask_display_part();

    if ( tag_display_part == NULL_TAG )
    {
        // tag_display_part is NULL
        return 1;
    }

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

    faceting_parameters.specify_max_facet_size = true;
    faceting_parameters.max_facet_size = corner_point[0].DistanceTo( corner_point[1] ) *
                                         0.03/*parameter to be configurable*/;

    // UF_FACET_facet_solid
    tag_t tag_faceted_model = NULL_TAG;
    UF_CALL( ::UF_FACET_facet_solid( tag_solid_body,
                                     &faceting_parameters,
                                     &tag_faceted_model ) );

    // потоки для вывода в STL файл, в файл ограничений и нагрузок
    std::ostringstream out_stl, out_constraints, out_forces, out_vertices;
    out_stl << "solid my_solid\n";

    // запросить количество вершин фасетной модели
    int facet_id = UF_FACET_NULL_FACET_ID;
    UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );

    // каждый фасет (полигон) всегда состоит из 3-х вершин
    // двумерный массив под вершины фасетной модели
    double facet_vertices[3][3];

    tag_t saved_face_tag = NULL_TAG;

//    unsigned int vertex_counter = 0;

    std::set<TetgenPoint3D> cached_tetgen_points;
    std::set<TetgenFacet> cached_tetgen_facets;

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

        out_stl << "  facet normal 0 0 0\n";
        out_stl << "    outer loop\n";

//        // cache start and middle points from facet
//        TetgenPoint3D tet_point_start,
//                      tet_point_middle;

        TetgenFacet tetgen_facet;
        tetgen_facet.marker = BoundaryMarker::DEFAULT;

        for ( int i = 0; i < verts_in_facet; i++ )
        {
            Point3D current_point( facet_vertices[i][0], facet_vertices[i][1], facet_vertices[i][2] );
            TetgenPoint3D current_tet_point{ current_point, BoundaryMarker::DEFAULT, 0.0 };

            // write vertices to stl
            out_stl << "      vertex " << current_point.x << " " << current_point.y << " " << current_point.z << "\n";

            if ( has_attribute_constraint )
            {
                current_tet_point.marker = BoundaryMarker::CONSTRAINT;


//                // facet has constraint
//                out_constraints << vertex_counter << ",0,0.0\n"     // x
//                                << vertex_counter << ",1,0.0\n"     // y
//                                << vertex_counter << ",2,0.0\n";    // z
            }

            if ( has_attribute_force )
            {
                current_tet_point.marker = BoundaryMarker::FORCE;

//                // facet has force
//                out_forces << vertex_counter << ",0,0.0\n"     // x
//                           << vertex_counter << ",1,0.0\n"     // y
//                           << vertex_counter << ",2,0.0\n";    // z
            }

            switch ( i )
            {
                case 0:
                    tetgen_facet.p1 = current_tet_point;    // start
//                    tetgen_facet.p4 = current_tet_point;    // end
                    break;

                case 1:
                    tetgen_facet.p2 = current_tet_point;
                    break;

                case 2:
                    // last point from facet
                    tetgen_facet.p3 = current_tet_point;
                    break;
            }

            // cache tetgen point
            cached_tetgen_points.insert( current_tet_point );

//            out_vertices << vertex_counter << ",0," << point.x << "\n"     // x
//                         << vertex_counter << ",1," << point.y << "\n"     // y
//                         << vertex_counter << ",2," << point.z << "\n";    // z
        }

        out_stl << "    endloop\n";
        out_stl << "  endfacet\n";

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

        // cache tetgen facet
        cached_tetgen_facets.insert( tetgen_facet );

        // next facet
        UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );
    }

    out_stl << "endsolid my_solid\n";

    // parameters
    QString homePath = QDir::homePath();
    QDir homeDir( homePath );

    QString bestshaftWorkspaceFolder = "BestshaftWorkspace";

    // folder not exists
    if ( !homeDir.exists( bestshaftWorkspaceFolder ) )
    {
        if ( !homeDir.mkdir( bestshaftWorkspaceFolder ) )
        {
            // failed to create folder
            return 2;
        }

        // folder created successfuly
    }

    // folder exists
    QString bestshaftWorkspacePath = homePath + QDir::separator() + bestshaftWorkspaceFolder;
    QDir bestshaftWorkspaceDir( bestshaftWorkspacePath );

    // folder not exists
    if ( !bestshaftWorkspaceDir.exists( variant_name ) )
    {
        if ( !bestshaftWorkspaceDir.mkdir( variant_name ) )
        {
            // failed to create folder
            return 3;
        }

        // folder created successfuly
    }

    // if folder exists, write files. STL:
    QString defaultName = "model.tri.mesh.stl";
    QString filePath = bestshaftWorkspacePath + QDir::separator() + variant_name + QDir::separator() + defaultName;

    std::ofstream file;
    file.open( filePath.toStdString(), std::ios::out );

    if ( !file.good() )
    {
        // problems with file
        file.close();
        return 4;
    }

    file << out_stl.str();
    file.close();

    // write to .poly file
    defaultName = "model.tri.mesh.poly";
    filePath = bestshaftWorkspacePath + QDir::separator() + variant_name + QDir::separator() + defaultName;
    writePolyFile( filePath.toStdString(), cached_tetgen_points, cached_tetgen_facets );

//    // constraints:
//    defaultName = "nodes.disp.csv";
//    filePath = bestshaftWorkspacePath + QDir::separator() + variant_name + QDir::separator() + defaultName;

//    file.open( filePath.toStdString(), std::ios::out );

//    if ( !file.good() )
//    {
//        // problems with file
//        file.close();
//        return 5;
//    }

//    file << out_constraints.str();
//    file.close();

//    // forces:
//    defaultName = "nodes.force.csv";
//    filePath = bestshaftWorkspacePath + QDir::separator() + variant_name + QDir::separator() + defaultName;

//    file.open( filePath.toStdString(), std::ios::out );

//    if ( !file.good() )
//    {
//        // problems with file
//        file.close();
//        return 6;
//    }

//    file << out_forces.str();
//    file.close();

//    // all vertices:
//    defaultName = "nodes.all.csv";
//    filePath = bestshaftWorkspacePath + QDir::separator() + variant_name + QDir::separator() + defaultName;

//    file.open( filePath.toStdString(), std::ios::out );

//    if ( !file.good() )
//    {
//        // problems with file
//        file.close();
//        return 7;
//    }

//    file << out_vertices.str();
//    file.close();

    // return values
    returned_file_path = filePath;
    returned_max_facet_size = faceting_parameters.max_facet_size;

    return 0;
}

void NXExtensionImpl::writePolyFile( string fileName, std::set<TetgenPoint3D>& points, std::set<TetgenFacet>& facets )
{
    std::ofstream outFile( fileName );

    // Write the number of points and dimensions to the file
    outFile << points.size() << " 3 0 1" << std::endl;

    // Write the points to the file
    std::map<TetgenPoint3D, int> pointMap;
    int index = 1;

    for ( auto it = points.begin(); it != points.end(); it++ )
    {
        pointMap[*it] = index++;
        outFile << pointMap[*it] << " " << it->point.x << " " << it->point.y << " " << it->point.z << " ";

        if ( it->marker == BoundaryMarker::DEFAULT )
            outFile << BoundaryMarker::DEFAULT;
        else if ( it->marker == BoundaryMarker::CONSTRAINT )
            outFile << BoundaryMarker::CONSTRAINT;
        else if ( it->marker == BoundaryMarker::FORCE )
            outFile << BoundaryMarker::FORCE;

        outFile << std::endl;
    }

    // Write the number of facets to the file
    outFile << facets.size() << " 1" << std::endl;

    // Write the facets to the file
    std::map<TetgenFacet, int> facetMap;
    index = 1;

    for ( auto it = facets.begin(); it != facets.end(); it++ )
    {
        outFile << "1 0 ";

        if ( it->marker == BoundaryMarker::DEFAULT )
            outFile << BoundaryMarker::DEFAULT;
        else if ( it->marker == BoundaryMarker::CONSTRAINT )
            outFile << BoundaryMarker::CONSTRAINT;
        else if ( it->marker == BoundaryMarker::FORCE )
            outFile << BoundaryMarker::FORCE;

        outFile << "\n";

        facetMap[*it] = index++;
//        outFile << "4 " << pointMap[it->p1] << " " << pointMap[it->p2] << " " << pointMap[it->p3] << " " << pointMap[it->p4] <<
//                std::endl;
        outFile << "3 " << pointMap[it->p1] << " " << pointMap[it->p2] << " " << pointMap[it->p3] << std::endl;
    }

    // part 3 and part 4
    outFile << "0\n0" << std::endl;
    outFile.close();
}

void NXExtensionImpl::SaveAbaqusInputFile( const QString& tetgen_output_nodes_file_path,
                                           const QString& tetgen_output_faces_file_path,
                                           const QString& tetgen_output_tetrahedrons_file_path,
                                           const QString& calculix_input_file_path,
                                           const QString& variant_name,
                                           const double& applied_force )
{
    // read nodes
    std::ifstream in( tetgen_output_nodes_file_path.toStdString() );

    if ( !in.is_open() )
        throw std::runtime_error( "Cannot open file " + tetgen_output_nodes_file_path.toStdString() );

    int num_points, dimension, attributes_flag, boundary_markers_flag;
    in >> num_points >> dimension >> attributes_flag >> boundary_markers_flag;

    if ( num_points <= 0 || dimension != 3 || attributes_flag != 0 || boundary_markers_flag != 1 )
        throw std::runtime_error( "Invalid .node file format" );

    std::vector<TetgenPoint3D> points( num_points );

    for ( std::vector<TetgenPoint3D>::iterator it = points.begin(), end = points.end(); it != end; it++ )
        in >> *it;

    in.close();

    // read faces
    in.open( tetgen_output_faces_file_path.toStdString() );

    if ( !in.is_open() )
        throw std::runtime_error( "Cannot open file " + tetgen_output_faces_file_path.toStdString() );

    int num_faces;
    boundary_markers_flag = 0;

    in >> num_faces >> boundary_markers_flag;

    if ( num_faces <= 0 || boundary_markers_flag != 1 )
        throw std::runtime_error( "Invalid .face file format" );

    int face_id/* ignored */, p1_id, p2_id, p3_id, boundary_marker;

    double global_area = 0.0;
    std::vector<Facet> facets;

    for ( int index = 0; index < num_faces; index++ )
    {
        in >> face_id >> p1_id >> p2_id >> p3_id >> boundary_marker;

        if ( boundary_marker != BoundaryMarker::FORCE )
            continue;   // not force face

        Facet facet( p1_id - 1, p2_id - 1, p3_id - 1 );

        TetgenPoint3D p1 = points[facet._p1_id];
        TetgenPoint3D p2 = points[facet._p2_id];
        TetgenPoint3D p3 = points[facet._p3_id];

        // calculate facet area
        double a = p2.point.x - p1.point.x,
               b = p2.point.y - p1.point.y,
               c = p2.point.z - p1.point.z,
               d = p3.point.x - p1.point.x,
               e = p3.point.y - p1.point.y,
               f = p3.point.z - p1.point.z;

        double i = b * f - c * e,
               j = c * d - a * f,
               k = a * e - b * d;

        double local_area = 0.5 * std::sqrt( i * i + j * j + k * k );
        global_area += local_area;

        // save facet
        facet.facet_area = local_area;
        facets.push_back( facet );
    }

    in.close();

    // read tetrahedrones
    in.open( tetgen_output_tetrahedrons_file_path.toStdString() );

    if ( !in.is_open() )
        throw std::runtime_error( "Cannot open file " + tetgen_output_tetrahedrons_file_path.toStdString() );

    int num_tetrahedrons, nodes_per_tetrahedron;
    attributes_flag = 0;

    in >> num_tetrahedrons >> nodes_per_tetrahedron >> attributes_flag;

    if ( num_tetrahedrons <= 0 || nodes_per_tetrahedron != 4 || attributes_flag != 0 )
        throw std::runtime_error( "Invalid .ele file format" );

    std::vector<Tetrahedron> tetrahedrons( num_tetrahedrons );

    for ( std::vector<Tetrahedron>::iterator it = tetrahedrons.begin(), end = tetrahedrons.end(); it != end; it++ )
        in >> *it;

    in.close();

    // write to abaqus
    std::ofstream out( calculix_input_file_path.toStdString() );

    if ( !out.is_open() )
        throw std::runtime_error( "Cannot open file " + calculix_input_file_path.toStdString() );

    // write header
    out << "** General information\n"
        << "*HEADING\n"
        << variant_name.toStdString() << "\n\n";

//    out << "*PREPRINT, ECHO=NO, MODEL=NO, HISTORY=NO\n\n";

    // write nodes
    out << "** Defien set for all model nodes\n"
        << "*NODE, NSET=NODES\n";

    for ( int i = 0; i < points.size(); i++ )
        out << ( i + 1 ) << ", " << points[i].point.x << ", " << points[i].point.y << ", " << points[i].point.z << "\n";

    out << "\n";

    // write tetrahedrons
    out << "** All tetrahedrons\n"
        << "*ELEMENT, TYPE=C3D4, ELSET=TETRAHEDRONS\n";

    for ( int i = 0; i < tetrahedrons.size(); i++ )
        out << ( i + 1 ) << ", " << tetrahedrons[i].p1_id << ", " << tetrahedrons[i].p2_id << ", " <<
            tetrahedrons[i].p3_id << ", " << tetrahedrons[i].p4_id << "\n";

    out << "\n";

    // define element set
    out << "** Define set of elements\n"
        << "*ELSET, ELSET=ELEMENTS\n"
        << "TETRAHEDRONS\n\n";

    // element sets for materials and FEM element type
    out << "** Element sets for materials and FEM element type (solid, shell, beam, fluid)\n"
        << "*ELSET, ELSET=STEEL_SOLID\n"
        << "TETRAHEDRONS\n\n";

    // print set of constraint nodes
    out << "** Define set of constraint nodes\n"
        << "*NSET, NSET=CONSTRAINT_FIXED\n";

    for ( int i = 0; i < points.size(); i++ )
        if ( points[i].marker == BoundaryMarker::CONSTRAINT )
            out << ( i + 1 ) << ",\n";    // нулевое смещение в узле (закреплен намертво)

    out << "\n";

    // materials
    out << "** Description of material properties\n"
        << "*MATERIAL, NAME=STEEL\n"
        << "*ELASTIC\n"
        << "200.E9, 0.3\n\n";

    // set solid section
    out << "** Set material to all const nodes\n"
        << "*SOLID SECTION, ELSET=STEEL_SOLID, MATERIAL=STEEL\n\n";

//    // step begin
//    out << "*STEP, PERTURBATION\n"
//        << "*STATIC\n\n";

    // step begin
    out << "** Step begin\n"
        << "*STEP\n"
        << "*STATIC\n\n";

    // fixed nodes
    out << "** Fixed nodes\n"
        << "*BOUNDARY\n"
        << "CONSTRAINT_FIXED, 1\n"
        << "CONSTRAINT_FIXED, 2\n"
        << "CONSTRAINT_FIXED, 3\n\n";

    // calculate loads
    double distributed_load = applied_force /
                              global_area;    // распределенная нагрузка грани, глобальная сила == 1

    for ( Facet facet : facets )
    {
        double force = ( distributed_load * facet.facet_area ) / 3.0;

        points[facet._p1_id].force_value += force;
        points[facet._p2_id].force_value += force;
        points[facet._p3_id].force_value += force;
    }

    // loads
    out << "** Loads of each node\n"
        << "*CLOAD\n";

    for ( int i = 0; i < points.size(); i++ )
        if ( points[i].marker == BoundaryMarker::FORCE )
            out << ( i + 1 ) << ", 2, " << points[i].force_value << "\n";    // нагрузка против оси Y

    out << "\n";

    // print data
    out << "** Output result to .frd and .dat files\n"
        << "*NODE FILE\n"
        << "U\n"
        << "*EL FILE\n"
        << "S, E\n"
        << "*NODE PRINT, NSET=CONSTRAINT_FIXED, TOTALS=ONLY\n"
        << "RF\n\n";

    // end step
    out << "** Step end\n"
        << "*END STEP"
        << std::endl;

    out.close();
}
