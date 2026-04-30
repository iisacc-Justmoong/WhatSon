#include "app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.hpp"

#include "app/models/file/note/WhatSonNoteFolderBindingService.hpp"
#include "app/models/file/note/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/WhatSonNoteHeaderParser.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>

namespace
{
    QString normalizeNoteId(const QString& noteId)
    {
        return noteId.trimmed();
    }
}

WhatSonNoteHeaderSessionStore::WhatSonNoteHeaderSessionStore(QObject* parent)
    : IWhatSonNoteHeaderSessionStore(parent)
{
}

bool WhatSonNoteHeaderSessionStore::ensureLoaded(
    const QString& noteId,
    const QString& noteDirectoryPath,
    QString* errorMessage,
    const bool forceReload)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    const QString normalizedDirectoryPath = noteDirectoryPath.trimmed();
    if (normalizedNoteId.isEmpty() || normalizedDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note id and note directory path are required to load .wsnhead metadata.");
        }
        return false;
    }

    Entry& entry = m_entries[normalizedNoteId];
    if (!forceReload && entry.loaded && entry.noteDirectoryPath == normalizedDirectoryPath)
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    entry.noteId = normalizedNoteId;
    entry.noteDirectoryPath = normalizedDirectoryPath;
    entry.headerFilePath = resolveHeaderFilePath(normalizedDirectoryPath);
    entry.header.clear();
    entry.loaded = false;
    entry.dirty = false;

    QFile file(entry.headerFilePath);
    if (!file.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(".wsnhead file does not exist for note: %1").arg(entry.headerFilePath);
        }
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = file.errorString();
        }
        return false;
    }

    const QString text = QString::fromUtf8(file.readAll());
    file.close();

    WhatSonNoteHeaderParser parser;
    QString parseError;
    if (!parser.parse(text, &entry.header, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return false;
    }

    entry.loaded = true;
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    emit entryChanged(normalizedNoteId);
    return true;
}

bool WhatSonNoteHeaderSessionStore::hasEntry(const QString& noteId) const
{
    const Entry* existing = findEntry(noteId);
    return existing != nullptr && existing->loaded;
}

WhatSonNoteHeaderSessionStore::Entry WhatSonNoteHeaderSessionStore::entry(const QString& noteId) const
{
    const Entry* existing = findEntry(noteId);
    return existing != nullptr ? *existing : Entry{};
}

QString WhatSonNoteHeaderSessionStore::noteDirectoryPath(const QString& noteId) const
{
    const Entry* existing = findEntry(noteId);
    return existing != nullptr ? existing->noteDirectoryPath : QString();
}

QString WhatSonNoteHeaderSessionStore::headerFilePath(const QString& noteId) const
{
    const Entry* existing = findEntry(noteId);
    return existing != nullptr ? existing->headerFilePath : QString();
}

WhatSonNoteHeaderStore WhatSonNoteHeaderSessionStore::header(const QString& noteId) const
{
    const Entry* existing = findEntry(noteId);
    return existing != nullptr ? existing->header : WhatSonNoteHeaderStore{};
}

bool WhatSonNoteHeaderSessionStore::updateProject(const QString& noteId, const QString& project, QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    existing->header.setProject(project);
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::updateBookmarked(const QString& noteId, bool bookmarked, QStringList bookmarkColors, QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    existing->header.setBookmarked(bookmarked);
    existing->header.setBookmarkColors(std::move(bookmarkColors));
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::updateProgress(const QString& noteId, int progress, QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    existing->header.setProgress(progress);
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::assignFolderBinding(
    const QString& noteId,
    const QString& folderPath,
    const QString& folderUuid,
    QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    const WhatSonNoteFolderBindingService bindingService;
    const WhatSonNoteFolderBindingService::Bindings previousBindings = bindingService.bindings(
        existing->header.folders(),
        existing->header.folderUuids());
    const WhatSonNoteFolderBindingService::Bindings nextBindings = bindingService.assignFolder(
        previousBindings,
        folderPath,
        folderUuid);
    if (bindingService.matches(previousBindings, nextBindings))
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    existing->header.setFolderBindings(nextBindings.folders, nextBindings.folderUuids);
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::assignTag(
    const QString& noteId,
    const QString& tag,
    QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    const QString normalizedTag = tag.trimmed();
    if (normalizedTag.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Tag value is empty.");
        }
        return false;
    }

    QStringList tags = existing->header.tags();
    QSet<QString> seenTags;
    seenTags.reserve(tags.size());

    for (const QString& existingTag : tags)
    {
        seenTags.insert(existingTag.trimmed().toCaseFolded());
    }

    const QString normalizedTagKey = normalizedTag.toCaseFolded();
    if (seenTags.contains(normalizedTagKey))
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    tags.push_back(normalizedTag);
    existing->header.setTags(std::move(tags));
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::removeFolderAt(const QString& noteId, int index, QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    QStringList folders = existing->header.folders();
    QStringList folderUuids = existing->header.folderUuids();
    if (index < 0 || index >= folders.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Folder index is out of range.");
        }
        return false;
    }

    folders.removeAt(index);
    if (index >= 0 && index < folderUuids.size())
    {
        folderUuids.removeAt(index);
    }
    existing->header.setFolderBindings(std::move(folders), std::move(folderUuids));
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

bool WhatSonNoteHeaderSessionStore::removeTagAt(const QString& noteId, int index, QString* errorMessage)
{
    Entry* existing = findEntry(noteId);
    if (existing == nullptr || !existing->loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Note header session is not loaded.");
        }
        return false;
    }

    QStringList tags = existing->header.tags();
    if (index < 0 || index >= tags.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Tag index is out of range.");
        }
        return false;
    }

    tags.removeAt(index);
    existing->header.setTags(std::move(tags));
    existing->dirty = true;
    const bool saved = persistEntry(*existing, errorMessage);
    if (saved)
    {
        emit entryChanged(existing->noteId);
    }
    return saved;
}

QString WhatSonNoteHeaderSessionStore::resolveHeaderFilePath(const QString& noteDirectoryPath)
{
    const QString normalizedNoteDirectoryPath = QDir::cleanPath(noteDirectoryPath.trimmed());
    if (normalizedNoteDirectoryPath.isEmpty())
    {
        return {};
    }

    const QDir noteDirectory(normalizedNoteDirectoryPath);
    if (!noteDirectory.exists())
    {
        return {};
    }

    const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
    if (!noteStem.isEmpty())
    {
        const QString stemHeaderPath = noteDirectory.filePath(noteStem + QStringLiteral(".wsnhead"));
        if (QFileInfo(stemHeaderPath).isFile())
        {
            return QDir::cleanPath(stemHeaderPath);
        }
    }

    const QString canonicalHeaderPath = noteDirectory.filePath(QStringLiteral("note.wsnhead"));
    if (QFileInfo(canonicalHeaderPath).isFile())
    {
        return QDir::cleanPath(canonicalHeaderPath);
    }

    const QFileInfoList headerCandidates = noteDirectory.entryInfoList(
        QStringList{QStringLiteral("*.wsnhead")},
        QDir::Files,
        QDir::Name);
    QString draftHeaderPath;
    for (const QFileInfo& fileInfo : headerCandidates)
    {
        const QString loweredName = fileInfo.fileName().toCaseFolded();
        if (loweredName.contains(QStringLiteral(".draft.")))
        {
            if (draftHeaderPath.isEmpty())
            {
                draftHeaderPath = fileInfo.absoluteFilePath();
            }
            continue;
        }

        return QDir::cleanPath(fileInfo.absoluteFilePath());
    }

    if (!draftHeaderPath.isEmpty())
    {
        return QDir::cleanPath(draftHeaderPath);
    }

    if (!noteStem.isEmpty())
    {
        return QDir::cleanPath(noteDirectory.filePath(noteStem + QStringLiteral(".wsnhead")));
    }

    return QDir::cleanPath(noteDirectory.filePath(QStringLiteral("note.wsnhead")));
}

WhatSonNoteHeaderSessionStore::Entry* WhatSonNoteHeaderSessionStore::findEntry(const QString& noteId)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    auto it = m_entries.find(normalizedNoteId);
    return it == m_entries.end() ? nullptr : &it.value();
}

const WhatSonNoteHeaderSessionStore::Entry* WhatSonNoteHeaderSessionStore::findEntry(const QString& noteId) const
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    const auto it = m_entries.constFind(normalizedNoteId);
    return it == m_entries.cend() ? nullptr : &it.value();
}

bool WhatSonNoteHeaderSessionStore::persistEntry(Entry& entry, QString* errorMessage)
{
    WhatSonLocalNoteFileStore localNoteFileStore;
    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document.noteDirectoryPath = entry.noteDirectoryPath;
    updateRequest.document.noteHeaderPath = entry.headerFilePath;
    updateRequest.document.headerStore = entry.header;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = false;
    updateRequest.touchLastModified = true;

    WhatSonLocalNoteDocument persistedDocument;
    if (!localNoteFileStore.updateNote(std::move(updateRequest), &persistedDocument, errorMessage))
    {
        return false;
    }

    entry.header = persistedDocument.headerStore;
    entry.noteDirectoryPath = persistedDocument.noteDirectoryPath;
    entry.headerFilePath = persistedDocument.noteHeaderPath;
    entry.dirty = false;
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}
