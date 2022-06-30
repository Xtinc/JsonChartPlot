#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    // keywordFormat.setFontWeight(QFont::Bold);
    // rule.pattern = QRegularExpression("^\\[.*\\]", QRegularExpression::CaseInsensitiveOption);
    // rule.format = keywordFormat;
    // highlightingRules.append(rule);

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("^\\[.*\\] CRIT.*");
    rule.format = classFormat;
    highlightingRules.append(rule);

    commentFormat.setFontWeight(QFont::Bold);
    commentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("^\\[.*\\] MSGS.*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    commentFormat.setFontWeight(QFont::Bold);
    plusFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegularExpression("^\\[.*\\] WARN.*");
    rule.format = plusFormat;
    highlightingRules.append(rule);

    commentFormat.setFontWeight(QFont::Bold);
    minusFormat.setFontWeight(QFont::Black);
    rule.pattern = QRegularExpression("^\\[.*\\] INFO.*");
    rule.format = minusFormat;
    highlightingRules.append(rule);
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString &text)
{
    for each (const HighlightingRule &rule in highlightingRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);
}