#include "core/syntaxhighlighter.hpp"
#include <QTextDocument>
#include <QFont>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);

	classFormat.setForeground(Qt::darkMagenta);
	classFormat.setFontWeight(QFont::Bold);

	singleLineCommentFormat.setForeground(Qt::red);

	multiLineCommentFormat.setForeground(Qt::red);

	quotationFormat.setForeground(Qt::darkGreen);

	functionFormat.setForeground(Qt::blue);
	functionFormat.setFontItalic(true);

	numberFormat.setForeground(Qt::darkCyan);

	setLanguage("cpp");
}

void SyntaxHighlighter::setLanguage(const QString& language)
{
	highlightingRules.clear();

	if (language == "cpp" || language == "c" || language == "h" || language == "hpp")
	{
		setupCppHighlighting();
	}
	else if (language == "py" || language == "python")
	{
		setupPythonHighlighting();
	}
	else if (language == "java")
	{
		setupJavaHighlighting();
	}
	else if (language == "js" || language == "javascript")
	{
		setupJavaScriptHighlighting();
	}
	else if (language == "html" || language == "xml")
	{
		setupHtmlHighlighting();
	}

	rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
	for (const HighlightingRule& rule : highlightingRules)
	{
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext())
		{
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
	{
		startIndex = text.indexOf(commentStartExpression);
	}

	while (startIndex >= 0)
	{
		QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1)
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex + match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}
}

void SyntaxHighlighter::setupCppHighlighting()
{
	HighlightingRule rule;

	QStringList keywordPatterns = {
		"\\bchar\\b",    "\\bclass\\b",    "\\bconst\\b",     "\\bdouble\\b",   "\\benum\\b",     "\\bexplicit\\b",  "\\bfriend\\b",   "\\binline\\b",
		"\\bint\\b",     "\\blong\\b",     "\\bnamespace\\b", "\\boperator\\b", "\\bprivate\\b",  "\\bprotected\\b", "\\bpublic\\b",   "\\bshort\\b",
		"\\bsignals\\b", "\\bsigned\\b",   "\\bslots\\b",     "\\bstatic\\b",   "\\bstruct\\b",   "\\btemplate\\b",  "\\btypedef\\b",  "\\btypename\\b",
		"\\bunion\\b",   "\\bunsigned\\b", "\\bvirtual\\b",   "\\bvoid\\b",     "\\bvolatile\\b", "\\bbool\\b",      "\\bif\\b",       "\\belse\\b",
		"\\bfor\\b",     "\\bwhile\\b",    "\\bdo\\b",        "\\bswitch\\b",   "\\bcase\\b",     "\\bbreak\\b",     "\\bcontinue\\b", "\\breturn\\b",
		"\\bgoto\\b",    "\\btry\\b",      "\\bcatch\\b",     "\\bthrow\\b",    "\\bnew\\b",      "\\bdelete\\b",    "\\bsizeof\\b",   "\\bthis\\b",
		"\\btrue\\b",    "\\bfalse\\b",    "\\bnullptr\\b",   "\\bnull\\b",     "\\bauto\\b",     "\\busing\\b",     "\\bnamespace\\b"
	};

	for (const QString& pattern : keywordPatterns)
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
	rule.format = classFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("'.*'");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("//[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
	rule.format = numberFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegularExpression("/\\*");
	commentEndExpression = QRegularExpression("\\*/");
}

void SyntaxHighlighter::setupPythonHighlighting()
{
	HighlightingRule rule;

	QStringList keywordPatterns = { "\\bdef\\b",    "\\bclass\\b",  "\\bif\\b",      "\\belse\\b",  "\\belif\\b",     "\\bfor\\b",    "\\bwhile\\b",
		                            "\\breturn\\b", "\\bimport\\b", "\\bfrom\\b",    "\\bas\\b",    "\\btry\\b",      "\\bexcept\\b", "\\bfinally\\b",
		                            "\\braise\\b",  "\\bwith\\b",   "\\bpass\\b",    "\\bbreak\\b", "\\bcontinue\\b", "\\bTrue\\b",   "\\bFalse\\b",
		                            "\\bNone\\b",   "\\band\\b",    "\\bor\\b",      "\\bnot\\b",   "\\bin\\b",       "\\bis\\b",     "\\blambda\\b",
		                            "\\byield\\b",  "\\bglobal\\b", "\\bnonlocal\\b" };

	for (const QString& pattern : keywordPatterns)
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	rule.pattern = QRegularExpression("\\bdef\\s+(\\w+)");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\".*\"|'''.*'''");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("#[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
	rule.format = numberFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegularExpression("\"\"\"");
	commentEndExpression = QRegularExpression("\"\"\"");
}

void SyntaxHighlighter::setupJavaHighlighting()
{
	HighlightingRule rule;

	QStringList keywordPatterns = { "\\bpublic\\b",  "\\bprivate\\b",    "\\bprotected\\b", "\\bstatic\\b", "\\bfinal\\b",  "\\bclass\\b",    "\\binterface\\b",
		                            "\\bextends\\b", "\\bimplements\\b", "\\bpackage\\b",   "\\bimport\\b", "\\bif\\b",     "\\belse\\b",     "\\bfor\\b",
		                            "\\bwhile\\b",   "\\bdo\\b",         "\\bswitch\\b",    "\\bcase\\b",   "\\bbreak\\b",  "\\bcontinue\\b", "\\breturn\\b",
		                            "\\btry\\b",     "\\bcatch\\b",      "\\bfinally\\b",   "\\bthrow\\b",  "\\bthrows\\b", "\\bnew\\b",      "\\bthis\\b",
		                            "\\bsuper\\b",   "\\btrue\\b",       "\\bfalse\\b",     "\\bnull\\b",   "\\bint\\b",    "\\bvoid\\b",     "\\bboolean\\b",
		                            "\\bchar\\b",    "\\bdouble\\b",     "\\bfloat\\b",     "\\blong\\b",   "\\bshort\\b",  "\\bbyte\\b" };

	for (const QString& pattern : keywordPatterns)
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
	rule.format = classFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b[a-zA-Z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("//[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
	rule.format = numberFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegularExpression("/\\*");
	commentEndExpression = QRegularExpression("\\*/");
}

void SyntaxHighlighter::setupJavaScriptHighlighting()
{
	HighlightingRule rule;

	QStringList keywordPatterns = { "\\bfunction\\b", "\\bvar\\b",   "\\blet\\b",       "\\bconst\\b",  "\\bif\\b",         "\\belse\\b",     "\\bfor\\b",
		                            "\\bwhile\\b",    "\\bdo\\b",    "\\bswitch\\b",    "\\bcase\\b",   "\\bbreak\\b",      "\\bcontinue\\b", "\\breturn\\b",
		                            "\\btry\\b",      "\\bcatch\\b", "\\bfinally\\b",   "\\bthrow\\b",  "\\bnew\\b",        "\\bthis\\b",     "\\btrue\\b",
		                            "\\bfalse\\b",    "\\bnull\\b",  "\\bundefined\\b", "\\btypeof\\b", "\\binstanceof\\b", "\\bin\\b",       "\\bclass\\b",
		                            "\\bextends\\b",  "\\bsuper\\b", "\\bimport\\b",    "\\bexport\\b", "\\bdefault\\b",    "\\basync\\b",    "\\bawait\\b" };

	for (const QString& pattern : keywordPatterns)
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	rule.pattern = QRegularExpression("\\bfunction\\s+(\\w+)|(\\w+)\\s*:\\s*function");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\".*\"|'.*'|`.*`");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("//[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
	rule.format = numberFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegularExpression("/\\*");
	commentEndExpression = QRegularExpression("\\*/");
}

void SyntaxHighlighter::setupHtmlHighlighting()
{
	HighlightingRule rule;

	rule.pattern = QRegularExpression("<[^>]+>");
	rule.format = keywordFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("&[a-zA-Z]+;");
	rule.format = classFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegularExpression("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegularExpression("<!--");
	commentEndExpression = QRegularExpression("-->");
}
