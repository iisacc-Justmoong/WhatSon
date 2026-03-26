#include "WhatSonNoteHeaderSessionStore.hpp"

#include "file/note/WhatSonNoteHeaderCreator.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace
{
    QString normalizeNoteId(const QString& noteId)
    {
        return noteId.trimmed();
    }
}

WhatSonNoteHeaderSessionStore::WhatSonNoteHeaderSessionStore(QObject* parent)
    : QObject(parent)
{
}

bool WhatSonNoteHeaderSessionStore::ensureLoaded(const QString& noteId, const QString& noteDirectoryPath, QString* errorMessage)
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
    if (entry.loaded && entry.noteDirectoryPath == normalizedDirectoryPath)
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
    return findEntry(noteId) != nullptr;
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

QString WhatSonNoteHeaderSessionStore::resolveHeaderFilePath(const QString& noteDirectoryPath)
{
    const QFileInfo noteDirInfo(noteDirectoryPath.trimmed());
    const QString noteStem = noteDirInfo.completeBaseName();
    return QDir(noteDirInfo.absoluteFilePath()).filePath(QStringLiteral(".meta/%1.wsnhead").arg(noteStem));
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
    QDir().mkpath(QFileInfo(entry.headerFilePath).absolutePath());
    WhatSonNoteHeaderCreator creator(entry.noteDirectoryPath, QString());
    const QString text = creator.createHeaderText(entry.header);

    QFile file(entry.headerFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << text;
    file.close();
    entry.dirty = false;
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}
