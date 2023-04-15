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
    void on_solveButton_clicked();
    void on_cancelButton_clicked();
    void solveEnd(int exitCode, QProcess::ExitStatus exitStatus = QProcess::NormalExit);
    void onMultiplySelection();
private:
    QProcess* m_currentProcess;
    QList<int> calculatedVariants;
    QString m_tmpName;
    void on_solve_stop(int error...);
    void startTetgen(int selectedItemId);

    void on_pushButton_2_clicked();

private:
    DataModel* m_tableModel;
};
