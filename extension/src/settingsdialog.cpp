#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "ExtensionWindow.h"

SettingsDialog::SettingsDialog(ExtensionWindow *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    // Taking a pointer to BaseExtension from parent ExtensionWindow
    m_extension = parent->m_extension;
    ui->workspaceLineEdit->setText(m_extension->bestshaft_workspace_path);
    ui->postprocessor_lineEdit->setText(m_extension->postprocessor_path);
    ui->args_lineEdit->setText(m_extension->postprocessor_args);
    ui->maxFacetSizeSpinBox->setValue(m_extension->mesh_max_facet_size_factor);

    setAttribute(Qt::WA_DeleteOnClose);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    m_extension->bestshaft_workspace_path = ui->workspaceLineEdit->text();
    m_extension->postprocessor_path = ui->postprocessor_lineEdit->text();
    m_extension->postprocessor_args = ui->args_lineEdit->text();
    m_extension->mesh_max_facet_size_factor = ui->maxFacetSizeSpinBox->value();
}


void SettingsDialog::on_browseWorkspaceButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                               m_extension->bestshaft_workspace_path,
                                                               QFileDialog::ShowDirsOnly
                                                               | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
    {
        ui->workspaceLineEdit->setText(directory);
    }
}


void SettingsDialog::on_browsePostprocessorButton_clicked()
{
    QString postprocessorFile = QFileDialog::getOpenFileName(this, tr("Select Binary Postprocessor File"),
                                                               m_extension->m_homePath,
                                                               tr("Binary (*.exe)"));
    if (!postprocessorFile.isEmpty())
    {
        ui->postprocessor_lineEdit->setText(postprocessorFile);
    }
}

