#pragma once

#include "WhatSonLocalNoteFileStore.hpp"
#include "file/IO/WhatSonSystemIoGateway.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QVector>

namespace WhatSon::NoteMutationSupport
{
    QString currentNoteTimestamp();
    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
    QString createUniqueNoteId(const QString& libraryPath, const QVector<LibraryNoteRecord>& existingNotes);
    bool ensureDirectoryPath(const QString& directoryPath, QString* errorMessage = nullptr);
    bool writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage = nullptr);
    bool removeFilePath(const QString& filePath, QString* errorMessage = nullptr);
    bool removeDirectoryPath(const QString& directoryPath, QString* errorMessage = nullptr);
    QString createAttachmentManifestText(const QString& noteId);
    QString createLinkManifestText(const QString& noteId, const QString& schema);
    QString resolveNoteHeaderPath(const LibraryNoteRecord& note);
    void syncNoteRecordFromDocument(LibraryNoteRecord* note, const WhatSonLocalNoteDocument& document);
} // namespace WhatSon::NoteMutationSupport
