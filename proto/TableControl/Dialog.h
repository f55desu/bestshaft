#pragma once

#include "ui_Dialog.h"

#include <QProcess>
#include <QDialog>

#include "DataModel.h"

class Dialog : public QDialog, private Ui::Dialog
{
    Q_OBJECT
public:
    friend class QObject;
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    int get_col_id(const QString& name);
private slots:
    void on_pushButton_clicked();
<<<<<<< HEAD
    void on_solveButton_clicked();
    void on_cancelButton_clicked();
    void solveEnd(int exitCode, QProcess::ExitStatus exitStatus = QProcess::NormalExit);
private:
    QProcess* m_currentProcess;
    QString m_tmpName;
    void on_solve_stop(int error...);
    void startTetgen(int selectedItemId);
=======

    void on_pushButton_2_clicked();

private:
    DataModel* m_tableModel;
>>>>>>> c2a8b55771909f436c0902f27c185c83d93c95ca
};
