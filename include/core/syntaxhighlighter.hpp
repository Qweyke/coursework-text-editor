#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class SyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

  public:
	explicit SyntaxHighlighter(QTextDocument* parent = nullptr);
	void setLanguage(const QString& language);

  protected:
	void highlightBlock(const QString& text) override;

  private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	QTextCharFormat keywordFormat;
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat numberFormat;

	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

	void setupCppHighlighting();
	void setupPythonHighlighting();
	void setupJavaHighlighting();
	void setupJavaScriptHighlighting();
	void setupHtmlHighlighting();
};
