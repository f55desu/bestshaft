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
