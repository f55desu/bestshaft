#include "Syntaxhighlighter.h"

/*SyntaxHighlighter::SyntaxHighlighter(QObject *parent) :
QSyntaxHighlighter(parent)
{
}*/

void SyntaxHighlighter::highlightBlock( const QString& text )
{
    HighlightingRule rule;

    keywordFormat.setForeground( Qt::darkMagenta );
    keywordFormat.setFontWeight( QFont::Bold );
    keywordFormat.setFontItalic( true );
    //QStringList keywordPatterns;

    //keywordPatterns << "\\bvar\\b" << "\\bwith\\b";

    /*foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }*/

    rule.pattern = QRegularExpression( "\\b(var|with|for)\\b" );
    rule.format = keywordFormat;
    highlightingRules.append( rule );

    digitalFormat.setForeground( Qt::darkCyan );
    rule.pattern = QRegularExpression( "(\\b\\d+\\b)" );
    rule.format = digitalFormat;
    highlightingRules.append( rule );

    textFormat.setForeground( Qt::darkRed );
    QRegularExpression reg ( "\".*?\"" );
    rule.pattern = reg;
    rule.format = textFormat;
    highlightingRules.append( rule );

    singleLineCommentFormat.setForeground( Qt::darkGreen );
    rule.pattern = QRegularExpression( "//[^\n]*" );
    rule.format = singleLineCommentFormat;
    highlightingRules.append( rule );

    foreach ( const HighlightingRule& rule, highlightingRules )
    {
        QRegularExpression expression( rule.pattern );
        QRegularExpressionMatchIterator i = expression.globalMatch( text );

        while ( i.hasNext() )
        {
            QRegularExpressionMatch match = i.next();
            setFormat( match.capturedStart(), match.capturedLength(), rule.format );
        }
    }
}
