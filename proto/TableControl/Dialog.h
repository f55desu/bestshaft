#pragma once

#include "ui_Dialog.h"

#include <QDialog>

class Dialog : public QDialog, private Ui::Dialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    int get_col_id(const QString& name);
private slots:
    void on_pushButton_clicked();
};
