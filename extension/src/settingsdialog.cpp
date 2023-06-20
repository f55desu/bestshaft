#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "ExtensionWindow.h"
#include <QTranslator>

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


void SettingsDialog::on_comboBox_currentIndexChanged(int index)
{
    QTranslator t;
    bool loaded = false;
    if (index == 0)
    {
        loaded = t.load(":/extension/ui/extension_en.qm");
    }
    else if (index == 3)
    {
        loaded = t.load(":/extension/ui/extension_jp.qm");
    }

//    case 1:
//       // loaded = t.load("../ui/extension_ru.qm");
//        break;
//    case 2:
//        //loaded = t.load("../ui/extension_de.qm");
//        break;
//    case 3:
//        loaded = t.load(":/ui/extension_jp.qm");
//        break;
//    case 4:
//        //loaded = t.load("../ui/extension_it.qm");
//        break;
//    case 5:
//        //loaded = t.load("../ui/extension_zh.qm");
//        break;
    qApp->installTranslator(&t);
    ui->retranslateUi(this);
    if (loaded)
    {

    }
}

