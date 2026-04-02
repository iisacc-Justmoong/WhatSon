#pragma once

#include "WhatSonLocalNoteDocument.hpp"
#include "file/IO/WhatSonSystemIoGateway.hpp"

#include <QJsonObject>
#include <QString>

class WhatSonLocalNoteFileStore final
{
public:
    struct ReadRequest final
    {
        QString noteId;
        QString noteDirectoryPath;
        QString noteHeaderPath;
        QString noteBodyPath;
    };

    struct CreateRequest final
    {
        QString noteId;
        QString noteDirectoryPath;
        WhatSonNoteHeaderStore headerStore;
        QString bodyPlainText;
    };

    struct UpdateRequest final
    {
        WhatSonLocalNoteDocument document;
        bool persistHeader = true;
        bool persistBody = true;
        bool touchLastModified = false;
    };

    struct DeleteRequest final
    {
        QString noteDirectoryPath;
        QString noteHeaderPath;
        QString noteBodyPath;
    };

    WhatSonLocalNoteFileStore();
    ~WhatSonLocalNoteFileStore();

    bool createNote(CreateRequest request, WhatSonLocalNoteDocument* outDocument = nullptr, QString* errorMessage = nullptr) const;
    bool readNote(ReadRequest request, WhatSonLocalNoteDocument* outDocument, QString* errorMessage = nullptr) const;
    bool updateNote(UpdateRequest request, WhatSonLocalNoteDocument* outDocument = nullptr, QString* errorMessage = nullptr) const;
    bool deleteNote(DeleteRequest request, QString* errorMessage = nullptr) const;

private:
    QString normalizePath(QString path) const;
    QString resolveNoteStem(const QString& noteId, const QString& noteDirectoryPath) const;
    QString resolveNoteId(const QString& noteId, const QString& noteDirectoryPath, const WhatSonNoteHeaderStore* headerStore = nullptr) const;
    QString resolveDirectoryPath(const QString& noteDirectoryPath, const QString& noteHeaderPath, const QString& noteBodyPath) const;
    QString headerPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const;
    QString bodyPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const;
    QString versionPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const;
    QString paintPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const;
    QString currentNoteTimestamp() const;

    bool loadHeaderStore(const QString& headerPath, WhatSonNoteHeaderStore* outHeaderStore, QString* errorMessage = nullptr) const;
    void applyBodyDocumentText(const QString& bodyDocumentText, WhatSonLocalNoteDocument* document) const;

    WhatSonSystemIoGateway m_ioGateway;
};
