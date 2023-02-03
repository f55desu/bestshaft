#include "Stable.h"
#include "NXExtensionImpl.h"

NXExtensionImpl NXExtensionImpl::m_instance;
HHOOK NXExtensionImpl::m_hHook = 0x0;

UF_MB_action_t NXExtensionImpl::m_actionTable[] =
{
    {( char* )"RUN_EDITOR_ACTION", &NXExtensionImpl::RunEditorAction, 0x0},
    {( char* )"ABOUT_ACTION", &NXExtensionImpl::AboutAction, 0x0},
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

void NXExtensionImpl::CheckContextIsValid()
{
    if ( ::UF_ASSEM_ask_work_part() == NULL_TAG )
        m_scriptEngine->throwError(
            QString( "Work part doesn't exist. Create new or open existing part model." ) );
}

void NXExtensionImpl::CheckFirstSolidInPart()
{
    if ( !firstSolidFlag )
    {
        tag_t tag_solid = NULL_TAG;;

        ::UF_MODL_ask_object( UF_solid_type, UF_solid_body_subtype, &tag_solid );

        if ( tag_solid != NULL_TAG )
            firstSolidFlag = true;
    }
}

void NXExtensionImpl::CheckPostprocess()
{
    globalTagPart = ::UF_PART_ask_display_part();

    if ( globalTagPart == NULL_TAG )
        ::UF_PART_new( "model", 1 /*metric*/, &globalTagPart );

    firstSolidFlag = false;
    CheckFirstSolidInPart();

    if ( firstSolidFlag )
        m_scriptEngine->throwError( QString( "Model has already exist." ) );
}
