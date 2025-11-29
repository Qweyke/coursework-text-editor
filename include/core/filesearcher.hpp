#pragma once
#include <QObject>
#include <QString>

#include <QFileDialog>

class FileSearcher : public QObject
{
	Q_OBJECT

  private:
	QString workingDir;
	QString filePath;

	bool removeAppDir();

  public:
	explicit FileSearcher(QObject* parent = nullptr);
	~FileSearcher() = default;

  public slots:
	bool saveFile(const QString& text);
	bool saveFileAs(const QString& newFilePath, const QString& text);
	QString openFile(const QString& filePath);
	void setFilePath(const QString& path);
	QString getFilePath() const;

  signals:
};
