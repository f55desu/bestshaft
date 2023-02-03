#include "BestShaftWindow.h"
#include "BaseExtension.h"

BestShaftWindow::BestShaftWindow(QWidget *parent, BaseExtension* ext) :
    QMainWindow(parent), m_extension( ext )
{
    //ui->setupUi(this);
    setupUi(this);
    tableWidget->setColumnCount(5); // 5 columns
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // Selecting one row at once
    tableWidget->setSelectionMode(QAbstractItemView::MultiSelection); // Multiply selection of rows
    QStringList headersList = { "Var\Par", "R1", "R2", "Fi", "Von Mises" };

    tableWidget->setHorizontalHeaderLabels(headersList); // Table headers
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow(rowCount);

    installEventFilter( this );
}

BestShaftWindow::~BestShaftWindow()
{
    if ( !m_extension->GetModalState() )
        m_extension->m_bestShaftWindow = NULL; //Reset extension window reference
}

void BestShaftWindow::showEvent(QShowEvent *e)
{
    QWidget::showEvent( e );

    raise();
    activateWindow();
}

void BestShaftWindow::closeEvent(QCloseEvent *e)
{
    e->accept();
}

void BestShaftWindow::on_actionExit_triggered()
{
    close();
}

void BestShaftWindow::on_addButton_clicked()
{
    int rowCount = tableWidget->rowCount();
    tableWidget->insertRow(rowCount);
}


void BestShaftWindow::on_applyButton_clicked()
{

}


void BestShaftWindow::on_calculateButton_clicked()
{

}


void BestShaftWindow::on_paraviewButton_clicked()
{

}


void BestShaftWindow::on_deleteButton_clicked()
{

}

