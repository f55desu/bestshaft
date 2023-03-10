#include "Stable.h"
#include "NXExtensionImpl.h"

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

    for (int i = 0; i < number_of_exps; i++)
    {
        value = 0.0;
        UF_CALL( ::UF_MODL_ask_exp_tag_string( exps[i], &name ) );
        QString string = QString(name);
        QString before;
        size_t pos = string.indexOf("=");
        if (pos != -1)
        {
            before = string.left(pos);
        }
        else
        {
            before = string;
        }
        UF_CALL( ::UF_MODL_ask_exp_tag_value( exps[i], &value));
        variant.insert(before, value);
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

void NXExtensionImpl::ApplyVariant(BaseExtension::Variant variant)
{
    //tag_t tagDisplayPart = ::UF_PART_ask_display_part();
    for (auto &var : variant.keys())
    {
//        tag_t expression_tag;
//        // Ask for expression tag by label first
//        UF_CALL(::UF_MODL_ask_exp_tag_by_name(var.toStdString(), &expression_tag));

        // Set new value for expression by providing an entire expression like 'exp=value'
        QString exp = QString(var + "=" + QString::number((variant[var])));
        QByteArray byteArray = exp.toStdString().c_str();
        char *expC = byteArray.data();
        UF_CALL(::UF_MODL_edit_exp(expC));
    }

    // You call this routine after you use UF_MODL_import_exp, UF_MODL_edit_exp, and UF_MODL_move_feature.
    UF_CALL(::UF_MODL_update());
}

void NXExtensionImpl::CalculateMaxTension(BaseExtension::Variant variant)
{
//    UF_initialize();
    GetMesh();
//    UF_terminate();
}

void NXExtensionImpl::writeSTL(const std::vector<double>& vertices, const std::string& filename)
{
  std::ofstream file;
  file.open(filename.c_str());

  file << "solid myobject\n";

  const int numTriangles = vertices.size() / 9;
  for (int i = 0; i < numTriangles; ++i)
  {
    const int j = i * 9;

    const double nx = (vertices[j + 1] - vertices[j]) * (vertices[j + 5] - vertices[j + 2]) - (vertices[j + 2] - vertices[j]) * (vertices[j + 4] - vertices[j + 1]);
    const double ny = (vertices[j + 2] - vertices[j]) * (vertices[j + 3] - vertices[j + 1]) - (vertices[j + 3] - vertices[j]) * (vertices[j + 2] - vertices[j + 1]);
    const double nz = (vertices[j + 3] - vertices[j]) * (vertices[j + 7] - vertices[j + 4]) - (vertices[j + 4] - vertices[j]) * (vertices[j + 6] - vertices[j + 3]);

    const double len = std::sqrt(nx * nx + ny * ny + nz * nz);

    const double nnx = nx / len;
    const double nny = ny / len;
    const double nnz = nz / len;

    file << "facet normal " << nnx << " " << nny << " " << nnz << "\n";
    file << "  outer loop\n";
    file << "    vertex " << vertices[j] << " " << vertices[j + 1] << " " << vertices[j + 2] << "\n";
    file << "    vertex " << vertices[j + 3] << " " << vertices[j + 4] << " " << vertices[j + 5] << "\n";
    file << "    vertex " << vertices[j + 6] << " " << vertices[j + 7] << " " << vertices[j + 8] << "\n";
    file << "  endloop\n";
    file << "endfacet\n";
  }

  file << "endsolid myobject\n";

  file.close();
}

//void NXExtensionImpl::writeMedit(const std::vector<double>& vertices, const std::vector<int>& triangles, const std::string& filename)
//{
//  const int numVertices = vertices.size() / 3;
//  const int numTriangles = triangles.size() / 3;

//  std::ofstream file;
//  file.open(filename.c_str());

//  file << numVertices << " " << numTriangles << " " << 0 << "\n";

//  for (int i = 0; i < numVertices; ++i)
//  {
//    const int j = i * 3;
//    file << vertices[j] << " " << vertices[j + 1] << " " << vertices[j + 2] << " " << 0 << "\n";
//  }

//  for (int i = 0; i < numTriangles; ++i)
//  {
//    const int j = i * 3;
//    file << "3 " << triangles[j] << " " << triangles[j + 1] << " " << triangles[j + 2] << "\n";
//  }

//  file.close();
//}

void NXExtensionImpl::GetMesh()
{
//    // UF_ASSEM_ask_work_part
//    tag_t current_work_part_tag = NULL_TAG;
//    current_work_part_tag = UF_CALL( ::UF_ASSEM_ask_work_part() );

//    if ( current_work_part_tag == NULL_TAG )
//    {
//        qDebug() << "current_work_part_tag is NULL";
//        return;
//    }

    // UF_PART_ask_diplay_part
    tag_t tag_display_part = NULL_TAG;
    tag_display_part = ::UF_PART_ask_display_part();

    if ( tag_display_part == NULL_TAG )
    {
        qDebug() << "tag_display_part is NULL";
        return;
    }

    qDebug() << "tag_display_part: " << tag_display_part;

//    // UF_PART_ask_part_name
//    char work_part_name[MAX_FSPEC_SIZE+1];
//    UF_CALL( ::UF_PART_ask_part_name( tag_display_part, work_part_name ) );
//    qDebug() << "Work part name: " << QString::fromUtf8( work_part_name, MAX_FSPEC_SIZE+1 );

//    tag_t tag_block_feature = NULL_TAG;
//    UF_CALL( ::UF_MODL_create_block( tag_display_part,
//                                     ,
//                                     ,
//                                     &tag_block_feature) );

//    UF_CALL( ::UF_MODL_active_part( tag_display_part,
//                                    0) );

    // параметры детали
//    tag_t *exps;
//    int number_of_exps = 0;
//    UF_CALL( ::UF_MODL_ask_exps_of_part( tag_display_part,
//                                         &number_of_exps,
//                                         &exps) );

    tag_t tag_faceted_model = NULL_TAG;
    UF_CALL( ::UF_OBJ_cycle_objs_in_part( tag_display_part,
                                          UF_faceted_model_type,
                                          &tag_faceted_model) );

//    tag_t tag_block = NULL_TAG;
//    UF_CALL( ::UF_MODL_ask_feat_body( tag_feature,
//                                      &tag_block ) );

    UF_FACET_parameters_t faceting_parameters;
    UF_FACET_INIT_PARAMETERS( &faceting_parameters );
    UF_CALL( ::UF_FACET_ask_default_parameters( &faceting_parameters ) );

//    tag_t tag_faceted_model = NULL_TAG;
//    UF_CALL( ::UF_FACET_facet_solid( tag_feature,
//                                     &faceting_parameters,
//                                     &tag_faceted_model ) );

    UF_CALL( ::UF_FACET_ask_model_parameters( tag_faceted_model, &faceting_parameters ) );

    // UF_FACET_ask_num_facets
    int num_faces = 0;
    UF_CALL( ::UF_FACET_ask_num_faces( tag_faceted_model, &num_faces ) );
    qDebug() << "Num faces: " << num_faces;

    // поток для вывода вершин
    std::ostringstream out;
//    int vertices_count = 0;

    std::vector<double> all_vertices;

    // запросить количество вершин фасетной модели
    int facet_id = UF_FACET_NULL_FACET_ID;
    UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );
    while( facet_id != UF_FACET_NULL_FACET_ID )
    {
        int num_vertices_in_facet = 0;
        UF_CALL( ::UF_FACET_ask_num_verts_in_facet( tag_faceted_model,
                                                    facet_id,
                                                    &num_vertices_in_facet) );

        // двумерный массив под вершины фасетной модели
        double (* facet_vertices)[3];
        int facet_vertices_size = 3 * num_vertices_in_facet * sizeof(double);
        facet_vertices = (double(*)[3])malloc(facet_vertices_size);

        int verts_in_facet = 0;
        UF_CALL( ::UF_FACET_ask_vertices_of_facet( tag_faceted_model,
                                                   facet_id,
                                                   &verts_in_facet,
                                                   facet_vertices) );

        for ( int i = 0; i < num_vertices_in_facet; i++ )
        {
            // stl export
            all_vertices.push_back(facet_vertices[i][0]);
            all_vertices.push_back(facet_vertices[i][1]);
            all_vertices.push_back(facet_vertices[i][2]);

            // wavefront obj export
            out << "v " << facet_vertices[i][0]
                << " " << facet_vertices[i][1]
                << " " << facet_vertices[i][2]
                << '\n';
        }

//        vertices_count += num_vertices_in_facet;
        UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );
    }

    // stl export
    writeSTL(all_vertices, "C:\\Users\\Joel\\Desktop\\FEM_OBJ_TEST.stl");

//    // wavefront export model
//    for (int i = 0; i < all_vertices.size() / 3; i += 3)
//        out << "f " << i + 1 << " " << i + 2 << " " << i + 3 << std::endl;

    // wavefront export wirefront
    for (int i = 0; i < all_vertices.size() / 3; i += 3)
        out << "l " << i + 1 << " " << i + 2 << " " << i + 3 << " " << i + 1 << std::endl;

    // Open a file in output mode
    std::string filename = "C:\\Users\\Joel\\Desktop\\FEM_OBJ_TEST.obj";

    std::ofstream outfile;
    outfile.open( filename, std::ios::out );

    if ( !outfile.good() )
    {
        outfile.close();
        return;
    }

    outfile << out.str();
    outfile.close();

// bad code
//    std::vector<int> triangles;
//    for (int i = 0; i < vertices_count; i++)
//        triangles.push_back(i);

//    writeMedit(all_vertices, triangles, "C:\\Users\\Joel\\Desktop\\FEM_OBJ_TEST.mesh");

//    // Output to VTK file
//    std::ofstream file;
//    file.open("C:\\Users\\Joel\\Desktop\\FEM_OBJ_TEST.vtk");

//    file << "# vtk DataFile Version 3.0\n";
//    file << "Points\n";
//    file << "ASCII\n";
//    file << "DATASET POLYDATA\n";
//    file << "POINTS " << vertices_count << " float\n";

//    for (int i = 0; i < vertices_count; ++i)
//    {
//        const int j = i * 3;
//        file << all_vertices[j] << " " << all_vertices[j + 1] << " " << all_vertices[j + 2] << "\n";
//    }

//    file.close();

//    // mesh file
//    std::ofstream outFile("C:\\Users\\Joel\\Desktop\\FEM_OBJ_TEST.mesh", std::ios::binary);

//    // mesh write vertices count
//    outFile.write(reinterpret_cast<char*>(&vertices_count), sizeof(vertices_count));

//    // mesh write vertices
//    outFile.write(reinterpret_cast<char*>(all_vertices.data()), all_vertices.size() * sizeof(double));

//    // mesh close file
//    outFile.close();

//    // mesh header
//    outFile << "MeshVersionFormatted 1\nDimension 3\n";

//    // Mesh write the vertices
//    outFile << "Vertices\n";
//    outFile << vertices_count << '\n';

//    for (int i = 0; i < vertices_count; i++) {
//        outFile.write(reinterpret_cast<const char*>(&all_vertices[i * 3]), sizeof(double) * 3);
//    }

//    // Mesh write the faces
//    outFile << "Triangles\n";
//    outFile << num_faces << '\n';
//    for (int i = 0; i < num_faces; i++) {
//        outFile.write(reinterpret_cast<const char*>(&face_indices[i * 3]), sizeof(int) * 3);
//    }

//    // Close the output file
//    outFile.close();

    // Генерация фасета по параметрам
//    tag_t tag_block_feature = NULL_TAG;
//    UF_CALL( ::UF_MODL_create_block1( ,
//                                      ,
//                                      ,
//                                      &tag_block_feature ) );


//    tag_t tag_block = NULL_TAG;=
//    UF_CALL( ::UF_MODL_ask_feat_body( tag_block_feature, &tag_block ) );

//    tag_t tag_solid_entity = NULL_TAG;
//    UF_CALL( ::UF_OBJ_cycle_objs_in_part( tag_display_part,
//                                          UF_solid_body_subtype,
//                                          &tag_solid_entity) );

//    tag_t tag_tetrahedral_faceted_model = NULL_TAG;
//    ::UF_SF_create_ugs_tet_mesh();

//    UF_CALL( ::UF_FACET_facet_solid( tag_block,
//                                     &faceting_parameters,
//                                     &tag_tetrahedral_faceted_model ) );

//    tag_t tag_model = 0;

//    int n_facet_models;
//    tag_t *facet_models;
//    UF_CALL( UF_FACET_ask_models_of_solid( tag_display_part,
//                                           &n_facet_models,
//                                           &facet_models ) );

//    tag_model = *facet_models;




//    logical result = false;
//    UF_CALL( ::UF_FACET_is_model_convex( tag_display_part, &result ) );
//    qDebug() << "Result: " << result;

//    // UF_SF_body_ask_faces
//    int num_faces = 0;

//    tag_p_t my_faces = 0;
//    tag_p_t *faces = &my_faces;

//    ::UF_SF_body_ask_faces( tag_display_part,
//                            &num_faces,
//                            faces );

//    UF_CALL( ::UF_SF_body_ask_faces( tag_display_part,
//                            &num_faces,
//                            &faces ) );


//    // UF_MODL_ask_feat_body
//    tag_t block_tag;
//    UF_CALL( UF_MODL_ask_feat_body( tag_display_part, &block_tag ) );

//    // UF_FACET_create_model
//    tag_model = 0;
//    //UF_FACET_create_model( tag_display_part, &tag_model );

//    // UF_FACET_cycle_facets
//    int facet_id = UF_FACET_NULL_FACET_ID,
//        adjacent_facets[3],
//        edge_in_adjacent_facet;

//    int ret = ::UF_FACET_cycle_facets( tag_model, &facet_id );
//    qDebug() << "UF_FACET_cycle_facets (ret) with tag_model: " << ret << "\n";

//    ret = UF_CALL( ::UF_FACET_cycle_facets( tag_display_part, &facet_id ) );
//    qDebug() << "UF_FACET_cycle_facets (ret - UF_CALL): " << ret << "\n";

//    qDebug() << "facet_id: " << facet_id;

//    while( facet_id != UF_FACET_NULL_FACET_ID )
//    {
//        for ( int edge = 0; edge < 3; edge++ )
//        {
//            ::UF_FACET_ask_adjacent_facet( tag_display_part,
//                                         facet_id,
//                                         edge,
//                                         &adjacent_facets[edge],
//                                         &edge_in_adjacent_facet );
//        }

//        qDebug() << "facet index: " << facet_id
//                 << "\tadjacent_facets: [" << adjacent_facets[0]
//                 << ", " << adjacent_facets[1]
//                 << ", " << adjacent_facets[2]
//                 << "]";

//        ::UF_FACET_cycle_facets( tag_display_part, &facet_id );
//    }


//    tag_t tagDisplayPart = ::UF_PART_ask_display_part();

//    int number_of_exps;
//    tag_t* exps = nullptr;
//    UF_CALL( ::UF_MODL_ask_exps_of_part( tagDisplayPart, &number_of_exps, &exps ) );

//    char* name = nullptr;
//    double value = 0.0;

//    for (int i = 0; i < number_of_exps; i++)
//    {
//        value = 0.0;
//        UF_CALL( ::UF_MODL_ask_exp_tag_string( exps[i], &name ) );
//        QString string = QString(name);
//        QString before;
//        size_t pos = string.indexOf("=");
//        if (pos != -1)
//        {
//            before = string.left(pos);
//        }
//        else
//        {
//            before = string;
//        }
//        UF_CALL( ::UF_MODL_ask_exp_tag_value( exps[i], &value));
//        variant.insert(before, value);
//    }

//    ::UF_free( name );
//    ::UF_free( exps );

}
