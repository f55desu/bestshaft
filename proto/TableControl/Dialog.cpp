#include "windows.h"
#include "Dialog.h"
#include "SettingsDialog.h"

#include <QDebug>
#include <QThread>

#define IDM_SETTINGS 0x0010

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    HMENU hMenu = ::GetSystemMenu(( HWND )winId(), FALSE);
    if (hMenu != NULL)
    {
        ::InsertMenuA(hMenu, 0, MF_BYPOSITION|MF_SEPARATOR, 0, nullptr);
        ::InsertMenuA(hMenu, 0, MF_BYPOSITION|MF_STRING, IDM_SETTINGS, qPrintable(tr("&Settings...")));
    }

//    std::generate(m_model.begin(), m_model.end(),
//                  [n = 1] () mutable { n++; return std::pair(std::format("Variant #{}",n),static_cast<double>(n));} );
    tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_tableModel = new DataModel(this);
    tableView->setModel(m_tableModel);

    tableWidget->setProperty("alreadyCalculated", false);
    connect( tableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( onMultiplySelection() ) );
//    m_tableModel->AddVariant(DataModel::Params{std::pair("Par1",0.111),std::pair("VonMises",.0)});
//    m_tableModel->AddVariant(DataModel::Params{std::pair("Par1",0.111),std::pair("Par2",0.222),std::pair("VonMises",.0)});
//    m_tableModel->AddVariant(DataModel::Params{std::pair("Par2",0.222),std::pair("VonMises",.0)});
//    m_tableModel->AddVariant(DataModel::Params{std::pair("VonMises",.0)});

    //tableView->setShowGrid(false);
//    tableView->horizontalHeader()->show();
//    tableView->verticalHeader()->show();
//    tableView->horizontalHeader()->setMinimumSectionSize(1);
//    tableView->verticalHeader()->setMinimumSectionSize(1);

    on_pushButton_clicked();
}

Dialog::~Dialog()
{
}

int Dialog::get_col_id(const QString& name)
{
    for ( int i = 1; i < tableWidget->columnCount() - 1; i++ )
    {
        if ( tableWidget->horizontalHeaderItem(i)->text() == name )
            return i;
    }
    return -1;
}

void Dialog::on_pushButton_clicked()
{
    /*QObject obj;
    obj.setProperty("colid",123);
    obj.setProperty("colname","namneco");
    int colid = obj.property("colid").toInt();
    QString name = obj.property("colname").toString();

    std::pair<QObject, double>(obj,0.1);

    std::vector<std::pair<QObject, double>> d;
    d.insert(std::pair<QObject, double>(obj,0.1));*/

    std::vector<std::pair<QString, double>> difference_table_model {std::pair("Col1",0.111)};
    std::vector<std::pair<QString, double>> difference_model_table {std::pair("Col4",0.444),std::pair("Col5",0.555)};
    std::vector<std::pair<QString, double>> intersection{std::pair("Col0",0.111),std::pair("Col1",0.222),std::pair("Col2",0.333)};

    for ( const auto& i : difference_model_table )
    {
        int insertedColumnId = tableWidget->columnCount() - 1;
        tableWidget->insertColumn( insertedColumnId );
        tableWidget->setHorizontalHeaderItem(insertedColumnId, new QTableWidgetItem(i.first));

        for ( int i = 0; i < tableWidget->rowCount(); i++ )
        {
            QTableWidgetItem* item = new QTableWidgetItem( "—");
            item->setFlags( item->flags() & ~Qt::ItemIsEditable );
            tableWidget->setItem( i, insertedColumnId, item);
        }
    }

    tableWidget->insertRow(tableWidget->rowCount());
    int insertedRowId = tableWidget->rowCount() - 1;
    tableWidget->setItem( insertedRowId, 0,
                          new QTableWidgetItem( QString( "Variant #%1" ).arg( tableWidget->rowCount() + 1 ) ) );

    for ( const auto& it : difference_table_model )
    {
        QTableWidgetItem* item = new QTableWidgetItem( "—" );
        item->setFlags( item->flags() & ~Qt::ItemIsEditable );
        tableWidget->setItem( insertedRowId, get_col_id(it.first), item );
    }

    for ( const auto& it : intersection )
        tableWidget->setItem( insertedRowId, get_col_id(it.first),
                              new QTableWidgetItem( QString( "%1" ).arg( it.second ) ) );

    for ( const auto& it : difference_model_table )
        tableWidget->setItem( insertedRowId, get_col_id(it.first),
                              new QTableWidgetItem( QString( "%1" ).arg( it.second ) ) );

    QTableWidgetItem* vonmises_item = new QTableWidgetItem( "—" );
    vonmises_item->setFlags( vonmises_item->flags() & ~Qt::ItemIsEditable );
    tableWidget->setItem( insertedRowId, tableWidget->columnCount() - 1, vonmises_item );
}

void Dialog::startTetgen( int selectedItemId )
{
    m_currentProcess = new QProcess(this);
    connect( m_currentProcess, &QProcess::finished, this, &Dialog::solveEnd );
    connect(m_currentProcess, &QProcess::errorOccurred, [=](QProcess::ProcessError error)
    {
        qDebug() << "error enum val = " << error;
    });
    m_currentProcess->setProcessChannelMode(QProcess::MergedChannels);

    m_currentProcess->start("notepad.exe");

//    QThread::msleep(1000);
//    qDebug() << m_currentProcess->state();
//    qDebug() << m_currentProcess->error();

    if ( !m_currentProcess->waitForStarted() && m_currentProcess->error() != 5 )
        emit on_solve_stop( m_currentProcess->error());
}

void Dialog::solveEnd( int exitCode, QProcess::ExitStatus exitStatus)
{
    srand( time( NULL ) );
    double someValue = static_cast<double>( rand() ) / RAND_MAX;

    tableWidget->item(0, 0)->setText(QString::number(someValue));
    // Disable further calculation
    if (!calculatedVariants.contains(0))
        calculatedVariants.append(0);

    emit on_solve_stop( exitCode/*no error*/ );
    disconnect( solveButton, SIGNAL(&QPushButton::clicked), this, SLOT(&Dialog::on_cancelButton_clicked) );
    connect( solveButton, SIGNAL(&QPushButton::clicked), this, SLOT(&Dialog::on_solveButton_clicked) );
    solveButton->setText( "&Solve" );
    QApplication::restoreOverrideCursor();
}

void Dialog::on_solve_stop( int error, ... )
{
    if (error)
        qDebug() << QString("Tetgen return %1 error code").arg(error);
    // activate interface
}

void Dialog::on_cancelButton_clicked()
{
    if (m_currentProcess->state() == QProcess::Running)
        m_currentProcess->terminate();
    QPushButton *button = (QPushButton*)sender();
    button->setText( "&Solve" );
    button->disconnect();
    connect( button, &QPushButton::clicked, this, &Dialog::on_solveButton_clicked);
    QApplication::restoreOverrideCursor();
}

void Dialog::on_solveButton_clicked()
{
    QApplication::setOverrideCursor( Qt::BusyCursor );
    //deactivate all interface
    m_tmpName = solveButton->text();
    QPushButton *button = (QPushButton*)sender();
    button->setText( "&Cancel" );
    button->disconnect();
    connect( button, &QPushButton::clicked, this, &Dialog::on_cancelButton_clicked);

    //disconnect(solveButton, SIGNAL(&QPushButton::clicked), nullptr, nullptr);

    //disconnect( solveButton, &QPushButton::clicked, this, &Dialog::on_solveButton_clicked );
    //connect( solveButton, &QPushButton::clicked, this, &Dialog::on_cancelButton_clicked);

    QApplication::processEvents();

    emit startTetgen( 0 );
}
void Dialog::onMultiplySelection()
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

    for (const int var : calculatedVariants)
    {
        if (selectedRows.contains(var))
        {
            tableWidget->setProperty("alreadyCalculated", true);
            break;
        }
    }
    solveButton->setEnabled( selectedRows.count() >= 1 && !tableWidget->property("alreadyCalculated").toBool());
    tableWidget->setProperty("alreadyCalculated", false);
}

void Dialog::on_pushButton_2_clicked()
{
    DataModel::Params params{std::pair("Par3",0.3),std::pair("Par4",0.4),std::pair("Par2",0.222),std::pair("VonMises",.0)};
    m_tableModel->AddVariant(params);
}

bool Dialog::nativeEvent( const QByteArray& eventType, void* message, qintptr* result )
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
