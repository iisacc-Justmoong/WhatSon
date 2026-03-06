#pragma once

#include <QString>

class WhatSonTxtFileCreator
{
public:
    explicit WhatSonTxtFileCreator(QString libraryRootPath = QString());
    ~WhatSonTxtFileCreator();

    void setLibraryRootPath(QString value);
    QString libraryRootPath() const;

    bool createFile(
        const QString& preferredBaseName = QString(),
        const QString& text = QString(),
        QString* outFilePath = nullptr,
        QString* errorMessage = nullptr) const;

private:
    QString sanitizedBaseName(const QString& value) const;
    QString uniqueFilePath(const QString& preferredBaseName) const;

    QString m_libraryRootPath;
};
