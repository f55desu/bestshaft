#ifndef BESTSHAFTWINDOW_H
#define BESTSHAFTWINDOW_H

#include <QMainWindow>
#include "ui_BestShaftWindow.h"

class BaseExtension;

class BestShaftWindow : public QMainWindow, public Ui::BestShaftWindow
{
    Q_OBJECT

public:
    BestShaftWindow( QWidget* parent = NULL, BaseExtension* ext = NULL );
    ~BestShaftWindow();
protected:
    virtual void showEvent( QShowEvent* e );
    virtual void closeEvent( QCloseEvent* e );
    void on_actionExit_triggered();
private slots:
    void on_addButton_clicked();

    void on_applyButton_clicked();

    void on_calculateButton_clicked();

    void on_paraviewButton_clicked();

    void on_deleteButton_clicked();

private:
    BaseExtension* m_extension;
    Ui::BestShaftWindow *ui;
    bool m_firstShowFlag;
};

#endif // BESTSHAFTWINDOW_H
