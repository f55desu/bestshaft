#include "Stable.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"

ExtensionWindow::ExtensionWindow( QWidget* parent, BaseExtension* ext ) :
    QMainWindow( parent ), m_extension( ext )
{
    setupUi( this );
    m_standartCommand = "// put your instructions here";

    m_lineLabel = new QLabel( "Line: 1" );
    m_columnLabel = new QLabel( "Column: 1" );
    m_countLabel = new QLabel( "Count: 1" );

    m_statusBar->addPermanentWidget( m_countLabel );
    m_statusBar->addPermanentWidget( m_lineLabel );
    m_statusBar->addPermanentWidget( m_columnLabel );

    m_highlighter.setDocument( m_textEdit->document() );

    installEventFilter( this );

    m_separatorAction = menuFile->addSeparator();

    for ( int j = 0; j < MaxRecentFiles; j++ )
    {
        m_recentFileActions[j] = new QAction( this );
        m_recentFileActions[j]->setVisible( false );
        connect( m_recentFileActions[j], SIGNAL( triggered() ),
                 this, SLOT ( openRecentFile() ) );
        menuFile->addAction( m_recentFileActions[j] );
    }

    menuFile->addSeparator();
    menuFile->addAction( actionExit );
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

    int size = m_settings.beginReadArray( "RECENTFILE" );

    for ( int i = 0; i < size; i++ )
    {
        m_settings.setArrayIndex( i );

        if ( QFile::exists( m_settings.value( "path" ).toString() ) )
            m_recentFiles.append( m_settings.value( "path" ).toString() );
    }

    m_settings.endArray();

    m_settings.beginGroup( "FILEDIALOG" );
    m_lastOpenedDir = m_settings.value( "m_lastOpenedDir" ).toString();
    m_lastOpenedFile = m_settings.value( "m_lastOpenedFile" ).toString();

    if ( m_lastOpenedDir.isEmpty() || !QDir( m_lastOpenedDir ).exists() )
        m_lastOpenedDir = QDir::rootPath();

    if ( m_lastOpenedFile.isEmpty() || !QFile::exists( m_lastOpenedFile ) )
    {
        m_textEdit->setText( m_standartCommand );
        setFileName( "" );
    }
    else loadFile( m_lastOpenedFile );

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

    m_settings.beginGroup( "FILEDIALOG" );

    if ( !m_lastOpenedDir.isEmpty() )
        m_settings.setValue( "m_lastOpenedDir", m_lastOpenedDir );

    if ( m_nameCurrentFile != "new.txt" )
        m_settings.setValue( "m_lastOpenedFile", m_nameCurrentFile );
    else m_settings.setValue( "m_lastOpenedFile", "" );

    m_settings.endGroup();

    m_settings.beginWriteArray( "RECENTFILE" );

    for ( int i = 0; i < m_recentFiles.size(); i++ )
    {
        m_settings.setArrayIndex( i );
        m_settings.setValue( "path", m_recentFiles.at( i ) );
    }

    m_settings.endArray();
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
    if ( mayBeSave() )
        event->accept();
    else
        event->ignore();
}

void ExtensionWindow::on_actionExit_triggered()
{
    close();
}

void ExtensionWindow::on_actionPostprocess_triggered()
{
    QString result = m_extension->PostprocessScript( m_textEdit->toPlainText() );

    if ( !result.isEmpty() )
    {
        BaseExtension::EnterModalLoop();
        QMessageBox::critical( m_extension->GetTopWindow(), tr( "Paratran postprocessing" ),
                               result, QMessageBox::Yes );
        BaseExtension::ExitModalLoop();
    }
}

void ExtensionWindow::on_actionPreprocess_triggered()
{
    QString result = m_extension->PreprocessModel();
    m_textEdit->setText( result );
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

void ExtensionWindow::on_actionOpenScript_triggered()
{
    if ( mayBeSave() )
    {
        QFileDialog fileDialog;
        QString fileName = fileDialog.getOpenFileName( this, tr( "Open" ),
                                                       m_lastOpenedDir,
                                                       tr( "Normal text file (*.txt)" ) );

        if ( fileName.isEmpty() )
            return;

        int indexLastSlash = fileName.lastIndexOf( "/" );
        m_lastOpenedDir = fileDialog.directory().filePath( fileName )
                          .remove( indexLastSlash + 1, fileName.size() - indexLastSlash - 1 );

        loadFile( fileName );
    }
}

void ExtensionWindow::loadFile( const QString& name )
{
    QFile file( name );

    if ( file.open( QFile::ReadWrite | QFile::Text ) )
    {
        QTextStream stream( &file );
        QApplication::setOverrideCursor( Qt::WaitCursor );
        m_textEdit->setText( stream.readAll() );
        file.close();
        QApplication::restoreOverrideCursor();

        setFileName( name );
    }
}

void ExtensionWindow::on_actionReloadScript_triggered()
{
    QString nameFile = m_nameCurrentFile;

    if ( mayBeSaveForReload() )
    {
        m_textEdit->clear();
        loadFile( nameFile );
    }
}

void ExtensionWindow::on_actionNewScript_triggered()
{
    if ( mayBeSave() )
    {
        m_textEdit->clear();
        m_textEdit->setText( m_standartCommand );
        setFileName( "" );
    }
}

bool ExtensionWindow::on_actionSaveScript_triggered()
{
    if ( m_nameCurrentFile == "new.txt" )
        return on_actionSaveAsScript_triggered();
    else
        return  saveFile( m_nameCurrentFile );
}

bool ExtensionWindow::on_actionSaveAsScript_triggered()
{
    QFileDialog fileDialog;
    QString nameSavedFile;

    if ( m_nameCurrentFile == "new.txt" )
        nameSavedFile = m_lastOpenedDir + "model.txt";
    else nameSavedFile = m_nameCurrentFile;

    QString fileName = fileDialog.getSaveFileName( this,
                                                   tr( "Save as.." ),
                                                   nameSavedFile,
                                                   tr( "Normal text file (*.txt)" ) );

    if ( !fileName.isEmpty() )
    {
        int indexLastSlash = fileName.lastIndexOf( "/" );
        m_lastOpenedDir = fileDialog.directory().filePath( fileName )
                          .remove( indexLastSlash + 1, fileName.size() - indexLastSlash - 1 );
        return saveFile( fileName );
    }
    else return false;
}

bool ExtensionWindow::saveFile ( const QString& fileName )
{
    QFile file( fileName );

    if ( !file.open( QFile::WriteOnly | QFile::Text ) )
    {
        QMessageBox::warning( this, tr( "Paratran" ), QString( "Unable to save the file %1 :\n%2" )
                              .arg( fileName )
                              .arg( file.errorString() ) );
        return false;
    }

    setFileName( fileName );

    QTextStream out( &file );
    QApplication::setOverrideCursor( Qt::WaitCursor );
    out << m_textEdit->toPlainText();
    out.flush();
    file.close();
    QApplication::restoreOverrideCursor();
    return true;
}

void ExtensionWindow::on_m_textEdit_textChanged()
{
    if ( m_textEdit->document()->isModified() )
    {
        setWindowTitle( "*" + m_nameCurrentFile + " - Paratran" );

        if ( m_nameCurrentFile != "new.txt" )
            actionReloadScript->setEnabled( true );
        else actionReloadScript->setEnabled( false );
    }
}

void ExtensionWindow::on_m_textEdit_cursorPositionChanged()
{
    int count = m_textEdit->document()->lineCount();

    int position = m_textEdit->textCursor().position();
    QTextBlock block = m_textEdit->document()->findBlock( position );
    int col = position - block.position() + 1;
    int row = block.blockNumber() + 1;

    m_countLabel->setText( QString ( "%1 %2" )
                           .arg( "Count: " )
                           .arg( count ) );
    m_lineLabel->setText( QString ( "%1 %2" )
                          .arg( "Line: " )
                          .arg( row ) );
    m_columnLabel->setText( QString ( "%1 %2" )
                            .arg( "Column: " )
                            .arg( col ) );
}

bool ExtensionWindow::mayBeSave()
{
    if ( windowTitle().contains( "*" ) )
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning( this, "Paratran", "Document is changed.\nAre you sure you want to save the file?",
                                    QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel );

        if ( ret == QMessageBox::Save )
            return on_actionSaveScript_triggered();
        else if ( ret == QMessageBox::Cancel )
            return false;
    }

    return true;
}

bool ExtensionWindow::mayBeSaveForReload()
{
    if ( windowTitle().contains( "*" ) )
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning( this, "Paratran", "Document is changed.\nAre you sure you want to save the file?",
                                    QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel );

        if ( ret == QMessageBox::Save )
            return on_actionSaveAsScript_triggered();
        else if ( ret == QMessageBox::Cancel )
            return false;
    }

    return true;
}

void ExtensionWindow::setFileName( const QString& fileName )
{
    m_nameCurrentFile = fileName;
    m_textEdit->document()->setModified( false );

    if ( m_nameCurrentFile.isEmpty() )
        m_nameCurrentFile = "new.txt";

    setWindowTitle( m_nameCurrentFile + " - Paratran" );

    actionReloadScript->setEnabled( false );

    updateRecentFileActions( fileName );
}

void ExtensionWindow::on_actionSelectAll_triggered()
{
    m_textEdit->selectAll();
}

void ExtensionWindow::on_actionUndo_triggered()
{
    m_textEdit->undo();

    if ( !actionUndo->isEnabled() )
        setFileName( m_nameCurrentFile );
}

void ExtensionWindow::on_actionRedo_triggered()
{
    m_textEdit->redo();
}

void ExtensionWindow::on_m_textEdit_copyAvailable( bool flag )
{
    actionCopy->setEnabled( flag );
    actionCut->setEnabled( flag );
}

void ExtensionWindow::on_actionPaste_triggered()
{
    m_textEdit->paste();
}

void ExtensionWindow::on_actionCopy_triggered()
{
    m_textEdit->copy();
}

void ExtensionWindow::on_actionCut_triggered()
{
    m_textEdit->cut();
}

void ExtensionWindow::on_m_textEdit_undoAvailable( bool flag )
{
    actionUndo->setEnabled( flag );
}

void ExtensionWindow::on_m_textEdit_redoAvailable( bool flag )
{
    actionRedo->setEnabled( flag );
}

void ExtensionWindow::on_actionTest_triggered()
{
    if ( mayBeSave() )
    {
        m_textEdit->clear();
        setFileName( "" );

        Category& logger = BaseExtension::GetLogger();
        logger.info( "Start test." );

        QApplication::setOverrideCursor( Qt::WaitCursor );
        QString result = m_extension->Test();

        QFile file( "T:\\paratran\\trunk\\system\\test\\standartModel.txt" );

        if ( file.open( QFile::ReadWrite | QFile::Text ) )
        {
            QTextStream stream( &file );
            QString standartPre = stream.readAll();
            file.close();

            QStringList resultList = result.split( "\n" );
            QStringList standartPreList = standartPre.split( "\n" );

            if ( result == standartPre )
            {
                QApplication::restoreOverrideCursor();
                QMessageBox::information( this, "Paratran", "Test passed",
                                          QMessageBox::Yes );
            }
            else
            {
                m_textEdit->setText( result );

                for ( int i = 0; i < qMin( resultList.size(), standartPreList.size() ); i++ )
                {
                    if ( resultList.at( i ) != standartPreList.at( i ) )
                    {
                        QApplication::restoreOverrideCursor();
                        QMessageBox::critical( this, "Paratran", QString( "Test is not passed. Line %1" )
                                               .arg( i + 1 ),
                                               QMessageBox::Yes );
                        break;
                    }
                }
            }
        }

        logger.info( "End test." );
    }
}

void ExtensionWindow::updateRecentFileActions( const QString& fileName )
{
    m_recentFiles.removeAll( fileName );
    m_recentFiles.prepend( fileName );

    QMutableStringListIterator i( m_recentFiles );

    while ( i.hasNext() )
    {
        if ( !QFile::exists( i.next() ) )
            i.remove();
    }

    if ( m_recentFiles.size() > MaxRecentFiles )
        m_recentFiles.removeLast();

    for ( int j = 0; j < MaxRecentFiles; j++ )
    {
        if ( j < m_recentFiles.size() )
        {
            QString text = tr( "&%1 %2" )
                           .arg( j + 1 )
                           .arg( m_recentFiles.at( j ) );
            m_recentFileActions[j]->setText( text );
            m_recentFileActions[j]->setData( m_recentFiles.at( j ) );
            m_recentFileActions[j]->setVisible( true );
        }
        else m_recentFileActions[j]->setVisible( false );
    }

    m_separatorAction->setVisible( !m_recentFiles.isEmpty() );
}

void ExtensionWindow::openRecentFile()
{
    if ( mayBeSave() )
    {
        QAction* action = qobject_cast<QAction*>( sender() );

        if ( action )
            loadFile( action->data().toString() );
    }
}

void ExtensionWindow::on_actionNamingTest_triggered()
{
    m_extension->NamingTest();
}
