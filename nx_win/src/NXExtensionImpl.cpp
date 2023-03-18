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
    //tag_t tagDisplayPart = ::UF_PART_ask_display_part();
    for ( auto& var : variant.keys() )
    {
//        tag_t expression_tag;
//        // Ask for expression tag by label first
//        UF_CALL(::UF_MODL_ask_exp_tag_by_name(var.toStdString(), &expression_tag));

        // Set new value for expression by providing an entire expression like 'exp=value'
        QString exp = QString( var + "=" + QString::number( ( variant[var] ) ) );
        QByteArray byteArray = exp.toStdString().c_str();
        char* expC = byteArray.data();
        UF_CALL( ::UF_MODL_edit_exp( expC ) );
    }

    // You call this routine after you use UF_MODL_import_exp, UF_MODL_edit_exp, and UF_MODL_move_feature.
    UF_CALL( ::UF_MODL_update() );
}

int NXExtensionImpl::SaveSTL( BaseExtension::Variant variant )
{
    // meshModel.SaveSTL(QSettings.GetWorkSpace + QDir::separator() + variant.id + QDir::separator() + TriMesh::DefaultName);

//    struct Vertex
//    {
//        double x, y, z;
//    };

//    struct Triangle
//    {
//        Vertex v1, v2, v3;
//    };

    // UF_PART_ask_diplay_part
    tag_t tag_display_part = NULL_TAG;
    tag_display_part = ::UF_PART_ask_display_part();

    if ( tag_display_part == NULL_TAG )
    {
        // tag_display_part is NULL
        return 1;
    }

    qDebug() << "tag_display_part: " << tag_display_part;

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

    faceting_parameters.specify_surface_tolerance = true;
    faceting_parameters.surface_dist_tolerance = 3.0;
    faceting_parameters.surface_angular_tolerance = 0.0;

    faceting_parameters.specify_curve_tolerance = true;
    faceting_parameters.curve_dist_tolerance = 3.0;
    faceting_parameters.curve_angular_tolerance =  0.0;
    faceting_parameters.curve_max_length = DBL_MAX;

    faceting_parameters.specify_max_facet_size = true;
    faceting_parameters.max_facet_size = 3.0;

    // UF_FACET_facet_solid
    tag_t tag_faceted_model = NULL_TAG;
    UF_CALL( ::UF_FACET_facet_solid( tag_solid_body,
                                     &faceting_parameters,
                                     &tag_faceted_model ) );

    // поток для вывода в STL файл
    std::ostringstream out;
    out << "solid my_solid\n";

    // запросить количество вершин фасетной модели
    int facet_id = UF_FACET_NULL_FACET_ID;
    UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );

    while ( facet_id != UF_FACET_NULL_FACET_ID )
    {
        int num_vertices_in_facet = 0;
        UF_CALL( ::UF_FACET_ask_num_verts_in_facet( tag_faceted_model,
                                                    facet_id,
                                                    &num_vertices_in_facet ) );

        // двумерный массив под вершины фасетной модели
        double ( * facet_vertices )[3];
        int facet_vertices_size = 3 * num_vertices_in_facet * sizeof( double );
        facet_vertices = ( double( * )[3] )malloc( facet_vertices_size );

        int verts_in_facet = 0;
        UF_CALL( ::UF_FACET_ask_vertices_of_facet( tag_faceted_model,
                                                   facet_id,
                                                   &verts_in_facet,
                                                   facet_vertices ) );

        out << "  facet normal 0 0 0\n";
        out << "    outer loop\n";

        for ( int i = 0; i < num_vertices_in_facet; i++ )
            out << "      vertex " << facet_vertices[i][0] << " " << facet_vertices[i][1] << " " << facet_vertices[i][2] << "\n";

        out << "    endloop\n";
        out << "  endfacet\n";

        UF_CALL( ::UF_FACET_cycle_facets( tag_faceted_model, &facet_id ) );
    }

    out << "endsolid my_solid\n";

    // parameters
    const QString DefaultName = "model.tri.mesh.stl";
    QSettings qs;
//    QString path = qs.applicationName() + QDir::separator() + DefaultName;
    QString path = "C:\\Users\\Joel\\Desktop\\" + DefaultName;

    // save stl
    std::ofstream file;
    file.open( path.toStdString(), std::ios::out );

    if ( !file.good() )
    {
        // problems with file
        file.close();
        return 2;
    }

    file << out.str();
    file.close();

    return 0;
}

//void NXExtensionImpl::writeSTL( const std::vector<double>& vertices, const std::string& filename )
//{
//    std::ofstream file;
//    file.open( filename.c_str() );

//    file << "solid myobject\n";

//    const int numTriangles = vertices.size() / 9;

//    for ( int i = 0; i < numTriangles; ++i )
//    {
//        const int j = i * 9;

//        const double nx = ( vertices[j + 1] - vertices[j] ) * ( vertices[j + 5] - vertices[j + 2] ) -
//                          ( vertices[j + 2] - vertices[j] ) * ( vertices[j + 4] - vertices[j + 1] );
//        const double ny = ( vertices[j + 2] - vertices[j] ) * ( vertices[j + 3] - vertices[j + 1] ) -
//                          ( vertices[j + 3] - vertices[j] ) * ( vertices[j + 2] - vertices[j + 1] );
//        const double nz = ( vertices[j + 3] - vertices[j] ) * ( vertices[j + 7] - vertices[j + 4] ) -
//                          ( vertices[j + 4] - vertices[j] ) * ( vertices[j + 6] - vertices[j + 3] );

//        const double len = std::sqrt( nx * nx + ny * ny + nz * nz );

//        const double nnx = nx / len;
//        const double nny = ny / len;
//        const double nnz = nz / len;

//        file << "facet normal " << nnx << " " << nny << " " << nnz << "\n";
//        file << "  outer loop\n";
//        file << "    vertex " << vertices[j] << " " << vertices[j + 1] << " " << vertices[j + 2] << "\n";
//        file << "    vertex " << vertices[j + 3] << " " << vertices[j + 4] << " " << vertices[j + 5] << "\n";
//        file << "    vertex " << vertices[j + 6] << " " << vertices[j + 7] << " " << vertices[j + 8] << "\n";
//        file << "  endloop\n";
//        file << "endfacet\n";
//    }

//    file << "endsolid myobject\n";

//    file.close();
//}
