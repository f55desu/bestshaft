#include "Stable.h"
#include "windows.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"
#include "SettingsDialog.h"

#define IDM_SETTINGS 0x0010

ExtensionWindow::ExtensionWindow( QWidget* parent, BaseExtension* ext ) :
    QDialog( parent ), m_extension( ext )
{
    setupUi( this );
    HMENU hMenu = ::GetSystemMenu(( HWND )winId(), FALSE);
    if (hMenu != NULL)
    {
        ::InsertMenuA(hMenu, 0, MF_BYPOSITION|MF_SEPARATOR, 0, nullptr);
        ::InsertMenuA(hMenu, 0, MF_BYPOSITION|MF_STRING, IDM_SETTINGS, qPrintable(tr("&Settings...")));
    }
    installEventFilter( this );

    // Selecting one row at once
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
    // Multiply selection of rows with ctrl
    tableWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( tableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( onMultiplySelection() ) );
    connect( tableWidget, &QTableWidget::cellChanged, this, &ExtensionWindow::on_cellChanged );
    connect( tableWidget, &QTableWidget::cellEntered, this, &ExtensionWindow::on_cellEntered );
    calculateButton->setToolTip( "Calculate selected variant(s) Von Mises" );
    deleteButton->setToolTip( "Delete selected variant(s)" );
    applyButton->setToolTip( "Apply selected variant to the current model" );
    addButton->setToolTip( "Add a copy of selected variant or add a current variant applied to model" );
    paraviewButton->setToolTip( "Open in ParaView selected variant(s)" );

    tableWidget->setSortingEnabled( true );
    tableWidget->sortByColumn( tableWidget->columnCount() - 1, Qt::DescendingOrder );

    currentVariantId = 0;
    tableWidget->setProperty( "alreadyCalculated", false );
    on_addButton_clicked();
    tableWidget->selectRow( 0 ); // by default select the first row
}

ExtensionWindow::~ExtensionWindow()
{
    writeSettings();

    if ( !m_extension->GetModalState() )
        m_extension->m_extensionWindow = NULL; //Reset extension window reference
}

bool ExtensionWindow::nativeEvent( const QByteArray& eventType, void* message, qintptr* result )
{
    if (eventType == "windows_generic_MSG")
    {
         MSG* m = reinterpret_cast<MSG *>(message);
         if (m->message == WM_SYSCOMMAND)
         {
           if ((m->wParam & 0xfff0) == IDM_SETTINGS)
           {
              *result = 0;
               SettingsDialog* dialog = new SettingsDialog(this);
               dialog->open();
               return true;
           }
         }
    }

   return false;
}

void ExtensionWindow::readSettings()
{
    QSettings m_settings( m_extension->GetHomePath() + BaseExtension::IniFileName,
                          QSettings::IniFormat );

    m_settings.beginGroup( "GENERAL" );
    QRect rect = m_settings.value( "GEOMETRY", geometry() ).toRect();
    setGeometry( rect );
    m_settings.endGroup();

    m_settings.beginGroup( "MISC" );
    if (!m_settings.value("WORKSPACEPATH").isNull())
        m_extension->bestshaft_workspace_path = m_settings.value("WORKSPACEPATH").toString();
    if (!m_settings.value("PARAVIEWPATH").isNull())
        m_extension->bestshaft_paraview_path = m_settings.value("PARAVIEWPATH").toString();
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
    m_settings.beginGroup( "MISC" );
    m_settings.setValue( "WORKSPACEPATH", m_extension->bestshaft_workspace_path);
    m_settings.setValue( "PARAVIEWPATH", m_extension->bestshaft_paraview_path);
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

    // Saving all variants to part
    QMap<QString, BaseExtension::Variant> variantsToSave;
    for (int row = 0; row < tableWidget->rowCount(); row++)
    {
        BaseExtension::Variant variantToSave;

        // Excluding variant name and von mises
        for (int col = 1; col < tableWidget->columnCount(); col++)
        {
            if (tableWidget->horizontalHeaderItem(col)->text() == "Von Mises" && tableWidget->item(row, col)->text() == "—")
                variantToSave.insert(tableWidget->horizontalHeaderItem(col)->text(), 0.0);
            else {
                variantToSave.insert(tableWidget->horizontalHeaderItem(col)->text(), tableWidget->item(row, col)->text().toDouble());
            }
        }

        variantsToSave.insert(tableWidget->item(row, 0)->text(), variantToSave);
    }
    m_extension->WriteVariants(variantsToSave);
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
    if ( tableWidget->property( "tmpEnteredCellValue" ).toString() != tableWidget->item( row, column )->text() )
    {
        applyButton->setEnabled( true );
        tableWidget->setProperty( "tmpEnteredCellValue", -1 );
    }

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

void ExtensionWindow::on_cellEntered ( int row, int column )
{
    if ( row != currentVariantId )
        return;

    if ( row == currentVariantId )
        tableWidget->setProperty( "tmpEnteredCellValue", tableWidget->item( row, column )->text() );
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

//    BaseExtension::Variant variantToSave;

//    for (int i = 1; i < tableWidget->columnCount()-1; i++)
//    {
//        variantToSave.insert(tableWidget->horizontalHeaderItem(i)->text(), tableWidget->item(rowCount, i)->text().toDouble());
//    }

    m_extension->variants.append( variant );
}

void ExtensionWindow::initilizeVariant()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    BaseExtension::Variant variant = m_extension->ExtractVariant();

    QMap<QString, BaseExtension::Variant> savedVariants;

    m_extension->ReadVariants(savedVariants);

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

    // First insert saved variants
    if (!savedVariants.empty())
    {
        for (const auto& var : savedVariants.keys())
        {
            int rowCount = tableWidget->rowCount();
            tableWidget->insertRow( rowCount );
            QTableWidgetItem* variantName = new QTableWidgetItem( var );
            tableWidget->setItem( rowCount, 0, variantName );

            for (const auto& attrName : savedVariants[var].keys())
            {
                for ( int i = 0; i < tableWidget->columnCount(); i++ )
                {
                    if ( tableWidget->horizontalHeaderItem( i )->text() == attrName )
                    {
                        // If there is a Von Mises disable any further editing
                        if (savedVariants[var][QString("Von Mises")] != 0.0)
                        {
                            QTableWidgetItem* item = new QTableWidgetItem( QString::number( savedVariants[var][attrName] ) );
                            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                            tableWidget->setItem( rowCount, i, item );
                            calculatedVariants.append(rowCount);
                        }
                        else {
                            QTableWidgetItem* item = new QTableWidgetItem( QString::number( savedVariants[var][attrName] ) );
                            tableWidget->setItem( rowCount, i, item );
                        }
                    }
                    if (tableWidget->horizontalHeaderItem( i )->text() == attrName && attrName == "Von Mises")
                    {
                        QTableWidgetItem* vonmises_item;
                        if (savedVariants[var][attrName] == 0.0)
                        {
                            vonmises_item = new QTableWidgetItem( "—" );
                        }
                        else {
                            vonmises_item = new QTableWidgetItem( QString::number( savedVariants[var][attrName] ) );
                            tableWidget->item(rowCount, 0)->setFlags(tableWidget->item(rowCount, 0)->flags() & ~Qt::ItemIsEditable);
                        }
                        vonmises_item->setFlags( vonmises_item->flags() & ~Qt::ItemIsEditable ); // Set flag to be non-editable
                        tableWidget->setItem( rowCount, i, vonmises_item );
                    }
                }
            }
        }

        for (const auto& var : savedVariants)
        {
            m_extension->variants.append(var);
        }
    }
    else {
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
    }

    // by default after loading saved variants the next inserted row is applied
    boldRow( currentVariantId, tableWidget );

    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::on_applyButton_clicked()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
    applyButton->setEnabled( false );
    QApplication::processEvents();

    BaseExtension::Variant variant;

    // Unbold all
    for ( int row = 0; row < tableWidget->rowCount(); ++row )
        boldRow( row, tableWidget, false );

    int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

    currentVariantId = selectedRowId;
    boldRow( currentVariantId, tableWidget );

    for ( int i = 1; i < tableWidget->columnCount() - 1; i++ )
    {
        // Takeout table information and add to variant
        variant.insert( tableWidget->horizontalHeaderItem( i )->text(), tableWidget->item( selectedRowId,
                                                                                           i )->text().toDouble() );
    }

    m_extension->ApplyVariant( variant );

    onMultiplySelection();
    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::startSolve()
{
    const QString default_mesh_file_name = "model.tri.mesh";
    const QString default_calculix_input_file_name = "abaqus.ccx";

    const QString user_default_path = QDir::homePath();

    QString variant_name = tableWidget->item( m_currentIndex, 0 )->text();
    std::replace( variant_name.begin(), variant_name.end(), ' ', '_' );

    try
    {
        // create workspace and variant folder if not exist
        QDir user_dir( user_default_path );

        // workspace folder not exists
        if ( !user_dir.exists( m_extension->bestshaft_workspace_folder_name ) )
        {
            // failed to create folder
            if ( !user_dir.mkdir( m_extension->bestshaft_workspace_folder_name ) )
                throw std::exception( ( "Cannot create folder: " + m_extension->bestshaft_workspace_folder_name.toStdString() ).c_str() );

            // folder created successfuly
        }

        //QString bestshaft_workspace_path = user_default_path + QDir::separator() + m_extension->bestshaft_workspace_folder_name;
        QDir bestshaft_workspace_dir( m_extension->bestshaft_workspace_path );

        // variant folder not exists
        if ( !bestshaft_workspace_dir.exists( variant_name ) )
        {
            // failed to create folder
            if ( !bestshaft_workspace_dir.mkdir( variant_name ) )
                throw std::exception( ( "Cannot create folder: " + variant_name.toStdString() ).c_str() );

            // folder created successfuly
        }

        // create mesh files paths
        const QString wavefront_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                            QDir::separator() + default_mesh_file_name + ".obj";
        const QString stl_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                      QDir::separator() + default_mesh_file_name + ".stl";
        const QString tetgen_input_poly_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                                    QDir::separator() + default_mesh_file_name + ".poly";
        const QString tetgen_input_smesh_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                                     QDir::separator() + default_mesh_file_name + ".smesh";
        const QString tetgen_input_mtr_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                                   QDir::separator() + default_mesh_file_name + ".mtr";
        const QString gmsh_msh_file_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name +
                                           QDir::separator() + default_mesh_file_name + ".msh";

        double max_facet_size = -1;

        // save .obj, .stl, .poly, .smesh and .mtr files
        m_extension->SaveMeshDatabase( wavefront_file_path,
                                       stl_file_path,
                                       tetgen_input_poly_file_path,
                                       tetgen_input_smesh_file_path,
                                       tetgen_input_mtr_file_path,
                                       gmsh_msh_file_path,
                                       max_facet_size );

        const QString bestshaft_home_path = QProcessEnvironment::systemEnvironment().value( "BESTSHAFT_HOME_PATH" );
        const QString run_script_path = bestshaft_home_path + QDir::separator() + "run.bat";
        const QString bestshaft_workspace_variant_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name;

        const QString tetgen_node_path = bestshaft_workspace_variant_path + QDir::separator() + default_mesh_file_name +
                                         ".1.node";
        const QString tetgen_face_path = bestshaft_workspace_variant_path + QDir::separator() + default_mesh_file_name +
                                         ".1.face";
        const QString tetgen_ele_path = bestshaft_workspace_variant_path + QDir::separator() + default_mesh_file_name +
                                        ".1.ele";

        const QString tet2inp_ccx_path = bestshaft_workspace_variant_path + QDir::separator() + default_calculix_input_file_name
                                         + ".inp";

        const QString ccx_inp_path_without_extension = bestshaft_workspace_variant_path + QDir::separator() +
                                                       default_calculix_input_file_name;
        const QString discLetter(m_extension->bestshaft_workspace_path.at(0));

        m_currentProcess = new QProcess(this);
        connect( m_currentProcess, &QProcess::finished, this, &ExtensionWindow::solveEnd );
        m_currentProcess->setProcessChannelMode( QProcess::SeparateChannels );

        // cmd.exe process did not terminate itself after executing
        //m_currentProcess->startDetached("cmd.exe", QStringList() << "/c" << "start /b cmd /c echo Hello && taskkill /f /im cmd.exe", QDir::rootPath(), nullptr);
        m_currentProcess->start( run_script_path,
                                 ( QStringList() <<
                                   bestshaft_workspace_variant_path <<
                                   bestshaft_home_path <<
                                   QString( "%1" ).arg( max_facet_size ) <<
                                   tetgen_node_path <<
                                   tetgen_face_path <<
                                   tetgen_ele_path <<
                                   tet2inp_ccx_path <<
                                   variant_name <<
                                   ccx_inp_path_without_extension <<
                                   discLetter)
                               );

//        qDebug() << "run.bat: " << run_script_path << bestshaft_workspace_variant_path << bestshaft_home_path <<
//                 QString( "%1" ).arg(
//                     max_facet_size ) << tetgen_node_path << tetgen_face_path << tetgen_ele_path << tet2inp_ccx_path << variant_name <<
//                 ccx_inp_path_without_extension;

    }
    catch ( const std::runtime_error& ex )
    {
        BaseExtension::GetLogger().error( ex.what() );
    }
    catch ( const std::exception& ex )
    {
        BaseExtension::GetLogger().error( ex.what() );
    }

    if ( !m_currentProcess->waitForStarted() && m_currentProcess->error() != 5 )
        emit on_solve_stop( m_currentProcess->error() );

    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::solveEnd( int exitCode, QProcess::ExitStatus /*exitStatus*/ )
{
    double someValue = -1;

    try
    {
        // Change to universal variable
        const QString user_default_path = QDir::homePath(),
                      bestshaft_workspace_folder_name = "BestshaftWorkspace",
                      default_calculix_input_file_name = "abaqus.ccx";

        QString variant_name = tableWidget->item( m_currentIndex, 0 )->text();
        std::replace( variant_name.begin(), variant_name.end(), ' ', '_' );

        const QString ccx_dat_path = user_default_path + QDir::separator() +
                                     bestshaft_workspace_folder_name + QDir::separator() +
                                     variant_name + QDir::separator() +
                                     default_calculix_input_file_name + ".dat";

        someValue = calculateMaxTension( ccx_dat_path );
    }
    catch ( const std::runtime_error& ex )
    {
        BaseExtension::GetLogger().error( ex.what() );
        QMessageBox msgBox;
        msgBox.setText( ex.what() );
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.setWindowTitle( QString( "BestShaft" ) );
        msgBox.setParent( this ); // Set parent to current widget
        msgBox.setWindowModality( Qt::WindowModal );
        msgBox.setStandardButtons( QMessageBox::Ok );
        msgBox.exec();
    }

    if ( exitCode )
        goto label_end;

    if (someValue > -1)
    {
        tableWidget->item( m_currentIndex, tableWidget->columnCount() - 1 )->setText( QString::number(
                                                                                              someValue ) ); // Set the Von Mises
        // Disable further calculation
        if ( !calculatedVariants.contains( currentVariantId ) )
            calculatedVariants.append( currentVariantId );

        //disable and unselect all cells in m_currentIndex row
        for ( int col = 0; col < tableWidget->columnCount(); col++ )
        {
            tableWidget->item( m_currentIndex, col )->setSelected( false );
            tableWidget->item( m_currentIndex, col )->setFlags( tableWidget->item( m_currentIndex,
                                                                                   col )->flags() & ~Qt::ItemIsEditable );
        }

        // Deselect current row
        for ( int column = 0; column < tableWidget->columnCount(); column++ )
        {
            QModelIndex index = tableWidget->selectionModel()->model()->index( m_currentIndex, column );
            tableWidget->selectionModel()->select( index, QItemSelectionModel::Deselect );
        }
    }

    m_rowsToBeProceed = tableWidget->selectionModel()->selectedRows();

    if ( !m_rowsToBeProceed.empty() )
    {
        m_currentIndex = m_rowsToBeProceed.cbegin()->row();
        emit solveStart( );
        return;
    }

label_end:
    emit on_solve_stop( exitCode/*no error*/ );
}

void ExtensionWindow::on_solve_stop( int error, ... )
{
    if ( error )
        BaseExtension::GetLogger().error( QString( "Something error: %1" ).arg( error ).toStdString() );

    // return all to the begining state
    calculateButton->setText( calculateButton->property( "tmpName" ).toString() );
    disconnect( calculateButton, &QPushButton::clicked, this, &ExtensionWindow::on_cancelButton_clicked );
    connect( calculateButton, &QPushButton::clicked, this, &ExtensionWindow::on_calculateButton_clicked );
    tableWidget->selectRow( tableWidget->property( "tmpCurrentVariantId" ).toInt() );
    on_applyButton_clicked();
    m_rowsToBeProceed.clear(); // Condition for addButton to be active
    // activate interface
    paraviewButton->setEnabled( true );
    deleteButton->setEnabled( true );
    addButton->setEnabled( true );
    applyButton->setEnabled( true );
    tableWidget->setEnabled( true );
    QApplication::processEvents();
}

void ExtensionWindow::on_cancelButton_clicked()
{
    if ( m_currentProcess->state() == QProcess::Running )
        m_currentProcess->kill();

    QPushButton* button = ( QPushButton* )sender();
    button->setText( button->property( "tmpName" ).toString() );
    button->disconnect();
    connect( button, &QPushButton::clicked, this, &ExtensionWindow::on_calculateButton_clicked );
    QApplication::restoreOverrideCursor();

    // restore applied variant
    if ( currentVariantId != tableWidget->property( "tmpCurrentVariantId" ).toInt() )
    {
        tableWidget->selectionModel()->clearSelection();
        tableWidget->selectRow( tableWidget->property( "tmpCurrentVariantId" ).toInt() );
        on_applyButton_clicked();
    }
    m_rowsToBeProceed.clear(); // Condition for addButton to be active

    // activate interface
    paraviewButton->setEnabled( true );
    deleteButton->setEnabled( true );
    addButton->setEnabled( true );
    applyButton->setEnabled( true );
    tableWidget->setEnabled( true );
}

void ExtensionWindow::on_calculateButton_clicked()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
    //deactivate all interface
    QPushButton* button = ( QPushButton* )sender();
    button->setProperty( "tmpName", button->text() );
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

    // save constant current variant
    tableWidget->setProperty( "tmpCurrentVariantId", currentVariantId );
    // before starting solving, first need to apply variant
    on_applyButton_clicked();
    emit startSolve();
}
void ExtensionWindow::solveStart( )
{
    if ( currentVariantId != m_currentIndex || currentVariantId == -1 )
        on_applyButton_clicked();

    emit startSolve( );
}

void ExtensionWindow::on_paraviewButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
    QVector<int> rowsToParaView;
    int tempRow = 0;

    // Fill the vector with unique rows
    for ( QTableWidgetItem* item : selectedItems )
    {
        tempRow = item->row();

        if ( !rowsToParaView.contains( tempRow ) )
            rowsToParaView.append( tempRow );
    }

    // paraview code

    for (auto& row : rowsToParaView)
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );

        QString variant_name = tableWidget->item(row, 0)->text();
        std::replace( variant_name.begin(), variant_name.end(), ' ', '_' );

        const QString bestshaft_workspace_variant_path = m_extension->bestshaft_workspace_path + QDir::separator() + variant_name;
        const QString abaqus_vtk_file = bestshaft_workspace_variant_path + QDir::separator() + QString("abaqus.ccx.vtk");

        // Check if vtk file is exists
        QFileInfo fileInfo(abaqus_vtk_file);
        if (!fileInfo.exists())
        {
            QApplication::restoreOverrideCursor();

            BaseExtension::GetLogger().error(QString("The Abaqus VTK file does not exist for \"%1\"").arg(variant_name).toStdString());

            QMessageBox msgBox;
            msgBox.setText( QString("The Abaqus VTK file does not exist for \"%1\"").arg(variant_name));
            msgBox.setIcon( QMessageBox::Warning );
            msgBox.setWindowTitle( QString( "BestShaft" ) );
            msgBox.setParent( this ); // Set parent to current widget
            msgBox.setWindowModality( Qt::WindowModal );
            msgBox.setStandardButtons( QMessageBox::Ok );
            msgBox.exec();
            continue;
        }

        // Check if is path to ParaView is specified
        if (m_extension->bestshaft_paraview_path.isEmpty())
        {
            QApplication::restoreOverrideCursor();
            BaseExtension::GetLogger().error(QString("Path to ParaView is not specified. Specify the path to ParaView in the settings.").arg(variant_name).toStdString());

            QMessageBox msgBox;
            msgBox.setText( QString("Path to ParaView is not specified. Specify the path to ParaView in the settings."));
            msgBox.setIcon( QMessageBox::Information );
            msgBox.setWindowTitle( QString( "BestShaft" ) );
            msgBox.setParent( this ); // Set parent to current widget
            msgBox.setWindowModality( Qt::WindowModal );
            msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.exec();

            break;
        }

        // Check if paraview.exe is exists in path to ParaView
        QFileInfo paraviewFileInfo(m_extension->bestshaft_paraview_path+QDir::separator()+"paraview.exe");
        if ( !paraviewFileInfo.exists() )
        {
            QApplication::restoreOverrideCursor();
            BaseExtension::GetLogger().error(("\""+m_extension->bestshaft_paraview_path+QDir::separator()+"paraview.exe"+"\" does not exists.").toStdString());
            QMessageBox msgBox;
            msgBox.setText( QString("\""+m_extension->bestshaft_paraview_path+QDir::separator()+"paraview.exe"+"\" does not exists. Specify another right path."));
            msgBox.setIcon( QMessageBox::Information );
            msgBox.setWindowTitle( QString( "BestShaft" ) );
            msgBox.setParent( this ); // Set parent to current widget
            msgBox.setWindowModality( Qt::WindowModal );
            msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.exec();

            break;
        }

        try
        {
            m_currentProcess = new QProcess(this);
            m_currentProcess->setProcessChannelMode( QProcess::SeparateChannels );
            m_currentProcess->start(m_extension->bestshaft_paraview_path+QDir::separator()+"paraview.exe",
                                    QStringList() << "--data=" << abaqus_vtk_file);
        }
        catch (const std::runtime_error& ex )
        {
            BaseExtension::GetLogger().error(ex.what());
        }
        catch ( const std::exception& ex )
        {
            BaseExtension::GetLogger().error( ex.what() );
        }
        QApplication::restoreOverrideCursor();
    }
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
    QList<int> selectedRows;

    foreach ( QTableWidgetSelectionRange range, ranges )
    {
        for ( int row = range.topRow(); row <= range.bottomRow(); ++row )
            selectedRows.append(row);
    }

    for ( const int var : calculatedVariants )
    {
        if ( selectedRows.contains( var ) )
        {
            tableWidget->setProperty( "alreadyCalculated", true );
            break;
        }
    }

    // Can't open in ParaView zero and not calculated variants
    paraviewButton->setEnabled( selectedRows.count() >= 1 && tableWidget->property( "alreadyCalculated" ).toBool());
    // Can't calculate zero and already calculated variants
    calculateButton->setEnabled( selectedRows.count() >= 1 && !tableWidget->property( "alreadyCalculated" ).toBool() );
    tableWidget->setProperty( "alreadyCalculated", false );
    // Can't delete zero variants and can't delete applied variant
    deleteButton->setEnabled( selectedRows.count() >= 1 && !selectedRows.contains( currentVariantId ) );
    // Can't copy multiply variants
    addButton->setEnabled( selectedRows.count() == 1 && m_rowsToBeProceed.empty());
    // Can't apply multiply variants
    applyButton->setEnabled( selectedRows.count() == 1 && !selectedRows.contains( currentVariantId ) );
}

double ExtensionWindow::calculateMaxTension( const QString& ccx_dat_filepath )
{
#if 0
    srand( time( NULL ) );
    double random_double = static_cast<double>( rand() ) / RAND_MAX;
    return random_double;
#else
    std::ifstream dat( ccx_dat_filepath.toStdString() );

    if ( !dat.is_open() )
        throw std::runtime_error( "Cannot open file " + ccx_dat_filepath.toStdString() );

    std::string line;
    std::getline( dat, line );

    int elem, integ_pnt;
    double Sxx, Syy, Szz, Sxy, Sxz, Syz;

    double von_mises_max = std::numeric_limits<double>::min(),
           von_mises;

    while ( std::getline( dat, line ) )
        if ( sscanf_s( line.c_str(), "%d %d %lf %lf %lf %lf %lf %lf",
                       &elem, &integ_pnt, &Sxx, &Syy, &Szz, &Sxy, &Sxz, &Syz ) == 8 )
        {
            von_mises = 0.5 * ( ::pow( Sxx - Syy, 2.0 ) + ::pow( Syy - Szz, 2.0 ) + ::pow( Szz - Sxx, 2.0 ) );
            von_mises += 3.0 * ( Sxy * Sxy + Sxz * Sxz + Syz * Syz );
            von_mises = ::sqrt( von_mises );

            if ( von_mises > von_mises_max )
                von_mises_max = von_mises;

        }

    dat.close();
    return von_mises_max;
#endif
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
