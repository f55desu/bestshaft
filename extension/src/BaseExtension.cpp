#include "Stable.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"
#include "BestShaftWindow.h"
#include "IAbstractModeler.h"

#include "../version.h"

int BaseExtension::m_modalLoopCount = 0;
Category& BaseExtension::m_logger = Category::getRoot();

const QString BaseExtension::IniFileName = "extension.ini";
const QString BaseExtension::LogFileName = "extension.log";
const QString BaseExtension::HomePathEnvName = "PARATRAN_HOME_PATH";

BaseExtension::BaseExtension() :
    m_topWindow( 0x0 ), m_extensionWindow( 0x0 ), m_bestShaftWindow( 0x0 ), m_scriptEngine( 0x0 )
{
}

void BaseExtension::SetTopWindow( QWidget* topWindow )
{
    Q_CHECK_PTR( topWindow );
    m_topWindow = topWindow;
}

Category& BaseExtension::GetLogger()
{
    return BaseExtension::m_logger;
}

void BaseExtension::RunEditor()
{
    InitQt();

    //Show modeless dialog
    if ( m_bestShaftWindow == 0x0 )
    {
//        m_extensionWindow = new ExtensionWindow( GetTopWindow(), this );
//        m_extensionWindow->setAttribute( Qt::WA_DeleteOnClose );
//        m_extensionWindow->show();
        m_bestShaftWindow = new BestShaftWindow( GetTopWindow(), this );
        m_bestShaftWindow->setAttribute( Qt::WA_DeleteOnClose );
        m_bestShaftWindow->show();
    }
    else
        m_bestShaftWindow->activateWindow();
}
void BaseExtension::About()
{
    InitQt();

    //Show modal dialog
    EnterModalLoop();
    ExtensionWindow* w = new ExtensionWindow( GetTopWindow(), this );
    w->setAttribute( Qt::WA_DeleteOnClose );
    w->show();
    ExitModalLoop();
}

void BaseExtension::InitQt()
{
    if ( qApp )
        return;

    Q_INIT_RESOURCE( resources );

    //Read options from ini file
    QSettings settings( m_iniFileName, QSettings::IniFormat );
    QString arglist = settings.value( "QTAPP/ARGV", "-style=plastique" ).toString();

    //Parse options to convert into command line string
    stringstream buf;
    buf << qPrintable( arglist );
    csv_parser cmd_parser( buf, ',', 0 );
    cmd_parser.next();
    csv_parser::row_type args;
    cmd_parser.get_row( args );
    args.insert( args.begin(), "" );

    //Convert command line options into native arguments of QApplication
    int offset = 0;
    string rawcmd;
    rawcmd.reserve( arglist.size() + 1 );
    vector<char*> argv;
    argv.reserve( args.size() );

    for ( csv_parser::row_type::const_iterator i = args.begin();
            i != args.end(); ++i )
    {
        rawcmd += *i + '\0';
        argv.push_back( &rawcmd[offset] );
        offset = ( int )rawcmd.size();
    }

    //Create Qt BaseExtension functionality
    int argc = ( int )args.size();
    ( void )new QApplication( argc, &argv[0] );

    InitScriptEngine();
}

void BaseExtension::Initialize()
{
    QString logFileName;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    if ( env.contains( HomePathEnvName ) )
    {
        m_homePath = env.value( HomePathEnvName );

        //Check if last char is separator
        if ( m_homePath.right( 1 ) != QDir::separator() )
            m_homePath.append( QDir::separator() );

        m_iniFileName = m_homePath + IniFileName;
        logFileName = m_homePath + LogFileName;
    }
    else
    {
        //Set current home path by default (in win7 it's equal to C:\Users\<user name>)
        m_homePath = QDir::home().path() + QDir::separator();
        m_iniFileName = m_homePath + IniFileName;
        logFileName = m_homePath + LogFileName;
    }

    //Default logger options
    RollingFileAppender* rollingAppender =
        new RollingFileAppender( "FileAppender", qPrintable( logFileName ), 65536, 3 );
    PatternLayout* layout = new PatternLayout();
    layout->setConversionPattern( "%d{ISO8601} %p - %m%n" );
    rollingAppender->setLayout( layout );
    m_logger.setAdditivity( false );
    m_logger.setAppender( rollingAppender );
    m_logger.setPriority( Priority::DEBUG );

    //Start writing to log file
    m_logger.info( QString( "------------ Start" +
                            QString( " (v%1.%2.%3_R%4) " ).arg( MAJOR ).arg( MINOR ).arg( PATCH ).arg( REVISION )
                            + "------------" ).toStdString() );

    if ( !env.contains( HomePathEnvName ) )
        m_logger.warn( HomePathEnvName.toStdString() + " environment variable isn't set." );

    QFileInfo iniFile( m_iniFileName );

    if ( !iniFile.exists() )
        m_logger.warn( qPrintable( QString(
                                       "File \"%1\" doesn't exist. Default options are applied." ).arg( m_iniFileName ) ) );
}

void BaseExtension::TermQt()
{
    if ( qApp )
    {
        qApp->quit();
        delete qApp;
    }

    m_logger.info( "Exit." );

    //Terminate logger
    Category::shutdown();
}

void BaseExtension::InitScriptEngine()
{
    m_scriptEngine = new QJSEngine( this );
    Q_CHECK_PTR( m_scriptEngine );

    m_scriptEngine->installExtensions( QJSEngine::AllExtensions );

    QJSValue globalObject = m_scriptEngine->globalObject();

    QJSValue scriptModeler = m_scriptEngine->newQObject( this );
    globalObject.setProperty( "Modeler", scriptModeler );

    //m_scriptContext = m_scriptEngine->currentContext();
    /*QJSValue globalAxisOX = QJSValue(m_scriptEngine,"globalAxisOX");
    globalObject.setProperty("AXIS_OX",globalAxisOX);*/

    /*QJSValue scriptFunction = m_scriptEngine->newFunction(TEST_Function);
    globalObject.setProperty("TEST_Function",scriptFunction);*/
}

void BaseExtension::EnterModalLoop()
{
    ++m_modalLoopCount;
}

void BaseExtension::ExitModalLoop()
{
    --m_modalLoopCount;
    Q_ASSERT( m_modalLoopCount >= 0 );
}

bool BaseExtension::GetModalState()
{
    return m_modalLoopCount > 0;
}

QString BaseExtension::GetHomePath() const
{
    return m_homePath;
}
