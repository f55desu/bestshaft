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

            BaseExtension::Variant::const_iterator it1 = variant1.constBegin();
            BaseExtension::Variant::const_iterator it2 = variant2.constBegin();

            BaseExtension::Variant intersection;

            while (it1 != variant1.constEnd() && it2 != variant2.constEnd())
            {
                    if (it1.key() < it2.key()) {
                        it1++;
                    } else if (it2.key() < it1.key()) {
                        it2++;
                    }
                    else
                    {
                        intersection.insert(it1.key(), it1.value());
                        it1++;
                        it2++;
                    }
            }

            BaseExtension::Variant notInIntersection;

            for (auto it1 = variant1.constBegin(); it1 != variant1.constEnd(); ++it1)
            {
                if (!intersection.contains(it1.key()))
                {
                    notInIntersection.insert(it1.key(), it1.value());
                }
            }

            std::list<QString> notInIntersectionHeaders;
            std::list<double> notInIntersectionValues;

            for (auto it1 = variant1.constBegin(); it1 != variant1.constEnd(); ++it1)
            {
                if (!intersection.contains(it1.key()))
                {
                    notInIntersectionHeaders.push_back(it1.key());
                    notInIntersectionValues.push_back(it1.value());
                }
            }

            std::list<QString> headersList;

            // Take current headerList
            for (int i = 0; i < tableWidget->columnCount(); i++)
            {
                headersList.push_back(tableWidget->takeHorizontalHeaderItem(i)->text());
            }

            auto it = headersList.begin();

            std::advance(it, tableWidget->columnCount()); // advance iterator to the penultimate element
            headersList.insert(it, notInIntersectionHeaders.begin(), notInIntersectionHeaders.end()); // Append parameters that are not in intersection

            QStringList headerStringList;
            std::copy(headersList.begin(), headersList.end(), std::back_inserter(headerStringList));
            tableWidget->setHorizontalHeaderLabels( headerStringList ); // Table headers

            for (const auto &var : intersection.keys())
            {
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(intersection[var]));
                for (int i = 0; i < tableWidget->columnCount(); i++)
                {
                    // Find the column to which the value belongs
                    if (tableWidget->horizontalHeaderItem(i)->text() == var)
                    {
                        tableWidget->setItem(rowCount, i, item);
                    }
                }
            }

            for (const auto &val : notInIntersection.keys())
            {
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(notInIntersection[val]));
                for (int i = 0; i < tableWidget->columnCount(); i++)
                {
                    // Find the column to which the value belongs
                    if (tableWidget->horizontalHeaderItem(i)->text() == val && i!=0 && i!=tableWidget->columnCount()-1)
                    {
                        tableWidget->setItem(rowCount, i, item);
                        for(int row = 0; row < tableWidget->rowCount()-1; row++)
                        {
                            QTableWidgetItem *blank = new QTableWidgetItem("—");
                            tableWidget->setItem(row, i, blank);
                        }
                    }
                }
            }

            QTableWidgetItem *item = new QTableWidgetItem("—");
            item->setFlags(item->flags() & !Qt::ItemIsEditable); // Set flag to be non-editable
            qDebug() << "Columns: " << tableWidget->columnCount();
            tableWidget->setItem(rowCount, tableWidget->columnCount()-1, item); // Cтавится прочерк у Von Mises.
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

//    if(tableWidget->selectionModel()->selectedRows().count() == 1)
//    {
//        BaseExtension::Variant variant;

//        int rowCount = tableWidget->rowCount();
//        tableWidget->insertRow( rowCount );
//        QTableWidgetItem *id = new QTableWidgetItem("Variant #" + QString::number(rowCount+1));
//        tableWidget->setItem(rowCount, 0, id);

//        int selectedRowId = tableWidget->selectionModel()->selectedRows().first().row();

//        for (int i = 1; i < tableWidget->columnCount()-1; i++)
//        {
//            // Takeout table information and add to variant
//            variant.insert(tableWidget->horizontalHeaderItem(i)->text(), tableWidget->item(selectedRowId, i)->text().toDouble());
//        }

//        int index = 0;
//        for (const auto &var : variant.keys())
//        {
//            QTableWidgetItem *item = new QTableWidgetItem(QString::number(variant[var]));
//            tableWidget->setItem(rowCount, index+1, item);
//            ++index;
//        }

//        QTableWidgetItem *item = new QTableWidgetItem("—");
//        item->setFlags(item->flags() & !Qt::ItemIsEditable); // Set flag to be non-editable
//        tableWidget->setItem(rowCount, index+1, item); // Cтавится прочерк у Von Mises.
//    }
//    else
//    {
//        initilizeVariant();
//    }
}

void ExtensionWindow::initilizeVariant()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

    int colCountOld = tableWidget->columnCount();

    BaseExtension::Variant variant = m_extension->ExtractVariant();

    tableWidget->setColumnCount( variant.count() + 2); // Variant columns + VarCol + VonMisesCol

    if (colCountOld != tableWidget->columnCount())
    {

    }

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

    int index = 1;
    for (const auto &var : variant.keys())
    {
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(variant[var]));
        tableWidget->setItem(rowCount, index, item);
        ++index;
    }
    QTableWidgetItem *item = new QTableWidgetItem("—");
    item->setFlags(item->flags() & !Qt::ItemIsEditable); // Set flag to be non-editable
    tableWidget->setItem(rowCount, index, item); // Cтавится прочерк у Von Mises.

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

    boldRow(selectedRowId, tableWidget);

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
