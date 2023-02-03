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
    void About();
public:
    static bool GetModalState();

public:
    virtual void InitScriptEngine();

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
public:
    static Category& GetLogger();

protected:
    QString PostprocessScript( QString scriptText );
protected:
    virtual QString PreprocessModel() = 0;
protected:
    virtual QString Test() = 0;
protected:
    virtual void NamingTest() = 0;

    friend class ExtensionWindow;
    friend class BestShaftWindow;
private:
    ExtensionWindow* m_extensionWindow;
    BestShaftWindow* m_bestShaftWindow;

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
protected:
    QJSEngine* m_scriptEngine;
};
