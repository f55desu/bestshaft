#include "ExtensionImpl.h"
#include "MainWindow.h"

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent )
{
    setupUi( this );
}

void MainWindow::on_actionRunExtension_triggered()
{
    ExtensionImpl::Instance().RunEditor();
}
