#include "gui/mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStatusBar>
#include <QPalette>
#include <QApplication>
#include <QFileInfo>
#include <QBrush>
#include <QColor>
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QFont>
#include <QStandardPaths>
#include <QDir>
#include <QComboBox>
#include <QTextEdit>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), fileSearcher(), syntaxHighlighter(nullptr), currentFilePath(QString()), isDarkTheme(false)
{
	ui->setupUi(this);
	syntaxHighlighter = new SyntaxHighlighter(ui->textEdit->document());
	setupUI();
	setupConnections();
	updateStatistics();
}

MainWindow::~MainWindow()
{
	delete syntaxHighlighter;
	delete ui;
}

void MainWindow::setupUI()
{
	statusBar()->showMessage("Ready");
	ui->textEdit->setFont(QFont("Consolas", 14));
}

void MainWindow::setupConnections()
{
	// File operations
	connect(ui->actionSave,
	        &QAction::triggered,
	        this,
	        [this]()
	        {
		        QString fileName = buildFileName();
		        if (fileName.isEmpty())
		        {
			        statusBar()->showMessage("Please enter a file name", 2000);
			        return;
		        }

		        QString extension = getFileExtension();
		        if (!extension.isEmpty() && extension != "No extension")
		        {
			        if (!fileName.endsWith(extension))
			        {
				        fileName += extension;
			        }
		        }

		        if (currentFilePath.isEmpty())
		        {
			        // First save - use default directory
			        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + "Noter/";
			        QDir dir;
			        if (!dir.exists(defaultDir))
			        {
				        dir.mkpath(defaultDir);
			        }
			        currentFilePath = defaultDir + fileName;
		        }
		        else
		        {
			        // Update filename if changed
			        QFileInfo fileInfo(currentFilePath);
			        currentFilePath = fileInfo.absolutePath() + "/" + fileName;
		        }

		        fileSearcher.setFilePath(currentFilePath);
		        auto text = ui->textEdit->toPlainText();
		        emit onSaveFile(text);
		        // Обновляем поле имени файла, показывая базовое имя без расширения
		        QFileInfo fileInfo(currentFilePath);
		        QString baseName = fileInfo.completeBaseName();
		        if (!baseName.isEmpty())
		        {
			        ui->lineEditFileName->setText(baseName);
		        }
		        statusBar()->showMessage("File saved: " + currentFilePath, 3000);
	        });

	connect(this, &MainWindow::onSaveFile, &fileSearcher, &FileSearcher::saveFile);
	connect(this, &MainWindow::onSaveFileAs, &fileSearcher, &FileSearcher::saveFileAs);

	connect(ui->actionSaveAs,
	        &QAction::triggered,
	        this,
	        [this]()
	        {
		        QString defaultFileName = buildFileName();
		        QString extension = getFileExtension();

		        // Build suggested filename with extension
		        if (!defaultFileName.isEmpty())
		        {
			        if (!extension.isEmpty() && extension != "No extension")
			        {
				        if (!defaultFileName.endsWith(extension))
				        {
					        defaultFileName += extension;
				        }
			        }
		        }
		        else
		        {
			        defaultFileName = "untitled";
			        if (!extension.isEmpty() && extension != "No extension")
			        {
				        defaultFileName += extension;
			        }
		        }

		        // Build filter based on extension
		        QString filter = "All Files (*.*)";
		        if (!extension.isEmpty() && extension != "No extension")
		        {
			        QString extUpper = extension.toUpper().mid(1); // Remove dot and uppercase
			        filter = QString("%1 Files (*%2);;All Files (*.*)").arg(extUpper).arg(extension);
		        }

		        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		        if (!currentFilePath.isEmpty())
		        {
			        QFileInfo fileInfo(currentFilePath);
			        defaultDir = fileInfo.absolutePath();
		        }

		        auto filePath = QFileDialog::getSaveFileName(this, "Save File", defaultDir + "/" + defaultFileName, filter);

		        if (!filePath.isEmpty())
		        {
			        // Ensure extension is added if not present
			        if (!extension.isEmpty() && extension != "No extension")
			        {
				        if (!filePath.endsWith(extension, Qt::CaseInsensitive))
				        {
					        filePath += extension;
				        }
			        }

			        auto text = ui->textEdit->toPlainText();
			        emit onSaveFileAs(filePath, text);
			        currentFilePath = filePath;
			        fileSearcher.setFilePath(filePath);
			        QFileInfo fileInfo(filePath);
			        QString baseName = fileInfo.completeBaseName();
			        QString fileName = fileInfo.fileName();
			        ui->lineEditFileName->setText(baseName.isEmpty() ? fileName : baseName);
			        updateExtensionFromFileName(fileName);
			        detectLanguageFromFileName(fileName);
		        }
	        });

	connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
	connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);

	// Text editing
	connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

	// Font controls
	connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &MainWindow::updateFont);
	connect(ui->spinBoxFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateFont);
	connect(ui->pushButtonBold, &QPushButton::clicked, this, &MainWindow::setBold);
	connect(ui->pushButtonItalic, &QPushButton::clicked, this, &MainWindow::setItalic);

	// File extension change
	connect(ui->comboBoxFileExtension,
	        QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this,
	        [this]()
	        {
		        QString extension = getFileExtension();
		        if (!extension.isEmpty())
		        {
			        QString lang = detectLanguageFromExtension("dummy" + extension);
			        syntaxHighlighter->setLanguage(lang);
		        }
	        });

	// Search and replace
	connect(ui->actionSearch, &QAction::triggered, this, &MainWindow::toggleSearchPanel);
	connect(ui->pushButtonSearch, &QPushButton::clicked, this, &MainWindow::onSearchText);
	connect(ui->pushButtonReplace, &QPushButton::clicked, this, &MainWindow::onReplaceText);
	connect(ui->pushButtonReplaceAll, &QPushButton::clicked, this, &MainWindow::onReplaceAll);
	connect(ui->lineEditSearch, &QLineEdit::returnPressed, this, &MainWindow::onSearchText);
	// Обновляем подсветку при изменении текста поиска
	connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::updateSearchHighlight);

	// Theme
	connect(ui->actionToggleTheme, &QAction::triggered, this, &MainWindow::toggleDarkTheme);

	// Edit actions
	connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
	connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
	connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
	connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
	connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
	connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAll);

	// File actions
	connect(ui->actionExit, &QAction::triggered, this, &MainWindow::closeApplication);
}

void MainWindow::onTextChanged()
{
	updateStatistics();
	// Не вызываем updateSearchHighlight() здесь, чтобы избежать конфликтов и рекурсии
}

void MainWindow::updateStatistics()
{
	QString text = ui->textEdit->toPlainText();
	int chars = text.length();
	int charsNoSpaces = text.remove(' ').length();
	int words = countWords(text);
	int lines = ui->textEdit->document()->blockCount();

	QString stats = QString("Words: %1 | Characters: %2 | Characters (no spaces): %3 | Lines: %4").arg(words).arg(chars).arg(charsNoSpaces).arg(lines);
	statusBar()->showMessage(stats);
}

int MainWindow::countWords(const QString& text)
{
	QRegularExpression re(R"(\b\w+\b)");
	int count = 0;
	QRegularExpressionMatchIterator i = re.globalMatch(text);
	while (i.hasNext())
	{
		i.next();
		count++;
	}
	return count;
}

void MainWindow::onOpenFile()
{
	QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	if (!currentFilePath.isEmpty())
	{
		QFileInfo fileInfo(currentFilePath);
		defaultDir = fileInfo.absolutePath();
	}

	QString filePath = QFileDialog::getOpenFileName(this, "Open File", defaultDir, "All Files (*.*)");
	if (!filePath.isEmpty())
	{
		QString content = fileSearcher.openFile(filePath);
		if (!content.isNull())
		{
			ui->textEdit->setPlainText(content);
			currentFilePath = filePath;
			QFileInfo fileInfo(filePath);
			QString baseName = fileInfo.completeBaseName(); // Имя без расширения
			QString fileName = fileInfo.fileName();         // Полное имя с расширением
			ui->lineEditFileName->setText(baseName.isEmpty() ? fileName : baseName);
			updateExtensionFromFileName(fileName);
			detectLanguageFromFileName(fileName);
			statusBar()->showMessage("File opened: " + filePath, 3000);
		}
		else
		{
			QMessageBox::warning(this, "Error", "Failed to open file: " + filePath);
		}
	}
}

void MainWindow::onNewFile()
{
	if (ui->textEdit->document()->isModified() || !ui->textEdit->toPlainText().isEmpty())
	{
		int ret = QMessageBox::question(
		    this, "New File", "Do you want to save changes to the current file?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
		{
			ui->actionSave->trigger();
		}
		else if (ret == QMessageBox::Cancel)
		{
			return;
		}
	}

	ui->textEdit->clear();
	currentFilePath.clear();
	fileSearcher.setFilePath("");
	ui->lineEditFileName->clear();
	ui->comboBoxFileExtension->setCurrentIndex(0); // Reset to .txt
	syntaxHighlighter->setLanguage("cpp");
}

void MainWindow::onSearchText()
{
	QString searchText = ui->lineEditSearch->text();
	if (searchText.isEmpty())
	{
		return;
	}

	QTextCursor cursor = ui->textEdit->textCursor();
	cursor.movePosition(QTextCursor::Start);
	ui->textEdit->setTextCursor(cursor);

	bool found = ui->textEdit->find(searchText);
	if (!found)
	{
		statusBar()->showMessage("Text not found", 2000);
	}
	else
	{
		statusBar()->showMessage("Text found", 2000);
	}
	updateSearchHighlight();
}

void MainWindow::onReplaceText()
{
	QString searchText = ui->lineEditSearch->text();
	QString replaceText = ui->lineEditReplace->text();

	if (searchText.isEmpty())
	{
		return;
	}

	QTextCursor cursor = ui->textEdit->textCursor();
	if (cursor.hasSelection() && cursor.selectedText() == searchText)
	{
		cursor.insertText(replaceText);
	}
	onSearchText();
	updateStatistics();
}

void MainWindow::onReplaceAll()
{
	QString searchText = ui->lineEditSearch->text();
	QString replaceText = ui->lineEditReplace->text();

	if (searchText.isEmpty())
	{
		return;
	}

	// Простой и безопасный способ: заменяем через QString
	QString documentText = ui->textEdit->toPlainText();
	int count = documentText.count(searchText);

	if (count == 0)
	{
		statusBar()->showMessage("Text not found", 2000);
		return;
	}

	// Сохраняем текущую позицию курсора
	QTextCursor originalCursor = ui->textEdit->textCursor();
	int originalPosition = originalCursor.position();

	// Выполняем замену
	documentText.replace(searchText, replaceText);

	// Устанавливаем новый текст
	ui->textEdit->setPlainText(documentText);

	// Перемещаем курсор в начало для удобства
	QTextCursor newCursor = ui->textEdit->textCursor();
	newCursor.movePosition(QTextCursor::Start);
	ui->textEdit->setTextCursor(newCursor);

	statusBar()->showMessage(QString("Replaced %1 occurrence(s)").arg(count), 3000);
	updateSearchHighlight();
	updateStatistics();
}

void MainWindow::toggleSearchPanel()
{
	bool visible = ui->searchPanel->isVisible();
	ui->searchPanel->setVisible(!visible);
	if (!visible)
	{
		ui->lineEditSearch->setFocus();
	}
}

void MainWindow::updateSearchHighlight()
{
	QString searchText = ui->lineEditSearch->text();

	// Очищаем предыдущие подсветки
	ui->textEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>());

	if (searchText.isEmpty())
	{
		return;
	}

	QTextDocument* document = ui->textEdit->document();
	QTextCharFormat highlightFormat;
	highlightFormat.setBackground(QBrush(QColor(255, 255, 0, 100)));

	QList<QTextEdit::ExtraSelection> extraSelections;
	QTextCursor cursor(document);
	cursor.movePosition(QTextCursor::Start);

	QRegularExpression re(QRegularExpression::escape(searchText), QRegularExpression::CaseInsensitiveOption);

	while (!cursor.isNull() && !cursor.atEnd())
	{
		cursor = document->find(re, cursor);
		if (!cursor.isNull())
		{
			QTextEdit::ExtraSelection selection;
			selection.cursor = cursor;
			selection.format = highlightFormat;
			extraSelections.append(selection);
		}
		else
		{
			break; // Больше не найдено
		}
	}

	ui->textEdit->setExtraSelections(extraSelections);
}

void MainWindow::toggleDarkTheme()
{
	isDarkTheme = !isDarkTheme;

	if (isDarkTheme)
	{
		QPalette darkPalette;
		darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
		darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
		darkPalette.setColor(QPalette::ToolTipText, Qt::white);
		darkPalette.setColor(QPalette::Text, Qt::white);
		darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::HighlightedText, Qt::black);

		qApp->setPalette(darkPalette);
		ui->textEdit->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #3c3c3c; }");
	}
	else
	{
		qApp->setPalette(QApplication::style()->standardPalette());
		ui->textEdit->setStyleSheet("");
	}

	statusBar()->showMessage(isDarkTheme ? "Dark theme enabled" : "Light theme enabled", 2000);
}

void MainWindow::updateFont()
{
	QFont font = ui->fontComboBox->currentFont();
	font.setPointSize(ui->spinBoxFontSize->value());
	ui->textEdit->setFont(font);
}

void MainWindow::setBold()
{
	QTextCursor cursor = ui->textEdit->textCursor();
	QTextCharFormat format;
	format.setFontWeight(cursor.charFormat().fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
	cursor.mergeCharFormat(format);
	ui->textEdit->setTextCursor(cursor);
}

void MainWindow::setItalic()
{
	QTextCursor cursor = ui->textEdit->textCursor();
	QTextCharFormat format;
	format.setFontItalic(!cursor.charFormat().fontItalic());
	cursor.mergeCharFormat(format);
	ui->textEdit->setTextCursor(cursor);
}

void MainWindow::detectLanguageFromFileName(const QString& fileName)
{
	QString lang = detectLanguageFromExtension(fileName);
	syntaxHighlighter->setLanguage(lang);
}

QString MainWindow::detectLanguageFromExtension(const QString& filePath)
{
	if (filePath.endsWith(".cpp") || filePath.endsWith(".cxx") || filePath.endsWith(".cc") || filePath.endsWith(".c"))
	{
		return "cpp";
	}
	else if (filePath.endsWith(".h") || filePath.endsWith(".hpp") || filePath.endsWith(".hxx"))
	{
		return "cpp";
	}
	else if (filePath.endsWith(".py"))
	{
		return "python";
	}
	else if (filePath.endsWith(".java"))
	{
		return "java";
	}
	else if (filePath.endsWith(".js"))
	{
		return "javascript";
	}
	else if (filePath.endsWith(".html") || filePath.endsWith(".xml"))
	{
		return "html";
	}
	return "cpp";
}

QString MainWindow::getFileExtension() const
{
	QString extension = ui->comboBoxFileExtension->currentText();
	if (extension == "No extension")
	{
		return QString();
	}
	return extension;
}

QString MainWindow::buildFileName() const
{
	QString fileName = ui->lineEditFileName->text().trimmed();

	// Remove extension if already present
	QString currentExt = getFileExtension();
	if (!currentExt.isEmpty() && fileName.endsWith(currentExt))
	{
		fileName = fileName.left(fileName.length() - currentExt.length());
	}

	return fileName;
}

void MainWindow::updateExtensionFromFileName(const QString& fileName)
{
	QFileInfo fileInfo(fileName);
	QString suffix = fileInfo.suffix();

	if (suffix.isEmpty())
	{
		// No extension - set to "No extension"
		int index = ui->comboBoxFileExtension->findText("No extension");
		if (index >= 0)
		{
			ui->comboBoxFileExtension->setCurrentIndex(index);
		}
	}
	else
	{
		// Find matching extension in combo box
		QString extWithDot = "." + suffix;
		int index = ui->comboBoxFileExtension->findText(extWithDot);
		if (index >= 0)
		{
			ui->comboBoxFileExtension->setCurrentIndex(index);
		}
		else
		{
			// Extension not in list, add it
			ui->comboBoxFileExtension->addItem(extWithDot);
			ui->comboBoxFileExtension->setCurrentIndex(ui->comboBoxFileExtension->count() - 1);
		}
	}
}

void MainWindow::undo()
{
	ui->textEdit->undo();
}

void MainWindow::redo()
{
	ui->textEdit->redo();
}

void MainWindow::cut()
{
	ui->textEdit->cut();
}

void MainWindow::copy()
{
	ui->textEdit->copy();
}

void MainWindow::paste()
{
	ui->textEdit->paste();
}

void MainWindow::selectAll()
{
	ui->textEdit->selectAll();
}

void MainWindow::closeApplication()
{
	close();
}
