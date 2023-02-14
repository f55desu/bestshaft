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

    tableWidget->setColumnCount( variant.count() + 2); // Variant columns + VarCol + VonMisesCol
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows ); // Selecting one row at once
    tableWidget->setSelectionMode( QAbstractItemView::MultiSelection ); // Multiply selection of rows

    QStringList headersList;

    headersList.append("Var\\Par");
    for (const auto &var : variant.keys())
    {
        headersList.append(QString(var));
    }
    headersList.append("Von Mises");

    tableWidget->setHorizontalHeaderLabels( headersList ); // Table headers
    tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow( rowCount );
    QTableWidgetItem *id = new QTableWidgetItem(QString::number(rowCount+1));
    tableWidget->setItem(rowCount, 0, id);

    int index = 0;
    for (const auto &var : variant.keys())
    {
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(variant[var]));
        tableWidget->setItem(rowCount, index+1, item);
        ++index;
    }
    QTableWidgetItem *item = new QTableWidgetItem("—");
    tableWidget->setItem(rowCount, index+1, item); // Cтавится прочерк у Von Mises.
}


void ExtensionWindow::on_applyButton_clicked()
{

}


void ExtensionWindow::on_calculateButton_clicked()
{
    auto selectedRows = tableWidget->selectionModel()->selectedRows();

    for (int i = 0; i < selectedRows.count(); i++)
    {
        double someValue = 0.0;
        someValue = calculateMaxTension();

        int colCount = tableWidget->columnCount();
        int selectedRow = selectedRows[i].row();
        QTableWidgetItem *selectedItem;
        selectedItem = new QTableWidgetItem(QString::number(someValue));

        tableWidget->setItem(selectedRow, colCount - 1, selectedItem); // Set the Von Misses Col
    }
}


void ExtensionWindow::on_paraviewButton_clicked()
{

}


void ExtensionWindow::on_deleteButton_clicked()
{
    QModelIndexList selection = tableWidget->selectionModel()->selectedRows();

    for (int i = 0; i < selection.count(); i++)
    {
        int row = selection.at(i).row();
        tableWidget->removeRow(row);
    }
}

double ExtensionWindow::calculateMaxTension()
{
    srand(time(NULL));
    double random_double = static_cast<double>(rand()) / RAND_MAX;
    return random_double;
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
