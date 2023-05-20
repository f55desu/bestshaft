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
    ui->paraview_lineEdit->setText(m_extension->bestshaft_paraview_path);

    setAttribute(Qt::WA_DeleteOnClose);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    m_extension->bestshaft_workspace_path = ui->workspaceLineEdit->text();
    m_extension->bestshaft_paraview_path = ui->paraview_lineEdit->text();

//    // Save mesh parameters
//    m_extension->mesh_max_facet_size_factor = ui->maxFacetSizeSpinBox->value();
//    m_extension->force_lenght_factor = ui->forceLenghtSpinBox->value();
//    m_extension->mesh_concentrator_factor = ui->meshConcentratorSpinBox->value();
//    m_extension->intermediate_top_length_factor = ui->topLenghtSpinBox->value();
//    m_extension->intermediate_middle_length_factor = ui->middleLenghtSpinBox->value();
//    m_extension->intermediate_bottom_length_factor = ui->bottomLenghtSpinBox->value();
//    m_extension->constraint_length_factor = ui->constraintLenghtSpinBox->value();
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


void SettingsDialog::on_browseParaViewButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                               QDir::homePath(),
                                                               QFileDialog::ShowDirsOnly
                                                               | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
    {
        ui->paraview_lineEdit->setText(directory);
    }
}

