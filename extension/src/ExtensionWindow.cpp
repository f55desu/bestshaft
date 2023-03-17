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
    calculateButton->setToolTip("Calculate selected variant(s) Von Mises");
    deleteButton->setToolTip("Delete selected variant(s)");
    applyButton->setToolTip("Apply selected variant to the current model");
    addButton->setToolTip("Add a copy of selected variant or add a current variant applied to model");
    paraviewButton->setToolTip("Open in ParaView selected variant(s)");

    tableWidget->setSortingEnabled(true);
    tableWidget->sortByColumn(tableWidget->columnCount()-1, Qt::DescendingOrder);

    currentVariantId = 0;
    on_addButton_clicked();
    boldRow(currentVariantId, tableWidget);
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
    BaseExtension::Variant variant1 = m_extension->ExtractVariant();
    BaseExtension::Variant variant2;

    int rowCount = tableWidget->rowCount();


    int selectedRowId = -1;
    if (rowCount > 0)
    {
        try {
            selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();
        } catch (...) {
            selectedRowId = -1;
        }
        if (selectedRowId > -1) // Variable has been re-initilized, row was selected
        {
            tableWidget->insertRow( rowCount );
            QTableWidgetItem *id = new QTableWidgetItem("Variant #" + QString::number(rowCount+1));
            tableWidget->setItem(rowCount, 0, id);

            for (int i = 1; i < tableWidget->columnCount()-1; i++)
            {
                // Takeout table information and add to variant
                variant2.insert(tableWidget->horizontalHeaderItem(i)->text(), tableWidget->item(selectedRowId, i)->text().toDouble());
            }

            std::map<QString, double> stdmap1;
            for (auto it = variant1.constBegin(); it != variant1.constEnd(); ++it)
            {
                stdmap1.insert(std::make_pair(it.key(), it.value()));
            }
            std::map<QString, double> stdmap2;
            for (auto it = variant2.constBegin(); it != variant2.constEnd(); ++it)
            {
                stdmap2.insert(std::make_pair(it.key(), it.value()));
            }

            auto compare = [](auto& x, auto& y) {
                return x.first < y.first;
            };

            // Setting an intersection between model and seleceted variants
            std::vector<std::pair<QString, double>> intersection;

            std::set_intersection(stdmap1.begin(), stdmap1.end(),
                                  stdmap2.begin(), stdmap2.end(),
                                  std::back_inserter(intersection), compare);

            // Setting difference between model and selected variants
            std::vector<std::pair<QString, double>> differenceV1V2;

            std::set_difference(stdmap1.begin(), stdmap1.end(),
                                stdmap2.begin(), stdmap2.end(),
                                std::back_inserter(differenceV1V2), compare);

            // Setting difference between selected and model variants
            std::vector<std::pair<QString, double>> differenceV2V1;

            std::set_difference(stdmap2.begin(), stdmap2.end(),
                                stdmap1.begin(), stdmap1.end(),
                                std::back_inserter(differenceV2V1), compare);

            BaseExtension::Variant variantFinal;

            QStringList headers;
            headers.append("Var\Par");
            for (int i = 0; i < intersection.size(); i++)
            {
                headers.append(intersection[i].first);
            }
            headers.append("Von Mises");

            if (!differenceV1V2.empty())
            {
                // Add columns
                for (int i = 0; i < differenceV1V2.size(); i++)
                {
                    headers.insert(headers.size()-1, differenceV1V2[i].first);
                }
                tableWidget->setColumnCount(headers.count());
                tableWidget->setHorizontalHeaderLabels( headers );
                for (int i = 0; i < differenceV1V2.size(); i++)
                {
                    for (int j = 0; j < tableWidget->columnCount(); j++)
                    {
                        if(tableWidget->horizontalHeaderItem(j)->text() == differenceV1V2[i].first)
                        {
                            variantFinal.insert(differenceV1V2[i].first, differenceV1V2[i].second);
                            for (int k = 0; k < tableWidget->rowCount()-1; k++)
                            {
                                QTableWidgetItem *dash = new QTableWidgetItem("—");
                                tableWidget->setItem(k, j, dash);
                            }
                        }
                    }
                }
            }

            if (!differenceV2V1.empty())
            {
                // Add a '-' into row
                for (int i = 0; i < differenceV2V1.size(); i++)
                {
                    headers.insert(headers.size()-1, differenceV2V1[i].first);
                }
                tableWidget->setColumnCount(headers.count());
                tableWidget->setHorizontalHeaderLabels( headers );

                for (int i = 0; i < differenceV2V1.size(); i++)
                {
                    for (int j = 0; j < tableWidget->columnCount(); j++)
                    {
                        if(tableWidget->horizontalHeaderItem(j)->text() == differenceV2V1[i].first)
                        {
                            for (int k = 0; k < tableWidget->rowCount()-1; k++)
                            {
                                QTableWidgetItem *dash = new QTableWidgetItem("—");
                                tableWidget->setItem(k, j, dash);
                            }
                        }
                    }
                }
            }

            tableWidget->setColumnCount(headers.count());
            tableWidget->setHorizontalHeaderLabels( headers );

            for (int i = 0; i < intersection.size(); i++)
            {
                for(int j = 0; j < tableWidget->columnCount(); j++)
                {
                    if (tableWidget->horizontalHeaderItem(j)->text() == intersection[i].first)
                    {
                        variantFinal.insert(intersection[i].first, intersection[i].second);
                    }
                }
            }

            m_extension->variants.append(variantFinal);
            updateTableRows(m_extension->variants);

            for (int i = 0; i < tableWidget->rowCount()+1; ++i)
            {
                if (!tableWidget->item(i, tableWidget->columnCount()-1))
                {
                    QTableWidgetItem *vonmises = new QTableWidgetItem("—");
                    vonmises->setFlags(vonmises->flags() & !Qt::ItemIsEditable); // Set flag to be non-editable
                    tableWidget->setItem(i, tableWidget->columnCount()-1, vonmises); // Cтавится прочерк у Von Mises.
                }
            }
        }
        else
        {
            initilizeVariant();
        }
    }
    else
    {
        initilizeVariant();
    }
}

void ExtensionWindow::initilizeVariant()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

    BaseExtension::Variant variant = m_extension->ExtractVariant();
    m_extension->variants.append(variant);

    tableWidget->setColumnCount( variant.count() + 2); // Variant columns + VarCol + VonMisesCol

    QStringList headersList;

    if(tableWidget->columnCount() < 1)
    {
        for (int i = 0; i < tableWidget->columnCount(); i++)
        {
            headersList.append(tableWidget->horizontalHeaderItem(i)->text());
        }

        bool inHeadersList = false;
        for (const auto &var : variant.keys())
        {
            for (int i = 0; i < headersList.count(); i++)
            {
                if (headersList[i] == var)
                {
                    inHeadersList = true;
                }
            }

            if(!inHeadersList)
            {
                headersList.insert(headersList.count()-2, var);
            }
        }
    }
    else
    {
        headersList.append("Var\\Par");
        for (const auto &var : variant.keys())
        {
            headersList.append(QString(var));
        }
        headersList.append("Von Mises");
    }

    tableWidget->setHorizontalHeaderLabels( headersList ); // Table headers
    tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow( rowCount );
    QTableWidgetItem *id = new QTableWidgetItem("Variant #" + QString::number(rowCount+1));
    tableWidget->setItem(rowCount, 0, id);
    for (const auto &var : variant.keys())
    {
        for (int i = 0; i < tableWidget->columnCount(); i++)
        {
            if (tableWidget->horizontalHeaderItem(i)->text() == var)
            {
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(variant[var]));
                tableWidget->setItem(rowCount, i, item);
            }
        }
    }
    QTableWidgetItem *item = new QTableWidgetItem("—");
    item->setFlags(item->flags() & !Qt::ItemIsEditable); // Set flag to be non-editable
    tableWidget->setItem(rowCount, tableWidget->columnCount()-1, item); // Cтавится прочерк у Von Mises.

    QApplication::restoreOverrideCursor();
}

void ExtensionWindow::on_applyButton_clicked()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    applyButton->setEnabled(false);
    QApplication::processEvents();

    BaseExtension::Variant variant;
    // Unbold all
    for(int row = 0; row < tableWidget->rowCount(); ++row)
    {
        boldRow(row, tableWidget, false);
    }

    int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

    currentVariantId=selectedRowId;
    boldRow(currentVariantId, tableWidget);

    for (int i = 1; i < tableWidget->columnCount()-1; i++)
    {
        // Takeout table information and add to variant
        variant.insert(tableWidget->horizontalHeaderItem(i)->text(), tableWidget->item(selectedRowId, i)->text().toDouble());
    }

    m_extension->ApplyVariant(variant);

    applyButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}


void ExtensionWindow::on_calculateButton_clicked()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    calculateButton->setEnabled(false);
    QApplication::processEvents();

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

    calculateButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}


void ExtensionWindow::on_paraviewButton_clicked()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

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
    // Fill the vector with unique rows
    for (QTableWidgetItem *item : selectedItems)
    {
        row = item->row();
        if (!rowsToDelete.contains(row))
        {
            rowsToDelete.append(row);
        }
    }
    // Sort the rows Id's
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());
    // Delete them
    for (int row : rowsToDelete)
    {
        tableWidget->removeRow(row);
    }

    if(tableWidget->rowCount()<1)
        addButton->setEnabled(true);
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

    paraviewButton->setEnabled(selectedRows.count() >= 1); // Can't open in ParaView zero variants
    calculateButton->setEnabled(selectedRows.count() >= 1); // Can't calculate zero variants
    deleteButton->setEnabled(selectedRows.count() >= 1); // Can't delete zero variants
    addButton->setEnabled(selectedRows.count() <= 1); // Can't copy multiply variants
    applyButton->setEnabled(selectedRows.count() == 1); // Can't apply multiply variants
}

double ExtensionWindow::calculateMaxTension()
{
    srand(time(NULL));
    double random_double = static_cast<double>(rand()) / RAND_MAX;
    return random_double;
}

void ExtensionWindow::updateTableRows(QList<BaseExtension::Variant> variants)
{
    for (int i = 0; i < variants.count(); i++)
    {
        for (int row = 0; row < tableWidget->rowCount(); row++)
        {
            BaseExtension::Variant variant = variants[i];
            for (auto it = variant.begin(); it != variant.end(); it++)
            {
                for (int col = 0; col < tableWidget->columnCount(); col++)
                {
                    if (tableWidget->horizontalHeaderItem(col)->text() == it.key() && i == row)
                    {
                        qDebug() << "Variant #" << i << ": \n" <<
                                 it.key() << it.value() << "\n";

                        QTableWidgetItem *item = new QTableWidgetItem(QString::number(it.value()));
                        tableWidget->setItem(row, col, item);
                    }
                }
            }
        }
    }
    boldRow(currentVariantId, tableWidget);
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
