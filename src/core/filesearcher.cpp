#include "core/filesearcher.hpp"
#include "core/defines.hpp"
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>

namespace
{
	QString getDocumentsPath()
	{
		return QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
	}

	QString getAppDirPath()
	{
		return getDocumentsPath() + "/" + defines::projectName + "/";
	}

}; // namespace

FileSearcher::FileSearcher(QObject* parent) : QObject(parent), workingDir(getAppDirPath())
{
	qDebug() << "Current path: " << workingDir;
}

bool FileSearcher::saveFile(const QString& text)
{
	if (workingDir.isEmpty())
	{
		return false;
	}

	auto getFirstWord = [this](const QString& textToParse)
	{
		QRegularExpression re(R"(\b(\w+)\b)");
		QRegularExpressionMatch match = re.match(textToParse);
		QString firstWord = match.hasMatch() ? match.captured(1) : "";
		return firstWord;
	};
	if (filePath.isEmpty())
	{
		filePath = workingDir + getFirstWord(text);
	}

	// Ensure directory exists
	QFileInfo fileInfo(filePath);
	QDir dir = fileInfo.absoluteDir();
	if (!dir.exists())
	{
		if (!dir.mkpath("."))
		{
			qDebug() << "Failed to create directory:" << dir.absolutePath();
			return false;
		}
	}

	QFile fileToSave(filePath);
	if (!fileToSave.open(QIODeviceBase::WriteOnly | QIODevice::Text))
	{
		return false;
	}

	QTextStream outputToFile(&fileToSave);
	outputToFile << text;
	fileToSave.close();

	return true;
}

bool FileSearcher::saveFileAs(const QString& newFilePath, const QString& text)
{
	filePath = newFilePath;
	if (!saveFile(text))
	{
		return false;
	}

	return true;
}

QString FileSearcher::openFile(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODeviceBase::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Failed to open file:" << filePath;
		return QString();
	}

	QTextStream inputFromFile(&file);
	QString content = inputFromFile.readAll();
	file.close();

	this->filePath = filePath;
	return content;
}

void FileSearcher::setFilePath(const QString& path)
{
	filePath = path;
}

QString FileSearcher::getFilePath() const
{
	return filePath;
}

bool FileSearcher::removeAppDir()
{
	QString appDirPath = getAppDirPath();
	QDir appDir(appDirPath);

	if (!appDir.exists())
	{
		qDebug() << "Delete" << appDirPath << ": Fail / No such directory exists";
		return false;
	}

	if (!appDir.removeRecursively())
	{
		qDebug() << "Delete" << appDirPath << ": Fail / Can't delete directory or its content";
		return false;
	}

	qDebug() << "Remove" << appDirPath << ": Success";
	return true;
}