#pragma once

#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog, private Ui::SettingsDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
};

