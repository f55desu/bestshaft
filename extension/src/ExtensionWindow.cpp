#include "Stable.h"

#include "BaseExtension.h"
#include "ExtensionWindow.h"

ExtensionWindow::ExtensionWindow( QWidget* parent, BaseExtension* ext ) :
    QDialog( parent ), m_extension( ext )
{
    setupUi( this );
    installEventFilter( this );

    // Selecting one row at once
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
    // Multiply selection of rows with ctrl
    tableWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( tableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( onMultiplySelection() ) );
    connect( tableWidget, &QTableWidget::cellChanged, this, &ExtensionWindow::on_cellChanged );
    calculateButton->setToolTip( "Calculate selected variant(s) Von Mises" );
    deleteButton->setToolTip( "Delete selected variant(s)" );
    applyButton->setToolTip( "Apply selected variant to the current model" );
    addButton->setToolTip( "Add a copy of selected variant or add a current variant applied to model" );
    paraviewButton->setToolTip( "Open in ParaView selected variant(s)" );

    tableWidget->setSortingEnabled( true );
    tableWidget->sortByColumn( tableWidget->columnCount() - 1, Qt::DescendingOrder );

    currentVariantId = 0;
    on_addButton_clicked();
    boldRow( currentVariantId, tableWidget ); // by default first row is applied
    tableWidget->setProperty("changedRowId", -1); // set a property that nothing is changed yet
    tableWidget->selectRow(0); // by default select the first row
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
    Q_UNUSED( event );
//    if ( mayBeSave() )
//        event->accept();
//    else
//        event->ignore();
}

void ExtensionWindow::on_actionExit_triggered()
{
    close();
}

int ExtensionWindow::GetColumnId( const QString& name )
{
    for ( int i = 1; i < tableWidget->columnCount() - 1; i++ )
    {
        if ( tableWidget->horizontalHeaderItem( i )->text() == name )
            return i;
    }

    return -1;
}

QString ExtensionWindow::GenerateVariantName()
{
    QString name;

    int j = 1, i;

    goto label_start;

    while ( i < tableWidget->rowCount() )
    {
        if ( tableWidget->item( i, 0 ) && tableWidget->item( i, 0 )->text() == name )
        {
label_start:
            name = QString( "Variant #%1" ).arg( j++ );
            i = 0;
            continue;
        }

        i++;
    }

    return name;
}

void ExtensionWindow::on_cellChanged( int row, int column )
{
    tableWidget->setProperty("changedRowId", row);
    onMultiplySelection();
    if ( column != 0 )
        return;

    QRegularExpression re( "^[^\\/:*?\"<>|]*$" );
    QRegularExpressionMatch match;

    if ( tableWidget->item( row, 0 ) && !( match = re.match( tableWidget->item( row, 0 )->text() ) ).hasMatch() )
    {
        tableWidget->item( row, 0 )->setText( GenerateVariantName() );
        return;
    }

    for ( int i = 0; i < tableWidget->rowCount(); i++ )
        if ( i != row && tableWidget->item( i, 0 )->text() == tableWidget->item( row, 0 )->text() )
        {
            tableWidget->item( row, 0 )->setText( GenerateVariantName() );
            break;
        }
}

void ExtensionWindow::on_addButton_clicked()
{
    Q_ASSERT( tableWidget->rowCount() > 0 && tableWidget->selectionModel()->selectedRows().size() == 1 );

    int rowCount = tableWidget->rowCount();

    if ( rowCount == 0 )
    {
        initilizeVariant();
        return;
    }

    int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

    std::map<QString, double> variant_from_table;

    for ( int i = 1; i < tableWidget->columnCount() - 1; i++ )
    {
        // Takeout table information and add to variant
        variant_from_table.insert( std::pair( tableWidget->horizontalHeaderItem( i )->text(), tableWidget->item( selectedRowId,
                                                                                                                 i )->text().toDouble() ) );
    }

    BaseExtension::Variant variant = m_extension->ExtractVariant();

    std::map<QString, double> variant_from_model( variant.toStdMap() );

    auto compare = []( auto & x, auto & y )
    {
        return x.first < y.first;
    };

    // Setting an intersection between model and seleceted variants
    std::vector<std::pair<QString, double>> intersection;

    std::set_intersection( variant_from_model.begin(), variant_from_model.end(),
                           variant_from_table.begin(), variant_from_table.end(),
                           std::back_inserter( intersection ), compare );

    // Setting difference between model and selected variants
    std::vector<std::pair<QString, double>> difference_model_table;

    std::set_difference( variant_from_model.begin(), variant_from_model.end(),
                         variant_from_table.begin(), variant_from_table.end(),
                         std::back_inserter( difference_model_table ), compare );

    // Setting difference between selected and model variants
    std::vector<std::pair<QString, double>> difference_table_model;

    std::set_difference( variant_from_table.begin(), variant_from_table.end(),
                         variant_from_model.begin(), variant_from_model.end(),
                         std::back_inserter( difference_table_model ), compare );

    for ( const auto& i : difference_model_table )
    {
        int insertedColumnId = tableWidget->columnCount() - 1;
        tableWidget->insertColumn( insertedColumnId );
        tableWidget->setHorizontalHeaderItem( insertedColumnId, new QTableWidgetItem( i.first ) );

        for ( int i = 0; i < tableWidget->rowCount(); i++ )
        {
            QTableWidgetItem* item = new QTableWidgetItem( "—" );
            item->setFlags( item->flags() & ~Qt::ItemIsEditable );
            tableWidget->setItem( i, insertedColumnId, item );
        }
    }

    tableWidget->insertRow( tableWidget->rowCount() );

    int insertedRowId = tableWidget->rowCount() - 1;

    tableWidget->setItem( insertedRowId, 0,
                          new QTableWidgetItem( GenerateVariantName() ) );

    for ( const auto& it : difference_table_model )
    {
        QTableWidgetItem* item = new QTableWidgetItem( "—" );
        item->setFlags( item->flags() & ~Qt::ItemIsEditable );
        tableWidget->setItem( insertedRowId, GetColumnId( it.first ), item );
    }

    for ( const auto& it : intersection )
        tableWidget->setItem( insertedRowId, GetColumnId( it.first ),
                              new QTableWidgetItem( QString( "%1" ).arg( it.second ) ) );

    for ( const auto& it : difference_model_table )
        tableWidget->setItem( insertedRowId, GetColumnId( it.first ),
                              new QTableWidgetItem( QString( "%1" ).arg( it.second ) ) );

    QTableWidgetItem* vonmises_item = new QTableWidgetItem( "—" );
    vonmises_item->setFlags( vonmises_item->flags() & ~Qt::ItemIsEditable );
    tableWidget->setItem( insertedRowId, tableWidget->columnCount() - 1, vonmises_item );

    tableWidget->selectRow( rowCount );

    m_extension->variants.append( variant );
}

void ExtensionWindow::initilizeVariant()
{
    QApplication::setOverrideCursor( Qt::BusyCursor );

    BaseExtension::Variant variant = m_extension->ExtractVariant();
    m_extension->variants.append( variant );

    tableWidget->setColumnCount( variant.count() + 2 ); // Variant columns + VarCol + VonMisesCol

    QStringList headersList;

    if ( tableWidget->columnCount() < 1 )
    {
        for ( int i = 0; i < tableWidget->columnCount(); i++ )
            headersList.append( tableWidget->horizontalHeaderItem( i )->text() );

        bool inHeadersList = false;

        for ( const auto& var : variant.keys() )
        {
            for ( int i = 0; i < headersList.count(); i++ )
            {
                if ( headersList[i] == var )
                    inHeadersList = true;
            }

            if ( !inHeadersList )
                headersList.insert( headersList.count() - 2, var );
        }
    }
    else
    {
        headersList.append( "Var\\Par" );

        for ( const auto& var : variant.keys() )
            headersList.append( QString( var ) );

        headersList.append( "Von Mises" );
    }

    tableWidget->setHorizontalHeaderLabels( headersList ); // Table headers
    tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow( rowCount );
    QTableWidgetItem* id = new QTableWidgetItem( GenerateVariantName() );
    tableWidget->setItem( rowCount, 0, id );

    for ( const auto& var : variant.keys() )
    {
        for ( int i = 0; i < tableWidget->columnCount(); i++ )
        {
            if ( tableWidget->horizontalHeaderItem( i )->text() == var )
            {
                QTableWidgetItem* item = new QTableWidgetItem( QString::number( variant[var] ) );
                tableWidget->setItem( rowCount, i, item );
            }
        }
    }

    QTableWidgetItem* vonmises_item = new QTableWidgetItem( "—" );
    vonmises_item->setFlags( vonmises_item->flags() & ~Qt::ItemIsEditable ); // Set flag to be non-editable
    tableWidget->setItem( rowCount, tableWidget->columnCount() - 1,
                          vonmises_item ); // Cтавится прочерк у Von Mises.

    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::on_applyButton_clicked()
{
    QApplication::setOverrideCursor( Qt::BusyCursor );
    applyButton->setEnabled( false );
    QApplication::processEvents();

    BaseExtension::Variant variant;

    // Unbold all
    for ( int row = 0; row < tableWidget->rowCount(); ++row )
        boldRow( row, tableWidget, false );

    int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

    currentVariantId = selectedRowId;
    boldRow( currentVariantId, tableWidget );
    tableWidget->setProperty("changedRowId", -1);

    for ( int i = 1; i < tableWidget->columnCount() - 1; i++ )
    {
        // Takeout table information and add to variant
        variant.insert( tableWidget->horizontalHeaderItem( i )->text(), tableWidget->item( selectedRowId,
                                                                                           i )->text().toDouble() );
    }

    m_extension->ApplyVariant( variant );

    onMultiplySelection();
    applyButton->setEnabled( true );
    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::startTetgen()
{
    // BaseExtension::Variant variant = m_model[tableWidget->item( i, 0 )->text()];
    // SaveSTL( tableWidget->item( selectedItemId, 0 )->text() );

    double returned_max_facet_size = -1;
    QString returned_file_path = "";

    int error_code = m_extension->SaveSTL( tableWidget->item( m_currentIndex, 0 )->text(), returned_file_path,
                                           returned_max_facet_size );

    if ( error_code /*|| returned_max_facet_size < 0 || returned_file_path == ""*/ )
    {
        // SaveSTL something error
        emit on_solve_stop( ERROR_TYPE_SAVESTL, error_code );
        return;
    }

    QString bestshaft_home_path = QProcessEnvironment::systemEnvironment().value( "BESTSHAFT_HOME_PATH" );
    QString tetgen_path = bestshaft_home_path + QDir::separator() + "tetgen.exe";

    m_currentProcess = new QProcess();
    connect( m_currentProcess, &QProcess::finished, this, &ExtensionWindow::tetgentEnd );

    m_currentProcess->setProcessChannelMode( QProcess::MergedChannels );
    // cmd.exe process did not terminate itself after executing
    //m_currentProcess->startDetached("cmd.exe", QStringList() << "/c" << "start /b cmd /c echo Hello && taskkill /f /im cmd.exe", QDir::rootPath(), nullptr);
    m_currentProcess->start( tetgen_path,
                             QStringList() << QString( "-ka%1" ).arg( returned_max_facet_size ) << returned_file_path );

    // TODO: Bind with tetgen and calculix processes
//    connect( m_currentProcess, &QProcess::finished, this, &ExtensionWindow::startCalculix );
//    m_currentProcess->startDetached( m_extension->GetPluginBinDir() + QDir::separator() + "tetgen.exe",
//                                     QStringList() << QString( "-a%1" ).arg( m_extension->getFacesSize() )
//                                     << m_extension->getWorkspaceDir() + QDir::separator() + tableWidget->item( i, 0 )->text()
//                                     + QDir::separator() + "mesh.stl" );

    if ( !m_currentProcess->waitForStarted() && m_currentProcess->error() != 5 )
        emit on_solve_stop( ERROR_TYPE_TETGEN, m_currentProcess->error() );
    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::tetgentEnd( int exitCode, QProcess::ExitStatus /*exitStatus*/ )
{
    if ( !exitCode )
        emit startCalculix();
    else
        emit on_solve_stop( 0, exitCode/*no error*/ );
}

void ExtensionWindow::startCalculix()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    // TODO: Bind with tetgen and calculix processes
//    if (exitCode)
//       emit on_solve_stop(exitCode,exitStatus)
//    log m_currentProcess.readAll();
//    m_currentProcess = new QProcess( parent() );
//    connect( m_currentProcess, &QProcess::finished, this, &ExtensionWindow::solveEnd );
//    m_currentProcess->startDetached( m_extension->GetPluginBinDir() + QDir::separator() + "calculix.exe",
//                                     QStringList() << QString( "-a%1" ).arg( m_extension->getFacesSize() )
//                                     << m_extension->getWorkspaceDir() + QDir::separator() + tableWidget->item( i, 0 )->text()
//                                     + QDir::separator() + "mesh.stl" );

    emit calculixEnd(0, QProcess::NormalExit);

//    if ( !m_currentProcess->waitForStarted() )
//        emit on_solve_stop( m_currentProcess->error(), ... );
    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::calculixEnd( int exitCode, QProcess::ExitStatus /*exitStatus*/ )
{
    double someValue = 0;
    if ( exitCode )
        goto label_end;

    someValue = calculateMaxTension();

    tableWidget->item( m_currentIndex, tableWidget->columnCount() - 1 )->setText( QString::number( someValue ) ); // Set the Von Mises

    //disable and unselect all cells in m_currentIndex row
    for (int col = 0; col < tableWidget->columnCount(); col++)
    {
        tableWidget->item(m_currentIndex, col)->setSelected(false);
        tableWidget->item(m_currentIndex, col)->setFlags(tableWidget->item(m_currentIndex, col)->flags() & ~Qt::ItemIsEditable);
    }

    if ( m_rowsToBeProceed.cbegin()->isValid() )
    {
        m_currentIndex = m_rowsToBeProceed.cbegin()->row();
        m_rowsToBeProceed.erase(m_rowsToBeProceed.cbegin());
        emit solveStart( );
        return;
    }

label_end:
    emit on_solve_stop( 0, exitCode/*no error*/ );
}

void ExtensionWindow::on_solve_stop( int type, int error, ... )
{
    if ( error )
    {
        switch ( type )
        {
            case ERROR_TYPE_TETGEN:
                BaseExtension::GetLogger().error(QString( "Tetgen return %1 error code" ).arg( error ).toStdString());
                break;

            case ERROR_TYPE_SAVESTL:
                BaseExtension::GetLogger().error(QString( "SaveSTL method return %1 error code" ).arg( error ).toStdString());
                break;

            default:
                BaseExtension::GetLogger().error(QString( "Undefined error type: %1" ).arg( error ).toStdString());
                break;
        }
    }

    //BaseExtension::m_logger.error(QString("Tetgen return %1 error code").arg(error));
    //
    //messagebox tetgen->error()
    //else
    //
    //        int colCount = tableWidget->columnCount();
    //        int selectedRow = selectedRows[i].row();
    //        QTableWidgetItem* selectedItem;
    //        selectedItem = new QTableWidgetItem( QString::number( someValue ) );

    //        tableWidget->setItem( selectedRow, colCount - 1, selectedItem ); // Set the Von Misses Col

    // return all to the begining state
    calculateButton->setText( calculateButton->property("tmpName").toString() );
    tableWidget->selectRow(tableWidget->property("tmpCurrentVariantId").toInt());
    on_applyButton_clicked();
    // activate interface
    paraviewButton->setEnabled( true );
    deleteButton->setEnabled( true );
    addButton->setEnabled( true );
    applyButton->setEnabled( true );
    tableWidget->setEnabled( true );
}

void ExtensionWindow::on_cancelButton_clicked()
{
    if ( m_currentProcess->state() == QProcess::Running )
        m_currentProcess->kill();

    QPushButton* button = ( QPushButton* )sender();
    button->setText( button->property("tmpName").toString() );
    button->disconnect();
    connect( button, &QPushButton::clicked, this, &ExtensionWindow::on_calculateButton_clicked );
    QApplication::restoreOverrideCursor();

    // restore applied variant
    if (currentVariantId != tableWidget->property("tmpCurrentVariantId").toInt())
    {
        tableWidget->selectionModel()->clearSelection();
        tableWidget->selectRow(tableWidget->property("tmpCurrentVariantId").toInt());
        on_applyButton_clicked();
    }

    // activate interface
    paraviewButton->setEnabled( true );
    deleteButton->setEnabled( true );
    addButton->setEnabled( true );
    applyButton->setEnabled( true );
    tableWidget->setEnabled( true );
}

void ExtensionWindow::on_calculateButton_clicked()
{
    QApplication::setOverrideCursor( Qt::BusyCursor );
    //deactivate all interface
    QPushButton* button = ( QPushButton* )sender();
    button->setProperty("tmpName", button->text());
    button->setText( tr( "&Cancel" ) );
    button->disconnect();
    connect( button, &QPushButton::clicked, this, &ExtensionWindow::on_cancelButton_clicked );

    //Deactivate all except cancel
    paraviewButton->setEnabled( false );
    deleteButton->setEnabled( false );
    addButton->setEnabled( false );
    applyButton->setEnabled( false );
    tableWidget->setEnabled( false );

    QApplication::processEvents();

    m_rowsToBeProceed = tableWidget->selectionModel()->selectedRows();

    m_currentIndex = m_rowsToBeProceed.cbegin()->row();
    m_rowsToBeProceed.erase(m_rowsToBeProceed.cbegin());

    // save constant current variant
    tableWidget->setProperty("tmpCurrentVariantId", currentVariantId);
    // before starting solving, first need to apply variant
    on_applyButton_clicked();
    emit startTetgen();
}
void ExtensionWindow::solveStart( )
{
    if ( currentVariantId != m_currentIndex || currentVariantId == -1 )
        on_applyButton_clicked();

    emit startTetgen( );
}

void ExtensionWindow::on_paraviewButton_clicked()
{
    QApplication::setOverrideCursor( Qt::BusyCursor );

    auto selectedRows = tableWidget->selectionModel()->selectedRows();

    // paraview code

    QApplication::restoreOverrideCursor();
}


void ExtensionWindow::on_deleteButton_clicked()
{
    // Delete all selected rows
    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
    QVector<int> rowsToDelete;
    int row = 0;

    QString message;

    // Fill the vector with unique rows
    for ( QTableWidgetItem* item : selectedItems )
    {
        row = item->row();

        if ( !rowsToDelete.contains( row ) )
            rowsToDelete.append( row );

    }

    if ( !rowsToDelete.empty() && rowsToDelete.size() == 1 )
        message.append( QString( "You are about to delete %1" ).arg( tableWidget->item( rowsToDelete.first(), 0 )->text() ) );
    else
        message.append( QString( "You are about to delete %1 items" ).arg( rowsToDelete.size() ) );

    message.append( "\nDo you want to proceed?" );

    QMessageBox msgBox;
    msgBox.setText( message );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( QString( "BestShaft" ) );
    msgBox.setParent( this ); // Set parent to current widget
    msgBox.setWindowModality( Qt::WindowModal );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    int ret = msgBox.exec();

    if ( ret == QMessageBox::Yes )
    {
        // Sort the rows Id's
        std::sort( rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>() );

        // Delete them
        for ( int row : rowsToDelete )
            tableWidget->removeRow( row );

        if ( tableWidget->rowCount() < 1 )
            addButton->setEnabled( true );
    }
    else if ( ret == QMessageBox::No )
        return;
}

// Buttons to be disabled if action cannot be performed
void ExtensionWindow::onMultiplySelection()
{
    // Get the selected ranges
    QList<QTableWidgetSelectionRange> ranges = tableWidget->selectedRanges();

    // Count the unique rows within the selected ranges
    QSet<int> selectedRows;

    foreach ( QTableWidgetSelectionRange range, ranges )
    {
        for ( int row = range.topRow(); row <= range.bottomRow(); ++row )
            selectedRows.insert( row );
    }

    // Can't open in ParaView zero variants
    paraviewButton->setEnabled( selectedRows.count() >= 1 );
    // Can't calculate zero variants
    calculateButton->setEnabled( selectedRows.count() >= 1 );
    // Can't delete zero variants and can't delete applied variant
    deleteButton->setEnabled( selectedRows.count() >= 1 && !selectedRows.contains( currentVariantId ) );
    // Can't copy multiply variants
    addButton->setEnabled( selectedRows.count() == 1 );
    // Can't apply multiply variants
    int i = tableWidget->property("changedRowId").toInt();
    applyButton->setEnabled( (selectedRows.count() == 1 && !selectedRows.contains( currentVariantId )) ||
                             (selectedRows.contains(tableWidget->property("changedRowId").toInt()) && selectedRows.count() == 1));
}

double ExtensionWindow::calculateMaxTension()
{
    srand( time( NULL ) );
    double random_double = static_cast<double>( rand() ) / RAND_MAX;
    return random_double;
}

// Bold specific row
void ExtensionWindow::boldRow( int rowId, QTableWidget* tableWidget, bool bold )
{
    for ( int column = 0; column < tableWidget->columnCount(); ++column )
    {
        QTableWidgetItem* item = tableWidget->item( rowId, column );
        QFont font = item->font();
        font.setBold( bold );
        item->setFont( font );
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
