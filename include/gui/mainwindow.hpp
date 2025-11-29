#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QStatusBar>
#include "core/filesearcher.hpp"
#include "core/syntaxhighlighter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

  private slots:
	void onTextChanged();
	void updateStatistics();
	void onOpenFile();
	void onNewFile();
	void onSearchText();
	void onReplaceText();
	void onReplaceAll();
	void toggleSearchPanel();
	void toggleDarkTheme();
	void updateFont();
	void setBold();
	void setItalic();
	void detectLanguageFromFileName(const QString& fileName);
	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void selectAll();
	void closeApplication();

  private:
	Ui::MainWindow* ui;
	FileSearcher fileSearcher;
	SyntaxHighlighter* syntaxHighlighter;
	QString currentFilePath;
	bool isDarkTheme;

	void setupUI();
	void setupConnections();
	void updateSearchHighlight();
	QString detectLanguageFromExtension(const QString& filePath);
	int countWords(const QString& text);
	QString getFileExtension() const;
	QString buildFileName() const;
	void updateExtensionFromFileName(const QString& fileName);

  signals:
	void onSaveFile(const QString& text);
	void onSaveFileAs(const QString& newFilePath, const QString& text);
};
