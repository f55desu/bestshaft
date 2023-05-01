#pragma once

#define QSCRIPT_ERROR(message) { \
                                  m_logger.error(message); \
                                  return m_scriptContext->throwError(message); \
                               }
#define QSCRIPT_COM_ERROR(exeption) { \
                                      QString errorMessage = QString("COM error in the extension: %1").arg(QString::fromLocal8Bit(exeption.ErrorMessage())); \
                                      m_logger.error(qPrintable(errorMessage)); \
                                      return m_scriptContext->throwError(errorMessage); \
                                    }

#define QSHOW_ERROR(message) { \
                               m_logger.error(message); \
                               QMessageBox::critical(0x0, tr("Error"), message); \
                             }

class BaseExtension : public QObject
{
    Q_OBJECT

public:
    static const QString IniFileName;
public:
    static const QString LogFileName;
public:
    static const QString HomePathEnvName;

public:
    typedef QMap<QString, double> Variant;

    QList<Variant> variants;
    //QMap<QString, Variant> model;

public:
    BaseExtension& operator=( const BaseExtension& ) {}
protected:
    BaseExtension();

public:
    virtual void Initialize() = 0;
protected:
    virtual QWidget* GetTopWindow() = 0;
public:
    void SetTopWindow( QWidget* topWindow );

public:
    void RunEditor();
public:
    static bool GetModalState();

//public:
//    virtual void InitScriptEngine();

protected:
    virtual void InitQt();
public:
    virtual void TermQt();

protected:
    static void EnterModalLoop();
protected:
    static void ExitModalLoop();

public:
    QString GetHomePath() const;
    const QString bestshaft_workspace_folder_name = "BestshaftWorkspace";
    QString bestshaft_workspace_path = QDir::homePath() + QDir::separator() + bestshaft_workspace_folder_name;
public:
    static Category& GetLogger();

protected:
    virtual Variant ExtractVariant() = 0;
    virtual void ApplyVariant( Variant variant ) = 0;
    virtual void WriteVariants(QMap<QString, BaseExtension::Variant> variants) = 0;
    virtual void ReadVariants(QMap<QString, Variant> &variants) = 0;

//    virtual void CalculateMaxTension( Variant variant ) = 0;

    virtual void SaveMeshDatabase( const QString& wavefront_obj_file_path,
                                   const QString& stl_file_path,
                                   const QString& tetgen_input_poly_file_path,
                                   const QString& tetgen_input_smesh_file_path,
                                   const QString& tetgen_input_mtr_file_path,
                                   const QString& gmsh_msh_file_path,
                                   double& max_facet_size ) = 0;

//protected:
//    virtual QString Test() = 0;
//protected:
//    virtual void NamingTest() = 0;

    friend class ExtensionWindow;
private:
    ExtensionWindow* m_extensionWindow;

protected:
    QWidget* m_topWindow;
protected:
    static int m_modalLoopCount;
protected:
    QString m_iniFileName;
protected:
    static Category& m_logger;
protected:
    QString m_homePath;
//protected:
//    QJSEngine* m_scriptEngine;
};
