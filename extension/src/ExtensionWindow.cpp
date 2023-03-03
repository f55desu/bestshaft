#include "Stable.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"

ExtensionWindow::ExtensionWindow( QWidget* parent, BaseExtension* ext ) :
    QDialog( parent ), m_extension( ext )
{
    setupUi( this );
    installEventFilter( this );

    // Selecting one row at once
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Multiply selection of rows with ctrl
    tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onMultiplySelection()));

    on_addButton_clicked();
    boldRow(0, tableWidget);
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
//    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows ); // Selecting one row at once
//    tableWidget->setSelectionMode( QAbstractItemView::MultiSelection ); // Multiply selection of rows

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
    QTableWidgetItem *id = new QTableWidgetItem("Variant #" + QString::number(rowCount+1));
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
    BaseExtension::Variant variant;
    // Unbold all
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        boldRow(row, tableWidget, false);
    }

    int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

    boldRow(selectedRowId, tableWidget);

    int columnsCount = tableWidget->columnCount();
    for (int i = 1; i < columnsCount; i++)
    {
        // Takeout table information and add to variant
        variant.insert(tableWidget->horizontalHeaderItem(i)->text(), tableWidget->item(selectedRowId, i)->text().toDouble());
    }

    m_extension->ApplyVariant(variant);
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

// Buttons to be disabled if action cannot be performed
void ExtensionWindow::onMultiplySelection()
{
    // Get the selected ranges
    QList<QTableWidgetSelectionRange> ranges = tableWidget->selectedRanges();

    // Count the unique rows within the selected ranges
    QSet<int> selectedRows;
    foreach (QTableWidgetSelectionRange range, ranges)
    {
        for (int row = range.topRow(); row <= range.bottomRow(); ++row)
        {
            selectedRows.insert(row);
        }
    }

    applyButton->setEnabled(selectedRows.count() == 1); // Can't apply multiply variants
}

double ExtensionWindow::calculateMaxTension()
{
    srand(time(NULL));
    double random_double = static_cast<double>(rand()) / RAND_MAX;
    return random_double;
}

// Bold specific row
void ExtensionWindow::boldRow(int rowId, QTableWidget* tableWidget, bool bold)
{
    for (int column = 0; column < tableWidget->columnCount(); ++column)
    {
        QTableWidgetItem *item = tableWidget->item(rowId, column);
        QFont font = item->font();
        font.setBold(bold);
        item->setFont(font);
    }
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
