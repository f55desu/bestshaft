#pragma once

#include "ui_ExtensionWindow.h"
#include "BaseExtension.h"

#define MaxRecentFiles 5

class BaseExtension;

class ExtensionWindow : public QDialog, public Ui::ExtensionWindow
{
    Q_OBJECT

    friend class SettingsDialog;

public:
    ExtensionWindow( QWidget* parent = NULL, BaseExtension* ext = NULL );
public:
    ~ExtensionWindow();

protected:
    virtual void showEvent( QShowEvent* e );
protected:
    virtual void closeEvent( QCloseEvent* e );
protected:
    bool nativeEvent( const QByteArray& eventType, void* message, qintptr* result );
    bool eventFilter( QObject* obj, QEvent* e );

private slots:
    void on_actionExit_triggered();
    void on_addButton_clicked();
    void on_applyButton_clicked();
    void on_calculateButton_clicked();
    void on_paraviewButton_clicked();
    void on_deleteButton_clicked();
    void on_cancelButton_clicked();

    void onMultiplySelection();
    void on_cellChanged( int row, int column );
    void on_cellEntered(int row, int column);

protected:
    double calculateMaxTension( const QString& ccx_dat_filepath );
private:
    void boldRow( int rowId, QTableWidget* tableWidget, bool bold = true );
    void initilizeVariant();
    int GetColumnId( const QString& name );
    QString GenerateVariantName();
private:
    void startSolve();
    void solveEnd( int exitCode, QProcess::ExitStatus );
    void solveStart();
    void on_solve_stop( int error, ... );

//private:
//    void createLanguageMenu();
protected:
    void writeSettings();
protected:
    void readSettings();

protected:
    BaseExtension* m_extension;
private:
    QProcess* m_currentProcess;
    QModelIndexList m_rowsToBeProceed;
    QList<int> calculatedVariants;
private:
    int currentVariantId;
    int m_currentIndex;
private:
    QTranslator m_qtranslator;
//private:
//    QString m_langPath;
private:
    bool m_firstShowFlag;
    bool onSolving = false;
};
