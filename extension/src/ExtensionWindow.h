#pragma once

#include "ui_ExtensionWindow.h"
#include "BaseExtension.h"

#define MaxRecentFiles 5

class BaseExtension;

class ExtensionWindow : public QDialog, public Ui::ExtensionWindow
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
    void on_actionExit_triggered();
    void on_addButton_clicked();
    void on_applyButton_clicked();
    void on_calculateButton_clicked();
    void on_paraviewButton_clicked();
    void on_deleteButton_clicked();

    void onMultiplySelection();
    void on_cellChanged( int row, int column );

protected:
    double calculateMaxTension();
    //void updateTableRows( QList<BaseExtension::Variant> variants );
private:
    void boldRow( int rowId, QTableWidget* tableWidget, bool bold = true );
    void initilizeVariant();
    int GetColumnId( const QString& name );
    QString GenerateVariantName();

//private:
//    void createLanguageMenu();
protected:
    void writeSettings();
protected:
    void readSettings();

private:
    BaseExtension* m_extension;
private:
    int currentVariantId;
private:
    QTranslator m_qtranslator;
//private:
//    QString m_langPath;

private:
    bool m_firstShowFlag;
};
