#pragma once

#include "ui_ExtensionWindow.h"
#include "Syntaxhighlighter.h"

#define MaxRecentFiles 5

class BaseExtension;

class ExtensionWindow : public QMainWindow, public Ui::ExtensionWindow
{
    Q_OBJECT

public:
    ExtensionWindow( QWidget* parent = NULL, BaseExtension* ext = NULL );
public:
    ~ExtensionWindow();

protected:
    virtual void showEvent( QShowEvent* e );
protected:
    virtual void closeEvent( QCloseEvent* e );
protected:
    bool eventFilter( QObject* obj, QEvent* e );

private slots:
    void on_actionPostprocess_triggered();
    void on_actionPreprocess_triggered();
    void on_actionOpenScript_triggered();
    bool on_actionSaveScript_triggered();
    bool on_actionSaveAsScript_triggered();
    void on_actionNewScript_triggered();
    void on_actionExit_triggered();
    void on_m_textEdit_textChanged();
    void on_m_textEdit_cursorPositionChanged();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionSelectAll_triggered();
    void on_m_textEdit_copyAvailable( bool flag );
    void on_actionReloadScript_triggered();
    void on_m_textEdit_undoAvailable( bool flag );
    void on_m_textEdit_redoAvailable( bool flag );
    void on_actionTest_triggered();
    void openRecentFile();

    void on_actionNamingTest_triggered();

private:
    void createLanguageMenu();
protected:
    void writeSettings();
protected:
    void readSettings();

private:
    void setFileName( const QString& fileName );
private:
    bool saveFile( const QString& fileName );
private:
    bool mayBeSave();
private:
    bool mayBeSaveForReload();
private:
    void loadFile( const QString& name );
private:
    void updateRecentFileActions( const QString& fileName );

private:
    BaseExtension* m_extension;
private:
    QTranslator m_qtranslator;
private:
    QString m_langPath;

private:
    SyntaxHighlighter m_highlighter;

private:
    QLabel* m_lineLabel;
private:
    QLabel* m_countLabel;
private:
    QLabel* m_columnLabel;
private:
    QAction* m_recentFileActions[MaxRecentFiles];
private:
    QAction* m_separatorAction;

private:
    QStringList m_recentFiles;
private:
    QString m_nameCurrentFile;
private:
    QString m_lastOpenedDir;
private:
    QString m_lastOpenedFile;
private:
    QString m_standartCommand;
private:
    bool m_firstShowFlag;
};
