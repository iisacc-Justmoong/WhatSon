#include "WhatSonHubTagsDepthProvider.hpp"

#include "WhatSonDebugTrace.hpp"
#include "note/WhatSonNoteHeaderParser.hpp"
#include "note/WhatSonNoteHeaderStore.hpp"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <utility>

namespace
{
    QString boolToText(bool value)
    {
        return value ? QStringLiteral("true") : QStringLiteral("false");
    }

    QString normalizeHubPath(const QString& input)
    {
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }

    bool resolveContentsDirectories(
        const QString& wshubPath,
        QStringList* outContentsDirectories,
        QString* errorMessage)
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.depth"),
            QStringLiteral("resolveContents.begin"),
            QStringLiteral("path=%1").arg(wshubPath));
        if (outContentsDirectories == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outContentsDirectories must not be null.");
            }
            return false;
        }

        const QString hubRootPath = normalizeHubPath(wshubPath);
        if (hubRootPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must not be empty.");
            }
            return false;
        }

        const QFileInfo hubInfo(hubRootPath);
        if (!hubInfo.exists())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(hubRootPath);
            }
            return false;
        }
        if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Path is not a .wshub package: %1").arg(hubRootPath);
            }
            return false;
        }
        if (!hubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage =
                    QStringLiteral("Only unpacked .wshub directories are supported: %1").arg(hubRootPath);
            }
            return false;
        }

        const QDir hubDir(hubRootPath);
        QStringList contentsDirectories;

        const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents"));
        if (QFileInfo(fixedInternalPath).isDir())
        {
            contentsDirectories.push_back(QDir::cleanPath(fixedInternalPath));
        }

        const QStringList dynamicContentsDirectories = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicContentsDirectories)
        {
            contentsDirectories.push_back(QDir::cleanPath(hubDir.filePath(directoryName)));
        }

        if (contentsDirectories.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("No *.wscontents directory was found inside .wshub: %1").arg(
                    hubRootPath);
            }
            return false;
        }

        contentsDirectories.removeDuplicates();
        *outContentsDirectories = contentsDirectories;
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.depth"),
            QStringLiteral("resolveContents.success"),
            QStringLiteral("count=%1").arg(outContentsDirectories->size()));
        return true;
    }

    bool parseTagsWstags(
        const QStringList& contentsDirectories,
        const WhatSonTagsFileReader& fileReader,
        const WhatSonTagsJsonParser& jsonParser,
        const WhatSonTagsDepthFlattener& depthFlattener,
        QVector<WhatSonTagDepthEntry>* outEntries,
        QString* errorMessage)
    {
        if (outEntries == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outEntries must not be null.");
            }
            return false;
        }

        for (const QString& contentsDirectory : contentsDirectories)
        {
            const QString tagsPath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
            if (!QFileInfo(tagsPath).isFile())
            {
                WhatSon::Debug::trace(
                    QStringLiteral("hub.tags.depth"),
                    QStringLiteral("parseTags.fileMissing"),
                    QStringLiteral("path=%1").arg(tagsPath));
                continue;
            }
            WhatSon::Debug::trace(
                QStringLiteral("hub.tags.depth"),
                QStringLiteral("parseTags.fileFound"),
                QStringLiteral("path=%1").arg(tagsPath));

            QString rawJson;
            if (!fileReader.readTextFile(tagsPath, &rawJson, errorMessage))
            {
                return false;
            }

            QJsonArray rootTags;
            if (!jsonParser.parseRootTags(rawJson, &rootTags, errorMessage))
            {
                return false;
            }

            QVector<WhatSonTagDepthEntry> flattenedEntries = depthFlattener.flatten(rootTags);
            for (WhatSonTagDepthEntry& entry : flattenedEntries)
            {
                entry.id = entry.id.trimmed();
                entry.label = entry.label.trimmed();
                if (entry.id.isEmpty())
                {
                    entry.id = entry.label;
                }
                if (entry.label.isEmpty())
                {
                    entry.label = entry.id;
                }
            }

            *outEntries = std::move(flattenedEntries);
            WhatSon::Debug::trace(
                QStringLiteral("hub.tags.depth"),
                QStringLiteral("parseTags.success"),
                QStringLiteral("entryCount=%1").arg(outEntries->size()));
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Tags.wstags was not found inside .wshub contents directories.");
        }
        return false;
    }

    QVector<WhatSonTagDepthEntry> parseNoteTagEntries(
        const QString& wsnHeadPath,
        const QString& wsnHeadText)
    {
        WhatSonNoteHeaderParser parser;
        WhatSonNoteHeaderStore store;
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.depth"),
            QStringLiteral("parseNoteHeader.begin"),
            QStringLiteral("file=%1 bytes=%2").arg(wsnHeadPath).arg(wsnHeadText.size()));
        QString parseError;
        if (!parser.parse(wsnHeadText, &store, &parseError))
        {
            qWarning().noquote()
                << QStringLiteral("[wsnhead:index] parse failed: file=%1 error=%2")
                .arg(wsnHeadPath, parseError);
            return {};
        }

        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.depth"),
            QStringLiteral("parseNoteHeader.success"),
            QStringLiteral("file=%1 tagCount=%2 folderCount=%3 progress=%4")
            .arg(wsnHeadPath)
            .arg(store.tags().size())
            .arg(store.folders().size())
            .arg(store.progress()));

        if (WhatSon::Debug::isEnabled())
        {
            qWarning().noquote()
                << QStringLiteral(
                    "[wsnhead:index] file=%1 id=%2 created=%3 author=%4 lastModified=%5 modifiedBy=%6 project=%7 bookmarked=%8 preset=%9 progress=%10 folders=[%11] tags=[%12]")
                .arg(
                    wsnHeadPath,
                    store.noteId(),
                    store.createdAt(),
                    store.author(),
                    store.lastModifiedAt(),
                    store.modifiedBy(),
                    store.project(),
                    boolToText(store.isBookmarked()),
                    boolToText(store.isPreset()),
                    QString::number(store.progress()),
                    store.folders().join(QStringLiteral(", ")),
                    store.tags().join(QStringLiteral(", ")));
        }

        QVector<WhatSonTagDepthEntry> entries;
        for (const QString& tag : store.tags())
        {
            const QString rawTagValue = tag.trimmed();
            if (rawTagValue.isEmpty())
            {
                continue;
            }

            WhatSonTagDepthEntry entry;
            entry.id = rawTagValue;
            entry.label = rawTagValue;
            entry.depth = 0;
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    QVector<WhatSonTagDepthEntry> parseTagEntriesFromNoteHeaders(
        const QStringList& contentsDirectories,
        QString* errorMessage)
    {
        QVector<WhatSonTagDepthEntry> entries;
        QSet<QString> dedupKeys;

        bool wsnHeadFound = false;
        for (const QString& contentsDirectory : contentsDirectories)
        {
            QDirIterator iterator(
                contentsDirectory,
                QStringList{QStringLiteral("*.wsnhead")},
                QDir::Files,
                QDirIterator::Subdirectories);
            WhatSon::Debug::trace(
                QStringLiteral("hub.tags.depth"),
                QStringLiteral("scanWsnhead.directory"),
                QStringLiteral("path=%1").arg(contentsDirectory));

            while (iterator.hasNext())
            {
                const QString wsnHeadPath = iterator.next();
                wsnHeadFound = true;
                WhatSon::Debug::trace(
                    QStringLiteral("hub.tags.depth"),
                    QStringLiteral("scanWsnhead.file"),
                    QStringLiteral("path=%1").arg(wsnHeadPath));

                QFile file(wsnHeadPath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    continue;
                }
                const QString fileContents = QString::fromUtf8(file.readAll());

                const QVector<WhatSonTagDepthEntry> parsedEntries =
                    parseNoteTagEntries(wsnHeadPath, fileContents);
                for (const WhatSonTagDepthEntry& parsedEntry : parsedEntries)
                {
                    const QString dedupKey = parsedEntry.id.trimmed().toCaseFolded();
                    if (dedupKey.isEmpty() || dedupKeys.contains(dedupKey))
                    {
                        continue;
                    }
                    dedupKeys.insert(dedupKey);
                    entries.push_back(parsedEntry);
                }
            }
        }

        if (!wsnHeadFound && errorMessage != nullptr)
        {
            *errorMessage =
                QStringLiteral("No .wsnhead files were found under .wshub contents directories.");
        }

        return entries;
    }

    QString resolveTagsWstagsPath(const QStringList& contentsDirectories)
    {
        for (const QString& contentsDirectory : contentsDirectories)
        {
            const QString candidatePath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
            if (QFileInfo(candidatePath).isFile())
            {
                return candidatePath;
            }
        }

        if (!contentsDirectories.isEmpty())
        {
            return QDir(contentsDirectories.first()).filePath(QStringLiteral("Tags.wstags"));
        }

        return {};
    }

    bool writeTagsWstags(
        const QStringList& contentsDirectories,
        const QVector<WhatSonTagDepthEntry>& entries,
        QString* errorMessage)
    {
        const QString tagsPath = resolveTagsWstagsPath(contentsDirectories);
        if (tagsPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Unable to resolve Tags.wstags path.");
            }
            return false;
        }

        QJsonArray tagsArray;
        for (const WhatSonTagDepthEntry& entry : entries)
        {
            const QString id = entry.id.trimmed();
            const QString label = entry.label.trimmed().isEmpty() ? id : entry.label.trimmed();
            if (id.isEmpty())
            {
                continue;
            }

            QJsonObject tagObject;
            tagObject.insert(QStringLiteral("id"), id);
            tagObject.insert(QStringLiteral("label"), label);
            tagsArray.append(tagObject);
        }

        QJsonObject rootObject;
        rootObject.insert(QStringLiteral("version"), 1);
        rootObject.insert(QStringLiteral("schema"), QStringLiteral("whatson.tags.tree"));
        rootObject.insert(QStringLiteral("tags"), tagsArray);

        QFile file(tagsPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write Tags.wstags: %1").arg(tagsPath);
            }
            return false;
        }

        const QJsonDocument document(rootObject);
        file.write(document.toJson(QJsonDocument::Indented));
        file.close();
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.depth"),
            QStringLiteral("writeTags.success"),
            QStringLiteral("path=%1 count=%2").arg(tagsPath).arg(tagsArray.size()));
        return true;
    }
} // namespace

WhatSonHubTagsDepthProvider::WhatSonHubTagsDepthProvider() = default;

WhatSonHubTagsDepthProvider::~WhatSonHubTagsDepthProvider() = default;

bool WhatSonHubTagsDepthProvider::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.depth"),
                              QStringLiteral("load.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    QStringList contentsDirectories;
    QString contentsError;
    if (!resolveContentsDirectories(wshubPath, &contentsDirectories, &contentsError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = contentsError;
        }
        return false;
    }

    QVector<WhatSonTagDepthEntry> tagsFileEntries;
    QString tagsFileError;
    if (parseTagsWstags(
        contentsDirectories,
        m_fileReader,
        m_jsonParser,
        m_depthFlattener,
        &tagsFileEntries,
        &tagsFileError))
    {
        m_entries = std::move(tagsFileEntries);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.tags.depth"),
                                  QStringLiteral("load.success.fromTags"),
                                  QStringLiteral("entryCount=%1").arg(m_entries.size()));
        return true;
    }

    QString noteHeadersError;
    const QVector<WhatSonTagDepthEntry> noteHeaderEntries =
        parseTagEntriesFromNoteHeaders(contentsDirectories, &noteHeadersError);
    if (!noteHeaderEntries.isEmpty())
    {
        QString writeError;
        if (!writeTagsWstags(contentsDirectories, noteHeaderEntries, &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }

        QVector<WhatSonTagDepthEntry> syncedTagsFileEntries;
        QString syncedTagsFileError;
        if (!parseTagsWstags(
            contentsDirectories,
            m_fileReader,
            m_jsonParser,
            m_depthFlattener,
            &syncedTagsFileEntries,
            &syncedTagsFileError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral(
                        "Failed to parse Tags.wstags after note-header sync: %1")
                    .arg(syncedTagsFileError);
            }
            return false;
        }

        m_entries = std::move(syncedTagsFileEntries);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.tags.depth"),
                                  QStringLiteral("load.success.fromWsnhead"),
                                  QStringLiteral("entryCount=%1").arg(m_entries.size()));
        return true;
    }

    if (errorMessage != nullptr)
    {
        if (!noteHeadersError.isEmpty())
        {
            *errorMessage = QStringLiteral(
                    "Failed to load tags from Tags.wstags (%1) and note headers (%2).")
                .arg(tagsFileError, noteHeadersError);
        }
        else
        {
            *errorMessage = tagsFileError;
        }
    }
    return false;
}

QVector<WhatSonTagDepthEntry> WhatSonHubTagsDepthProvider::tagDepthEntries() const
{
    return m_entries;
}

void WhatSonHubTagsDepthProvider::clear()
{
    m_entries.clear();
}
