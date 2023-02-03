#include "Stable.h"
#include "BaseExtension.h"
#include "ExtensionWindow.h"
#include "IAbstractModeler.h"

QString BaseExtension::PostprocessScript( QString scriptText )
{
    m_logger.info( "Start postprocessing." );

    QString resultMessage;

    //QScriptSyntaxCheckResult syntaxErrorResult = m_scriptEngine->checkSyntax(scriptText);
    //if( syntaxErrorResult.state() == QScriptSyntaxCheckResult::Valid)
    //{
    scriptText = "with(Modeler){" + scriptText + "\n}";
    QJSValue result = m_scriptEngine->evaluate( scriptText );

    if ( result.isError() /*|| m_scriptEngine->hasUncaughtException()*/ )
    {
        //int line = m_scriptEngine->uncaughtExceptionLineNumber();
        //resultMessage = "Script engine error at line " + QString("%1").arg(line) +
        //                ": " + result.toString();
        resultMessage = QString( "Script engine error at line %1: %2" )
                        .arg( result.property( "lineNumber" ).toInt() ).arg( result.toString() );
        m_logger.warn( resultMessage.toStdString() );
    }

    //}
    /*else
    {
        resultMessage = "Script check syntax error: " +
                syntaxErrorResult.errorMessage() +
                " at line " + QString("%1").arg(syntaxErrorResult.errorLineNumber()) +
                " at column " + QString("%1").arg(syntaxErrorResult.errorColumnNumber());
         m_logger.warn(resultMessage.toStdString());
    }*/

    m_logger.info( "End postprocessing." );

    return resultMessage;
}
