#pragma once

#include "ui_ExtensionWindow.h"

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

protected:
    double calculateMaxTension();
private:
    void boldRow(int rowId, QTableWidget* tableWidget, bool bold=true);
    void initilizeVariant();
//private:
//    void createLanguageMenu();
protected:
    void writeSettings();
protected:
    void readSettings();

private:
    BaseExtension* m_extension;
private:
    QTranslator m_qtranslator;
//private:
//    QString m_langPath;

private:
    bool m_firstShowFlag;
};
