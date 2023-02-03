#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter( QTextDocument* parent = 0 ): QSyntaxHighlighter( parent ) {}

protected:
    void highlightBlock( const QString& text );

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat textFormat;
    QTextCharFormat digitalFormat;
    QTextCharFormat singleLineCommentFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
