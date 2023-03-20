#include "Dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
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

