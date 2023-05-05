#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "ExtensionWindow.h"

class ExtensionWindow;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(ExtensionWindow *parent = NULL);
    ~SettingsDialog();

private slots:
    void on_buttonBox_accepted();

    void on_browseWorkspaceButton_clicked();

    void on_browseParaViewButton_clicked();

private:
    BaseExtension *m_extension;

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
