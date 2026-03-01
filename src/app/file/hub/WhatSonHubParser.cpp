#include "WhatSonHubParser.hpp"

#include "WhatSonDebugTrace.hpp"
#include "hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.hpp"
#include "hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.hpp"
#include "hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "hierarchy/library/WhatSonLibraryHierarchyParser.hpp"
#include "hierarchy/library/WhatSonLibraryHierarchyStore.hpp"
#include "hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "hierarchy/tags/WhatSonTagsHierarchyStore.hpp"

#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include <algorithm>

namespace
{
    QString cleanIfExists(const QString& path)
    {
        if (path.trimmed().isEmpty())
        {
            return {};
        }
        if (!QFileInfo(path).exists())
        {
            return {};
        }
        return QDir::cleanPath(path);
    }

    QStringList sanitizeStringList(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());
        for (QString& value : values)
        {
            value = value.trimmed();
            if (value.isEmpty())
            {
                continue;
            }
            if (!sanitized.contains(value))
            {
                sanitized.push_back(value);
            }
        }
        return sanitized;
    }
} // namespace

WhatSonHubParser::WhatSonHubParser(QObject* parent)
    : QObject(parent)
{
}

WhatSonHubParser::~WhatSonHubParser() = default;

bool WhatSonHubParser::parseFromWshub(
    const QString& wshubPath,
    WhatSonHubStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.parser"),
        QStringLiteral("parse.begin"),
        QStringLiteral("path=%1").arg(wshubPath));

    if (outStore == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outStore must not be null.");
        }
        return false;
    }
    outStore->clear();

    const QString normalized = normalizeHubPath(wshubPath);
    if (normalized.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must not be empty.");
        }
        return false;
    }

    const QFileInfo hubInfo(normalized);
    if (!hubInfo.exists() || !hubInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must be an existing directory: %1").arg(normalized);
        }
        return false;
    }
    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Path is not a .wshub directory: %1").arg(normalized);
        }
        return false;
    }

    const QString hubName = hubInfo.completeBaseName().trimmed();
    const QDir hubDir(normalized);

    const QString contentsPath = resolvePrimaryDirectory(
        hubDir,
        QStringLiteral(".wscontents"),
        QStringLiteral("*.wscontents"));
    if (contentsPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No *.wscontents directory found inside hub: %1").arg(normalized);
        }
        return false;
    }

    QString libraryPath = cleanIfExists(QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary")));
    if (libraryPath.isEmpty())
    {
        const QDir contentsDir(contentsPath);
        const QStringList libraryCandidates = contentsDir.entryList(
            QStringList{QStringLiteral("*.wslibrary")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        if (!libraryCandidates.isEmpty())
        {
            libraryPath = QDir::cleanPath(contentsDir.filePath(libraryCandidates.first()));
        }
    }
    if (libraryPath.isEmpty() || !QFileInfo(libraryPath).isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Library.wslibrary directory is missing: %1").arg(contentsPath);
        }
        return false;
    }

    const QString resourcesPath = resolvePrimaryDirectory(
        hubDir,
        QStringLiteral(".wsresources"),
        QStringLiteral("*.wsresources"));
    if (resourcesPath.isEmpty() || !QFileInfo(resourcesPath).isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No *.wsresources directory found inside hub: %1").arg(normalized);
        }
        return false;
    }

    const QString statPath = resolveStatPath(hubDir, hubName);
    if (statPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No *.wsstat file found inside hub: %1").arg(normalized);
        }
        return false;
    }

    QString fileCheckError;
    requireEntryPath(contentsPath, QStringLiteral("Folders.wsfolders"), false, true, &fileCheckError);
    requireEntryPath(contentsPath, QStringLiteral("ProjectLists.wsproj"), false, true, &fileCheckError);
    requireEntryPath(contentsPath, QStringLiteral("Bookmarks.wsbookmarks"), false, true, &fileCheckError);
    requireEntryPath(contentsPath, QStringLiteral("Tags.wstags"), false, true, &fileCheckError);
    requireEntryPath(contentsPath, QStringLiteral("Progress.wsprogress"), false, true, &fileCheckError);
    requireEntryPath(contentsPath, QStringLiteral("Preset.wspreset"), true, true, &fileCheckError);
    requireEntryPath(libraryPath, QStringLiteral("index.wsnindex"), false, true, &fileCheckError);
    if (!fileCheckError.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = fileCheckError;
        }
        return false;
    }

    QString statText;
    if (!readUtf8File(statPath, &statText, errorMessage))
    {
        return false;
    }

    WhatSonHubStat stat;
    if (!parseStatText(statText, hubName, libraryPath, resourcesPath, &stat, errorMessage))
    {
        return false;
    }

    QString domainError;
    const QVariantMap domainPayload = buildDomainPayload(contentsPath, libraryPath, resourcesPath, &domainError);
    if (!domainError.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = domainError;
        }
        return false;
    }

    outStore->setHubPath(normalized);
    outStore->setHubName(hubName);
    outStore->setContentsPath(contentsPath);
    outStore->setLibraryPath(libraryPath);
    outStore->setResourcesPath(resourcesPath);
    outStore->setStatPath(statPath);
    outStore->setStat(std::move(stat));
    outStore->setDomainValues(domainPayload);

    WhatSon::Debug::trace(
        QStringLiteral("hub.parser"),
        QStringLiteral("parse.success"),
        QStringLiteral("hub=%1 domains=%2")
        .arg(outStore->hubPath())
        .arg(outStore->domainValues().size()));
    return true;
}

QString WhatSonHubParser::normalizeHubPath(const QString& hubPath)
{
    const QString trimmed = hubPath.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }
    return QDir::cleanPath(trimmed);
}

void WhatSonHubParser::requestParseFromWshub(const QString& wshubPath)
{
    WhatSonHubStore store;
    QString errorMessage;
    if (!parseFromWshub(wshubPath, &store, &errorMessage))
    {
        emit parseFailed(normalizeHubPath(wshubPath), errorMessage);
        return;
    }

    emit hubParsed(store.hubPath(), buildHubPayload(store));
    emit hubStatParsed(store.hubPath(), buildStatPayload(store.stat()));
    emit hubDomainsParsed(store.hubPath(), store.domainValues());
}

QString WhatSonHubParser::resolvePrimaryDirectory(
    const QDir& hubDir,
    const QString& fixedDirectoryName,
    const QString& dynamicPattern)
{
    const QString fixedPath = cleanIfExists(hubDir.filePath(fixedDirectoryName));
    if (!fixedPath.isEmpty() && QFileInfo(fixedPath).isDir())
    {
        return fixedPath;
    }

    const QStringList candidates = hubDir.entryList(
        QStringList{dynamicPattern},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    if (!candidates.isEmpty())
    {
        return QDir::cleanPath(hubDir.filePath(candidates.first()));
    }
    return {};
}

QString WhatSonHubParser::resolveStatPath(const QDir& hubDir, const QString& hubName)
{
    const QString preferred = cleanIfExists(hubDir.filePath(hubName + QStringLiteral("Stat.wsstat")));
    if (!preferred.isEmpty() && QFileInfo(preferred).isFile())
    {
        return preferred;
    }

    const QStringList candidates = hubDir.entryList(
        QStringList{QStringLiteral("*.wsstat")},
        QDir::Files,
        QDir::Name);
    if (!candidates.isEmpty())
    {
        return QDir::cleanPath(hubDir.filePath(candidates.first()));
    }
    return {};
}

QString WhatSonHubParser::firstString(const QJsonObject& object, const QStringList& keys)
{
    for (const QString& key : keys)
    {
        const QJsonValue value = object.value(key);
        if (!value.isString())
        {
            continue;
        }

        const QString text = value.toString().trimmed();
        if (!text.isEmpty())
        {
            return text;
        }
    }
    return {};
}

int WhatSonHubParser::firstInt(
    const QJsonObject& object,
    const QStringList& keys,
    int fallbackValue)
{
    for (const QString& key : keys)
    {
        const QJsonValue value = object.value(key);
        if (value.isDouble())
        {
            return std::max(0, value.toInt());
        }
        if (value.isString())
        {
            bool converted = false;
            const int parsed = value.toString().trimmed().toInt(&converted);
            if (converted)
            {
                return std::max(0, parsed);
            }
        }
    }
    return fallbackValue;
}

QStringList WhatSonHubParser::firstStringList(const QJsonObject& object, const QStringList& keys)
{
    for (const QString& key : keys)
    {
        const QJsonValue value = object.value(key);
        if (value.isArray())
        {
            QStringList values;
            for (const QJsonValue& entry : value.toArray())
            {
                if (!entry.isString())
                {
                    continue;
                }
                const QString text = entry.toString().trimmed();
                if (!text.isEmpty() && !values.contains(text))
                {
                    values.push_back(text);
                }
            }
            if (!values.isEmpty())
            {
                return values;
            }
        }
        else if (value.isString())
        {
            const QString text = value.toString().trimmed();
            if (!text.isEmpty())
            {
                return QStringList{text};
            }
        }
    }
    return {};
}

QVariantMap WhatSonHubParser::firstObjectMap(const QJsonObject& object, const QStringList& keys)
{
    for (const QString& key : keys)
    {
        const QJsonValue value = object.value(key);
        if (!value.isObject())
        {
            continue;
        }

        QVariantMap parsed;
        const QJsonObject source = value.toObject();
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            const QString mapKey = it.key().trimmed();
            const QString mapValue = it.value().toString().trimmed();
            if (mapKey.isEmpty() || mapValue.isEmpty())
            {
                continue;
            }
            parsed.insert(mapKey, mapValue);
        }
        if (!parsed.isEmpty())
        {
            return parsed;
        }
    }
    return {};
}

int WhatSonHubParser::countFilesRecursive(const QString& rootPath, const QStringList& nameFilters)
{
    if (rootPath.trimmed().isEmpty() || !QFileInfo(rootPath).isDir())
    {
        return 0;
    }

    QDirIterator iterator(
        rootPath,
        nameFilters,
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);

    int count = 0;
    while (iterator.hasNext())
    {
        iterator.next();
        ++count;
    }
    return count;
}

int WhatSonHubParser::countNoteDirectories(const QString& libraryPath)
{
    if (libraryPath.trimmed().isEmpty() || !QFileInfo(libraryPath).isDir())
    {
        return 0;
    }

    QDirIterator iterator(
        libraryPath,
        QStringList{QStringLiteral("*.wsnote")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);

    int count = 0;
    while (iterator.hasNext())
    {
        iterator.next();
        ++count;
    }
    return count;
}

int WhatSonHubParser::countWsnbodyCharacters(const QString& libraryPath)
{
    if (libraryPath.trimmed().isEmpty() || !QFileInfo(libraryPath).isDir())
    {
        return 0;
    }

    QDirIterator iterator(
        libraryPath,
        QStringList{QStringLiteral("*.wsnbody")},
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);

    int count = 0;
    while (iterator.hasNext())
    {
        iterator.next();
        QFile bodyFile(iterator.filePath());
        if (!bodyFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }
        count += QString::fromUtf8(bodyFile.readAll()).size();
    }
    return count;
}

QStringList WhatSonHubParser::listRelativeFilesRecursive(const QString& rootPath)
{
    QStringList relativeFiles;
    if (rootPath.trimmed().isEmpty() || !QFileInfo(rootPath).isDir())
    {
        return relativeFiles;
    }

    const QDir rootDir(rootPath);
    QDirIterator iterator(
        rootPath,
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        relativeFiles.push_back(rootDir.relativeFilePath(iterator.filePath()));
    }
    relativeFiles = sanitizeStringList(std::move(relativeFiles));
    relativeFiles.sort();
    return relativeFiles;
}

QVariantMap WhatSonHubParser::buildHubPayload(const WhatSonHubStore& store)
{
    QVariantMap payload;
    payload.insert(QStringLiteral("hubPath"), store.hubPath());
    payload.insert(QStringLiteral("hubName"), store.hubName());
    payload.insert(QStringLiteral("contentsPath"), store.contentsPath());
    payload.insert(QStringLiteral("libraryPath"), store.libraryPath());
    payload.insert(QStringLiteral("resourcesPath"), store.resourcesPath());
    payload.insert(QStringLiteral("statPath"), store.statPath());
    return payload;
}

QVariantMap WhatSonHubParser::buildStatPayload(const WhatSonHubStat& stat)
{
    QVariantMap payload;
    payload.insert(QStringLiteral("noteCount"), stat.noteCount());
    payload.insert(QStringLiteral("resourceCount"), stat.resourceCount());
    payload.insert(QStringLiteral("characterCount"), stat.characterCount());
    payload.insert(QStringLiteral("createdAtUtc"), stat.createdAtUtc());
    payload.insert(QStringLiteral("lastModifiedAtUtc"), stat.lastModifiedAtUtc());
    payload.insert(QStringLiteral("participants"), stat.participants());
    payload.insert(QStringLiteral("profileLastModifiedAtUtc"), stat.profileLastModifiedAtUtc());
    return payload;
}

QVariantMap WhatSonHubParser::buildDomainPayload(
    const QString& contentsPath,
    const QString& libraryPath,
    const QString& resourcesPath,
    QString* errorMessage)
{
    QVariantMap payload;

    const QString foldersPath =
        requireEntryPath(contentsPath, QStringLiteral("Folders.wsfolders"), false, true, errorMessage);
    if (foldersPath.isEmpty())
    {
        return {};
    }
    const QString projectListsPath =
        requireEntryPath(contentsPath, QStringLiteral("ProjectLists.wsproj"), false, true, errorMessage);
    if (projectListsPath.isEmpty())
    {
        return {};
    }
    const QString bookmarksPath =
        requireEntryPath(contentsPath, QStringLiteral("Bookmarks.wsbookmarks"), false, true, errorMessage);
    if (bookmarksPath.isEmpty())
    {
        return {};
    }
    const QString tagsPath =
        requireEntryPath(contentsPath, QStringLiteral("Tags.wstags"), false, true, errorMessage);
    if (tagsPath.isEmpty())
    {
        return {};
    }
    const QString progressPath =
        requireEntryPath(contentsPath, QStringLiteral("Progress.wsprogress"), false, true, errorMessage);
    if (progressPath.isEmpty())
    {
        return {};
    }
    const QString presetPath =
        requireEntryPath(contentsPath, QStringLiteral("Preset.wspreset"), true, true, errorMessage);
    if (presetPath.isEmpty())
    {
        return {};
    }
    const QString libraryIndexPath =
        requireEntryPath(libraryPath, QStringLiteral("index.wsnindex"), false, true, errorMessage);
    if (libraryIndexPath.isEmpty())
    {
        return {};
    }

    QString rawText;
    QString parseError;

    WhatSonProjectsHierarchyStore foldersStore;
    WhatSonProjectsHierarchyParser projectsParser;
    if (!readUtf8File(foldersPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!projectsParser.parse(rawText, &foldersStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("folders"), foldersStore.projectNames());
    payload.insert(QStringLiteral("folderEntries"), toFolderEntryList(foldersStore.folderEntries()));

    WhatSonProjectsHierarchyStore projectsStore;
    if (!readUtf8File(projectListsPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!projectsParser.parse(rawText, &projectsStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("projects"), projectsStore.projectNames());

    WhatSonBookmarksHierarchyStore bookmarksStore;
    WhatSonBookmarksHierarchyParser bookmarksParser;
    if (!readUtf8File(bookmarksPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!bookmarksParser.parse(rawText, &bookmarksStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("bookmarks"), bookmarksStore.bookmarkIds());
    payload.insert(QStringLiteral("bookmarkColorCriteriaHex"), bookmarksStore.bookmarkColorCriteriaHex());

    WhatSonTagsHierarchyStore tagsStore;
    WhatSonTagsHierarchyParser tagsParser;
    if (!readUtf8File(tagsPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!tagsParser.parse(rawText, &tagsStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("tagEntries"), toTagEntryList(tagsStore.tagEntries()));

    WhatSonProgressHierarchyStore progressStore;
    WhatSonProgressHierarchyParser progressParser;
    if (!readUtf8File(progressPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!progressParser.parse(rawText, &progressStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("progressValue"), progressStore.progressValue());
    payload.insert(QStringLiteral("progressStates"), progressStore.progressStates());

    QStringList presetNames;
    QFileInfo presetInfo(presetPath);
    if (presetInfo.isFile())
    {
        WhatSonPresetHierarchyStore presetStore;
        WhatSonPresetHierarchyParser presetParser;
        if (!readUtf8File(presetPath, &rawText, errorMessage))
        {
            return {};
        }
        if (!presetParser.parse(rawText, &presetStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return {};
        }
        presetNames = presetStore.presetNames();
        payload.insert(QStringLiteral("presetContainerType"), QStringLiteral("file"));
    }
    else
    {
        const QDir presetDir(presetPath);
        presetNames = presetDir.entryList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name);
        payload.insert(QStringLiteral("presetContainerType"), QStringLiteral("directory"));
    }
    payload.insert(QStringLiteral("presets"), sanitizeStringList(std::move(presetNames)));

    WhatSonEventHierarchyStore eventStore;
    const QString eventPath = QDir(contentsPath).filePath(QStringLiteral("Event.wsevent"));
    if (QFileInfo(eventPath).isFile())
    {
        WhatSonEventHierarchyParser eventParser;
        if (!readUtf8File(eventPath, &rawText, errorMessage))
        {
            return {};
        }
        if (!eventParser.parse(rawText, &eventStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return {};
        }
    }
    payload.insert(QStringLiteral("events"), eventStore.eventNames());

    WhatSonLibraryHierarchyStore libraryStore;
    WhatSonLibraryHierarchyParser libraryParser;
    if (!readUtf8File(libraryIndexPath, &rawText, errorMessage))
    {
        return {};
    }
    if (!libraryParser.parse(rawText, &libraryStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return {};
    }
    payload.insert(QStringLiteral("libraryNoteIds"), libraryStore.noteIds());

    const QStringList resourcePaths = listRelativeFilesRecursive(resourcesPath);
    payload.insert(QStringLiteral("resourcePaths"), resourcePaths);
    payload.insert(QStringLiteral("resourceFileCount"), resourcePaths.size());
    return payload;
}

bool WhatSonHubParser::readUtf8File(
    const QString& filePath,
    QString* outText,
    QString* errorMessage)
{
    if (outText == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outText must not be null.");
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file: %1").arg(filePath);
        }
        return false;
    }
    *outText = QString::fromUtf8(file.readAll());
    return true;
}

QString WhatSonHubParser::requireEntryPath(
    const QString& basePath,
    const QString& entryName,
    bool allowDirectory,
    bool allowFile,
    QString* errorMessage)
{
    const QString candidatePath = QDir(basePath).filePath(entryName);
    const QFileInfo info(candidatePath);
    const bool isValid = (allowDirectory && info.isDir()) || (allowFile && info.isFile());
    if (!isValid)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Missing required hub entry: %1").arg(QDir::cleanPath(candidatePath));
        }
        return {};
    }
    return QDir::cleanPath(candidatePath);
}

QVariantList WhatSonHubParser::toFolderEntryList(const QVector<WhatSonFolderDepthEntry>& entries)
{
    QVariantList values;
    values.reserve(entries.size());
    for (const WhatSonFolderDepthEntry& entry : entries)
    {
        values.push_back(QVariantMap{
            {QStringLiteral("id"), entry.id},
            {QStringLiteral("label"), entry.label},
            {QStringLiteral("depth"), entry.depth}
        });
    }
    return values;
}

QVariantList WhatSonHubParser::toTagEntryList(const QVector<WhatSonTagDepthEntry>& entries)
{
    QVariantList values;
    values.reserve(entries.size());
    for (const WhatSonTagDepthEntry& entry : entries)
    {
        values.push_back(QVariantMap{
            {QStringLiteral("id"), entry.id},
            {QStringLiteral("label"), entry.label},
            {QStringLiteral("depth"), entry.depth}
        });
    }
    return values;
}

bool WhatSonHubParser::parseStatText(
    const QString& rawText,
    const QString& hubName,
    const QString& libraryPath,
    const QString& resourcesPath,
    WhatSonHubStat* outStat,
    QString* errorMessage) const
{
    if (outStat == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outStat must not be null.");
        }
        return false;
    }
    outStat->clear();

    if (rawText.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(".wsstat file is empty.");
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Invalid .wsstat JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject root = document.object();

    int noteCount = firstInt(root, {
                                 QStringLiteral("noteCount"),
                                 QStringLiteral("notes"),
                                 QStringLiteral("totalNotes")
                             }, -1);
    if (noteCount < 0)
    {
        noteCount = countNoteDirectories(libraryPath);
    }

    int resourceCount = firstInt(root, {
                                     QStringLiteral("resourceCount"),
                                     QStringLiteral("resources"),
                                     QStringLiteral("totalResources")
                                 }, -1);
    if (resourceCount < 0)
    {
        resourceCount = countFilesRecursive(resourcesPath, {});
    }

    int characterCount = firstInt(root, {
                                      QStringLiteral("characterCount"),
                                      QStringLiteral("characters"),
                                      QStringLiteral("textLength")
                                  }, -1);
    if (characterCount < 0)
    {
        characterCount = countWsnbodyCharacters(libraryPath);
    }

    QString createdAtUtc = firstString(root, {
                                           QStringLiteral("createdAtUtc"),
                                           QStringLiteral("createdAt")
                                       });
    QString lastModifiedAtUtc = firstString(root, {
                                                QStringLiteral("lastModifiedAtUtc"),
                                                QStringLiteral("updatedAtUtc"),
                                                QStringLiteral("lastModifiedAt")
                                            });

    if (createdAtUtc.isEmpty() || lastModifiedAtUtc.isEmpty())
    {
        const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        if (createdAtUtc.isEmpty())
        {
            createdAtUtc = now;
        }
        if (lastModifiedAtUtc.isEmpty())
        {
            lastModifiedAtUtc = now;
        }
    }

    outStat->setNoteCount(noteCount);
    outStat->setResourceCount(resourceCount);
    outStat->setCharacterCount(characterCount);
    outStat->setCreatedAtUtc(createdAtUtc);
    outStat->setLastModifiedAtUtc(lastModifiedAtUtc);
    outStat->setParticipants(firstStringList(root, {
                                                 QStringLiteral("participants"),
                                                 QStringLiteral("members"),
                                                 QStringLiteral("profiles")
                                             }));
    outStat->setProfileLastModifiedAtUtc(firstObjectMap(root, {
                                                            QStringLiteral("profileLastModifiedAtUtc"),
                                                            QStringLiteral("profileModifiedAtUtc"),
                                                            QStringLiteral("participantLastModifiedAtUtc")
                                                        }));

    WhatSon::Debug::trace(
        QStringLiteral("hub.parser"),
        QStringLiteral("parseStatText.success"),
        QStringLiteral("hub=%1 noteCount=%2 resourceCount=%3 characterCount=%4")
        .arg(hubName)
        .arg(outStat->noteCount())
        .arg(outStat->resourceCount())
        .arg(outStat->characterCount()));
    return true;
}
