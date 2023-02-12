#include "Stable.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"

ExtensionWindow::ExtensionWindow( QWidget* parent, BaseExtension* ext ) :
    QDialog( parent ), m_extension( ext )
{
    setupUi( this );
    installEventFilter( this );

    on_addButton_clicked();
}

ExtensionWindow::~ExtensionWindow()
{
    writeSettings();

    if ( !m_extension->GetModalState() )
        m_extension->m_extensionWindow = NULL; //Reset extension window reference
}

void ExtensionWindow::readSettings()
{
    QSettings m_settings( m_extension->GetHomePath() + BaseExtension::IniFileName,
                          QSettings::IniFormat );

    m_settings.beginGroup( "GENERAL" );
    QRect rect = m_settings.value( "GEOMETRY", geometry() ).toRect();
    setGeometry( rect );
    m_settings.endGroup();

    m_firstShowFlag = false;
}

void ExtensionWindow::writeSettings()
{
    QSettings m_settings( m_extension->GetHomePath() + BaseExtension::IniFileName,
                          QSettings::IniFormat );

    m_settings.beginGroup( "GENERAL" );
    m_settings.setValue( "GEOMETRY", geometry() );
    m_settings.endGroup();
}

void ExtensionWindow::showEvent( QShowEvent* e )
{
    QWidget::showEvent( e );

    if ( m_firstShowFlag )
        readSettings();
    else
    {
        raise();
        activateWindow();
    }
}

void ExtensionWindow::closeEvent( QCloseEvent* event )
{
//    if ( mayBeSave() )
//        event->accept();
//    else
//        event->ignore();
}

void ExtensionWindow::on_actionExit_triggered()
{
    close();
}

void ExtensionWindow::on_addButton_clicked()
{
    BaseExtension::Variant variant = m_extension->ExtractVariant();

    tableWidget->setColumnCount( 5 ); // 5 columns
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows ); // Selecting one row at once
    tableWidget->setSelectionMode( QAbstractItemView::MultiSelection ); // Multiply selection of rows
    QStringList headersList = { "Var\\Par", "R1", "R2", "Fi", "Von Mises" };

    tableWidget->setHorizontalHeaderLabels( headersList ); // Table headers
    tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow( rowCount );
}


void ExtensionWindow::on_applyButton_clicked()
{

}


void ExtensionWindow::on_calculateButton_clicked()
{

}


void ExtensionWindow::on_paraviewButton_clicked()
{

}


void ExtensionWindow::on_deleteButton_clicked()
{

}

bool ExtensionWindow::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == this )
    {
        if ( e->type() == QEvent::Enter )
        {
            setWindowOpacity( 1.0 );
            return true;
        }
        else if ( e->type() == QEvent::Leave && !geometry().contains( QCursor::pos() ) )
        {
            setWindowOpacity( 0.1 );
            return true;
        }
        else if ( e->type() == QEvent::Hide )
        {
            setWindowOpacity( 1.0 );
            return true;
        }
        else
            return false;
    }
    else
        return ExtensionWindow::eventFilter( obj, e );
}
