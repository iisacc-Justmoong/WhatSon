#include "LibraryAll.hpp"

#include "WhatSonDebugTrace.hpp"
#include "note/WhatSonNoteHeaderParser.hpp"
#include "note/WhatSonNoteHeaderStore.hpp"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QRegularExpression>

#include <utility>

namespace
{
    struct BodyContentExtract final
    {
        QString plainText;
        QString firstLine;
    };

    QString normalizePath(const QString& input)
    {
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }

    QString normalizeStorageKind(QString value)
    {
        value = value.trimmed().toCaseFolded();
        if (value.isEmpty())
        {
            return {};
        }
        return QStringLiteral("wsnote");
    }

    QStringList resolveContentsDirectories(const QString& wshubPath)
    {
        const QDir hubDir(wshubPath);
        QStringList directories;

        const QString fixedPath = hubDir.filePath(QStringLiteral(".wscontents"));
        if (QFileInfo(fixedPath).isDir())
        {
            directories.push_back(QDir::cleanPath(fixedPath));
        }

        const QStringList dynamicContents = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& dirName : dynamicContents)
        {
            directories.push_back(QDir::cleanPath(hubDir.filePath(dirName)));
        }

        directories.removeDuplicates();
        return directories;
    }

    QStringList resolveLibraryRoots(const QStringList& contentsDirectories)
    {
        QStringList roots;
        for (const QString& contentsDirectory : contentsDirectories)
        {
            const QDir contentsDir(contentsDirectory);
            const QString libraryPath = contentsDir.filePath(QStringLiteral("Library.wslibrary"));
            if (QFileInfo(libraryPath).isDir())
            {
                roots.push_back(QDir::cleanPath(libraryPath));
            }

            const QStringList dynamicLibraries = contentsDir.entryList(
                QStringList{QStringLiteral("*.wslibrary")},
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::Name);
            for (const QString& libraryDirName : dynamicLibraries)
            {
                roots.push_back(QDir::cleanPath(contentsDir.filePath(libraryDirName)));
            }
        }
        roots.removeDuplicates();
        return roots;
    }

    QString readUtf8File(const QString& filePath, QString* errorMessage)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("failed to open file: %1").arg(filePath);
            }
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    QString firstString(const QJsonObject& object, const QStringList& keys)
    {
        for (const QString& key : keys)
        {
            const QJsonValue value = object.value(key);
            if (value.isString())
            {
                const QString text = value.toString().trimmed();
                if (!text.isEmpty())
                {
                    return text;
                }
            }
        }
        return {};
    }

    int firstInt(const QJsonObject& object, const QStringList& keys, int fallbackValue)
    {
        for (const QString& key : keys)
        {
            const QJsonValue value = object.value(key);
            if (value.isDouble())
            {
                return value.toInt(fallbackValue);
            }
            if (value.isString())
            {
                bool converted = false;
                const int parsed = value.toString().trimmed().toInt(&converted);
                if (converted)
                {
                    return parsed;
                }
            }
        }
        return fallbackValue;
    }

    bool parseBooleanText(const QString& text, bool fallbackValue)
    {
        const QString normalized = text.trimmed().toCaseFolded();
        if (normalized == QStringLiteral("1")
            || normalized == QStringLiteral("true")
            || normalized == QStringLiteral("yes")
            || normalized == QStringLiteral("on"))
        {
            return true;
        }
        if (normalized == QStringLiteral("0")
            || normalized == QStringLiteral("false")
            || normalized == QStringLiteral("no")
            || normalized == QStringLiteral("off"))
        {
            return false;
        }
        return fallbackValue;
    }

    bool firstBool(const QJsonObject& object, const QStringList& keys, bool fallbackValue)
    {
        for (const QString& key : keys)
        {
            const QJsonValue value = object.value(key);
            if (value.isBool())
            {
                return value.toBool();
            }
            if (value.isDouble())
            {
                return value.toInt() != 0;
            }
            if (value.isString())
            {
                return parseBooleanText(value.toString(), fallbackValue);
            }
        }
        return fallbackValue;
    }

    QStringList toStringList(const QJsonValue& value)
    {
        QStringList list;
        if (value.isArray())
        {
            const QJsonArray array = value.toArray();
            list.reserve(array.size());
            for (const QJsonValue& item : array)
            {
                if (item.isString())
                {
                    const QString text = item.toString().trimmed();
                    if (!text.isEmpty())
                    {
                        list.push_back(text);
                    }
                }
            }
            return list;
        }

        if (value.isString())
        {
            const QString text = value.toString().trimmed();
            if (!text.isEmpty())
            {
                list.push_back(text);
            }
        }
        return list;
    }

    QStringList firstStringList(const QJsonObject& object, const QStringList& keys)
    {
        for (const QString& key : keys)
        {
            const QStringList values = toStringList(object.value(key));
            if (!values.isEmpty())
            {
                return values;
            }
        }
        return {};
    }

    QString formatTimestamp(QDateTime value)
    {
        if (!value.isValid())
        {
            return {};
        }
        if (value.timeSpec() == Qt::UTC)
        {
            value = value.toLocalTime();
        }
        return value.toString(QStringLiteral("yyyy-MM-dd-hh-mm-ss"));
    }

    bool pathHasAncestorWithSuffix(
        const QString& filePath,
        const QString& suffix,
        const QString& stopDirectory)
    {
        const QString normalizedStopDirectory = normalizePath(stopDirectory);
        QDir currentDirectory(QFileInfo(filePath).absolutePath());

        while (currentDirectory.exists())
        {
            const QString currentPath = normalizePath(currentDirectory.path());
            if (currentDirectory.dirName().endsWith(suffix, Qt::CaseInsensitive))
            {
                return true;
            }
            if (currentPath.isEmpty()
                || (!normalizedStopDirectory.isEmpty() && currentPath == normalizedStopDirectory))
            {
                break;
            }
            if (!currentDirectory.cdUp())
            {
                break;
            }
        }

        return false;
    }

    QString resolveWsnbodyPath(const QString& noteDirectoryPath)
    {
        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemBodyPath = noteDir.filePath(noteStem + QStringLiteral(".wsnbody"));
            if (QFileInfo(stemBodyPath).isFile())
            {
                return QDir::cleanPath(stemBodyPath);
            }
        }

        const QString canonicalBodyPath = noteDir.filePath(QStringLiteral("note.wsnbody"));
        if (QFileInfo(canonicalBodyPath).isFile())
        {
            return QDir::cleanPath(canonicalBodyPath);
        }

        const QFileInfoList bodyCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnbody")},
            QDir::Files,
            QDir::Name);
        QString draftBodyPath;
        for (const QFileInfo& fileInfo : bodyCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftBodyPath.isEmpty())
                {
                    draftBodyPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftBodyPath.isEmpty())
        {
            return QDir::cleanPath(draftBodyPath);
        }
        return {};
    }

    QString decodeXmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString normalizeBodyLine(QString text)
    {
        text.replace(QRegularExpression(QStringLiteral(R"(\s+)")), QStringLiteral(" "));
        return text.trimmed();
    }

    BodyContentExtract extractBodyContentFromWsnbody(const QString& wsnbodyText)
    {
        BodyContentExtract content;
        static const QRegularExpression bodyPattern(
            QStringLiteral(R"(<body\b[^>]*>([\s\S]*?)</body>)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch bodyMatch = bodyPattern.match(wsnbodyText);
        if (!bodyMatch.hasMatch())
        {
            return content;
        }

        QString innerText = bodyMatch.captured(1);
        innerText.remove(QRegularExpression(QStringLiteral(R"(<!--[\s\S]*?-->)")));
        innerText.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        innerText.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        innerText.replace(
            QRegularExpression(QStringLiteral(R"(<br\s*/?>)"), QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        innerText.replace(
            QRegularExpression(
                QStringLiteral(R"(</(?:p|paragraph|div|li|h[1-6]|section|article|blockquote|ul|ol|tr|table|pre)>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        innerText.replace(
            QRegularExpression(
                QStringLiteral(
                    R"(<(?:p|paragraph|div|li|h[1-6]|section|article|blockquote|ul|ol|tr|table|pre|hr)\b[^>]*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        innerText.replace(QRegularExpression(QStringLiteral(R"(<[^>]+>)")), QString());
        innerText = decodeXmlEntities(std::move(innerText));

        const QStringList rawLines = innerText.split(QLatin1Char('\n'));
        QStringList lines;
        lines.reserve(rawLines.size());
        for (QString line : rawLines)
        {
            line = normalizeBodyLine(std::move(line));
            if (!line.isEmpty())
            {
                lines.push_back(std::move(line));
            }
        }

        if (lines.isEmpty())
        {
            return content;
        }

        content.firstLine = lines.constFirst();
        content.plainText = lines.join(QLatin1Char('\n'));
        return content;
    }

    BodyContentExtract readBodyContent(const LibraryNoteRecord& record)
    {
        BodyContentExtract content;
        const QString bodyPath = resolveWsnbodyPath(record.noteDirectoryPath);
        if (bodyPath.isEmpty())
        {
            return content;
        }

        QString readError;
        const QString bodyText = readUtf8File(bodyPath, &readError);
        if (bodyText.isEmpty())
        {
            if (!readError.isEmpty())
            {
                WhatSon::Debug::trace(
                    QStringLiteral("library.all"),
                    QStringLiteral("scan.wsnbody.readFailed"),
                    QStringLiteral("file=%1 reason=%2").arg(bodyPath, readError));
            }
            return content;
        }

        return extractBodyContentFromWsnbody(bodyText);
    }

    LibraryNoteRecord parseIndexRecord(const QJsonObject& object)
    {
        LibraryNoteRecord record;
        record.noteId = firstString(object, {
                                        QStringLiteral("id"), QStringLiteral("noteId"), QStringLiteral("note_id")
                                    });
        record.storageKind = normalizeStorageKind(firstString(
            object,
            {QStringLiteral("storageKind"), QStringLiteral("format"), QStringLiteral("kind")}));
        record.createdAt = firstString(object, {
                                           QStringLiteral("createdAt"), QStringLiteral("created"),
                                           QStringLiteral("created_at")
                                       });
        record.lastModifiedAt = firstString(
            object,
            {
                QStringLiteral("lastModifiedAt"), QStringLiteral("lastModified"), QStringLiteral("modifiedAt"),
                QStringLiteral("modified"), QStringLiteral("updatedAt")
            });
        record.author = firstString(object, {QStringLiteral("author")});
        record.modifiedBy = firstString(object, {QStringLiteral("modifiedBy"), QStringLiteral("updatedBy")});
        record.project = firstString(object, {QStringLiteral("project")});
        record.folders = firstStringList(
            object,
            {QStringLiteral("folders"), QStringLiteral("folder"), QStringLiteral("folderValues")});
        record.bookmarkColors = firstStringList(
            object,
            {QStringLiteral("bookmarkColors"), QStringLiteral("bookmarkColor"), QStringLiteral("bookmarkColour")});
        record.tags = firstStringList(
            object,
            {QStringLiteral("tags"), QStringLiteral("tag"), QStringLiteral("tagValues")});
        record.progress = firstInt(object, {QStringLiteral("progress"), QStringLiteral("progressValue")}, 0);
        record.bookmarked = firstBool(
            object,
            {QStringLiteral("bookmarked"), QStringLiteral("isBookmarked")},
            false);
        record.preset = firstBool(object, {QStringLiteral("preset"), QStringLiteral("isPreset")}, false);
        record.noteDirectoryPath = normalizePath(firstString(
            object,
            {
                QStringLiteral("noteDirPath"), QStringLiteral("noteDirectoryPath"), QStringLiteral("notePath"),
                QStringLiteral("directory")
            }));
        record.noteHeaderPath = normalizePath(firstString(
            object,
            {
                QStringLiteral("wsnheadPath"), QStringLiteral("noteHeaderPath"), QStringLiteral("headPath"),
                QStringLiteral("path"), QStringLiteral("filePath")
            }));
        return record;
    }

    bool parseIndexText(
        const QString& text,
        QVector<LibraryNoteRecord>* outRecords,
        QString* errorMessage)
    {
        if (outRecords == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outRecords must not be null.");
            }
            return false;
        }

        outRecords->clear();
        if (text.trimmed().isEmpty())
        {
            return true;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(text.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && !document.isNull())
        {
            QJsonArray noteArray;

            auto appendObjectEntries = [&noteArray](const QJsonObject& object) -> bool
            {
                const bool looksLikeSingleRecord =
                    object.contains(QStringLiteral("id"))
                    || object.contains(QStringLiteral("noteId"))
                    || object.contains(QStringLiteral("note_id"))
                    || object.contains(QStringLiteral("noteHeaderPath"))
                    || object.contains(QStringLiteral("wsnheadPath"))
                    || object.contains(QStringLiteral("noteDirPath"));
                if (looksLikeSingleRecord)
                {
                    noteArray.append(object);
                    return true;
                }

                bool appended = false;
                for (auto it = object.constBegin(); it != object.constEnd(); ++it)
                {
                    if (it.value().isObject())
                    {
                        QJsonObject noteObject = it.value().toObject();
                        if (!noteObject.contains(QStringLiteral("id"))
                            && !noteObject.contains(QStringLiteral("noteId"))
                            && !noteObject.contains(QStringLiteral("note_id")))
                        {
                            noteObject.insert(QStringLiteral("id"), it.key());
                        }
                        noteArray.append(noteObject);
                        appended = true;
                        continue;
                    }

                    if (it.value().isString())
                    {
                        QJsonObject noteObject;
                        noteObject.insert(QStringLiteral("id"), it.key());
                        noteArray.append(noteObject);
                        appended = true;
                    }
                }
                return appended;
            };

            if (document.isArray())
            {
                noteArray = document.array();
            }
            else if (document.isObject())
            {
                const QJsonObject root = document.object();
                const QJsonValue notesValue = root.value(QStringLiteral("notes"));
                if (notesValue.isArray())
                {
                    noteArray = notesValue.toArray();
                }
                else if (notesValue.isObject())
                {
                    appendObjectEntries(notesValue.toObject());
                }
                else if (notesValue.isString())
                {
                    noteArray.append(notesValue.toString());
                }
                else
                {
                    appendObjectEntries(root);
                }
            }

            outRecords->reserve(noteArray.size());
            for (const QJsonValue& entry : noteArray)
            {
                LibraryNoteRecord record;
                if (entry.isObject())
                {
                    record = parseIndexRecord(entry.toObject());
                }
                else if (entry.isString())
                {
                    record.noteId = entry.toString().trimmed();
                }

                if (record.noteId.trimmed().isEmpty()
                    && record.noteHeaderPath.trimmed().isEmpty()
                    && record.noteDirectoryPath.trimmed().isEmpty())
                {
                    continue;
                }
                outRecords->push_back(std::move(record));
            }
            return true;
        }

        const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
        outRecords->reserve(lines.size());
        for (QString line : lines)
        {
            line = line.trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            {
                continue;
            }
            LibraryNoteRecord record;
            record.noteId = line;
            outRecords->push_back(std::move(record));
        }

        if (outRecords->isEmpty() && errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("failed to parse .wsnindex: %1").arg(parseError.errorString());
        }
        return true;
    }

    QString recordIdKey(const LibraryNoteRecord& record)
    {
        const QString noteId = record.noteId.trimmed();
        if (noteId.isEmpty())
        {
            return {};
        }
        const QString storageKind = normalizeStorageKind(record.storageKind);
        return QStringLiteral("%1:id:%2")
            .arg(storageKind.isEmpty() ? QStringLiteral("default") : storageKind, noteId.toCaseFolded());
    }

    QString recordHeaderPathKey(const LibraryNoteRecord& record)
    {
        const QString path = normalizePath(record.noteHeaderPath);
        if (path.isEmpty())
        {
            return {};
        }
        return QStringLiteral("head:%1").arg(path.toCaseFolded());
    }

    QString recordDirectoryPathKey(const LibraryNoteRecord& record)
    {
        const QString path = normalizePath(record.noteDirectoryPath);
        if (path.isEmpty())
        {
            return {};
        }
        return QStringLiteral("dir:%1").arg(path.toCaseFolded());
    }

    void overwriteIfNonEmpty(QString* target, const QString& value)
    {
        if (target == nullptr)
        {
            return;
        }
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty())
        {
            *target = trimmed;
        }
    }

    void overwriteListIfNonEmpty(QStringList* target, const QStringList& value)
    {
        if (target == nullptr || value.isEmpty())
        {
            return;
        }
        *target = value;
    }

    void mergeRecord(LibraryNoteRecord* base, const LibraryNoteRecord& overlay)
    {
        if (base == nullptr)
        {
            return;
        }
        overwriteIfNonEmpty(&base->noteId, overlay.noteId);
        overwriteIfNonEmpty(&base->storageKind, overlay.storageKind);
        overwriteIfNonEmpty(&base->createdAt, overlay.createdAt);
        overwriteIfNonEmpty(&base->lastModifiedAt, overlay.lastModifiedAt);
        overwriteIfNonEmpty(&base->author, overlay.author);
        overwriteIfNonEmpty(&base->modifiedBy, overlay.modifiedBy);
        overwriteIfNonEmpty(&base->project, overlay.project);
        overwriteIfNonEmpty(&base->bodyPlainText, overlay.bodyPlainText);
        overwriteIfNonEmpty(&base->bodyFirstLine, overlay.bodyFirstLine);
        overwriteListIfNonEmpty(&base->folders, overlay.folders);
        overwriteListIfNonEmpty(&base->bookmarkColors, overlay.bookmarkColors);
        overwriteListIfNonEmpty(&base->tags, overlay.tags);
        base->progress = overlay.progress;
        base->bookmarked = overlay.bookmarked;
        base->preset = overlay.preset;
        overwriteIfNonEmpty(&base->noteDirectoryPath, overlay.noteDirectoryPath);
        overwriteIfNonEmpty(&base->noteHeaderPath, overlay.noteHeaderPath);
    }

    void normalizeRecordFallbacks(LibraryNoteRecord* record)
    {
        if (record == nullptr)
        {
            return;
        }

        record->storageKind = normalizeStorageKind(record->storageKind);
        record->noteHeaderPath = normalizePath(record->noteHeaderPath);
        record->noteDirectoryPath = normalizePath(record->noteDirectoryPath);

        if (record->noteDirectoryPath.isEmpty() && !record->noteHeaderPath.isEmpty())
        {
            record->noteDirectoryPath = QFileInfo(record->noteHeaderPath).absolutePath();
        }

        if (record->storageKind.isEmpty())
        {
            record->storageKind = QStringLiteral("wsnote");
        }

        if (record->noteId.trimmed().isEmpty() && !record->noteDirectoryPath.isEmpty())
        {
            record->noteId = QFileInfo(record->noteDirectoryPath).completeBaseName().trimmed();
        }
    }

    void finalizeRecordContentFields(LibraryNoteRecord* record)
    {
        if (record == nullptr)
        {
            return;
        }

        record->noteId = record->noteId.trimmed();
        record->bodyPlainText = record->bodyPlainText.trimmed();
        record->bodyFirstLine = record->bodyFirstLine.trimmed();

        if (record->bodyFirstLine.isEmpty() && !record->bodyPlainText.isEmpty())
        {
            record->bodyFirstLine = record->bodyPlainText.section(QLatin1Char('\n'), 0, 0).trimmed();
        }
    }

    LibraryNoteRecord parseRecordFromWsnhead(const QString& wsnHeadPath)
    {
        LibraryNoteRecord record;
        record.noteHeaderPath = normalizePath(wsnHeadPath);
        record.noteDirectoryPath = QFileInfo(wsnHeadPath).absolutePath();
        record.storageKind = QStringLiteral("wsnote");

        QString readError;
        const QString text = readUtf8File(wsnHeadPath, &readError);
        if (text.isEmpty() && !readError.isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("library.all"),
                QStringLiteral("scan.wsnhead.readFailed"),
                QStringLiteral("file=%1 reason=%2").arg(wsnHeadPath, readError));
            normalizeRecordFallbacks(&record);
            return record;
        }

        WhatSonNoteHeaderParser parser;
        WhatSonNoteHeaderStore store;
        QString parseError;
        if (!parser.parse(text, &store, &parseError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("library.all"),
                QStringLiteral("scan.wsnhead.parseFailed"),
                QStringLiteral("file=%1 reason=%2").arg(wsnHeadPath, parseError));
            normalizeRecordFallbacks(&record);
            return record;
        }

        record.noteId = store.noteId();
        record.createdAt = store.createdAt();
        record.lastModifiedAt = store.lastModifiedAt();
        record.author = store.author();
        record.modifiedBy = store.modifiedBy();
        record.project = store.project();
        record.folders = store.folders();
        record.bookmarkColors = store.bookmarkColors();
        record.tags = store.tags();
        record.progress = store.progress();
        record.bookmarked = store.isBookmarked();
        record.preset = store.isPreset();
        normalizeRecordFallbacks(&record);
        return record;
    }

    QString boolToText(bool value)
    {
        return value ? QStringLiteral("true") : QStringLiteral("false");
    }
} // namespace

LibraryAll::LibraryAll()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.all"), QStringLiteral("ctor"));
}

LibraryAll::~LibraryAll()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.all"), QStringLiteral("dtor"));
}

bool LibraryAll::indexFromWshub(const QString& wshubPath, QString* errorMessage)
{
    clear();

    const QString normalizedHubPath = normalizePath(wshubPath);
    if (normalizedHubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must not be empty.");
        }
        return false;
    }

    const QFileInfo hubInfo(normalizedHubPath);
    if (!hubInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(normalizedHubPath);
        }
        return false;
    }
    if (!hubInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(
                normalizedHubPath);
        }
        return false;
    }
    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must end with .wshub: %1").arg(normalizedHubPath);
        }
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("index.begin"),
                              QStringLiteral("path=%1").arg(normalizedHubPath));

    const QStringList contentsDirectories = resolveContentsDirectories(normalizedHubPath);
    const QStringList libraryRoots = resolveLibraryRoots(contentsDirectories);
    if (libraryRoots.isEmpty())
    {
        m_sourceWshubPath = normalizedHubPath;
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No Library.wslibrary directory found inside: %1").arg(
                normalizedHubPath);
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.all"),
                                  QStringLiteral("index.noLibraryRoot"),
                                  QStringLiteral("path=%1").arg(normalizedHubPath));
        return false;
    }

    QVector<LibraryNoteRecord> mergedRecords;
    QHash<QString, int> recordIndexByKey;

    auto indexRecord = [&recordIndexByKey](const LibraryNoteRecord& record) -> int
    {
        const QString idKey = recordIdKey(record);
        if (!idKey.isEmpty() && recordIndexByKey.contains(idKey))
        {
            return recordIndexByKey.value(idKey);
        }

        const QString headerKey = recordHeaderPathKey(record);
        if (!headerKey.isEmpty() && recordIndexByKey.contains(headerKey))
        {
            return recordIndexByKey.value(headerKey);
        }

        const QString dirKey = recordDirectoryPathKey(record);
        if (!dirKey.isEmpty() && recordIndexByKey.contains(dirKey))
        {
            return recordIndexByKey.value(dirKey);
        }

        return -1;
    };

    auto registerKeysForRecord = [&recordIndexByKey](const LibraryNoteRecord& record, int index)
    {
        const QString idKey = recordIdKey(record);
        if (!idKey.isEmpty())
        {
            recordIndexByKey.insert(idKey, index);
        }

        const QString headerKey = recordHeaderPathKey(record);
        if (!headerKey.isEmpty())
        {
            recordIndexByKey.insert(headerKey, index);
        }

        const QString dirKey = recordDirectoryPathKey(record);
        if (!dirKey.isEmpty())
        {
            recordIndexByKey.insert(dirKey, index);
        }
    };

    auto upsertRecord = [&](LibraryNoteRecord record)
    {
        normalizeRecordFallbacks(&record);
        const int foundIndex = indexRecord(record);
        if (foundIndex >= 0)
        {
            mergeRecord(&mergedRecords[foundIndex], record);
            registerKeysForRecord(mergedRecords[foundIndex], foundIndex);
            return;
        }

        const int newIndex = mergedRecords.size();
        mergedRecords.push_back(std::move(record));
        registerKeysForRecord(mergedRecords[newIndex], newIndex);
    };

    for (const QString& libraryRoot : libraryRoots)
    {
        const QString indexPath = QDir(libraryRoot).filePath(QStringLiteral("index.wsnindex"));
        if (QFileInfo(indexPath).isFile())
        {
            QString readError;
            const QString indexText = readUtf8File(indexPath, &readError);
            if (readError.isEmpty())
            {
                QVector<LibraryNoteRecord> indexRecords;
                QString parseError;
                if (parseIndexText(indexText, &indexRecords, &parseError))
                {
                    for (LibraryNoteRecord& indexRecord : indexRecords)
                    {
                        if (!indexRecord.noteHeaderPath.isEmpty() && QFileInfo(indexRecord.noteHeaderPath).isRelative())
                        {
                            indexRecord.noteHeaderPath = QDir(libraryRoot).filePath(indexRecord.noteHeaderPath);
                        }
                        if (!indexRecord.noteDirectoryPath.isEmpty() && QFileInfo(indexRecord.noteDirectoryPath).
                            isRelative())
                        {
                            indexRecord.noteDirectoryPath = QDir(libraryRoot).filePath(indexRecord.noteDirectoryPath);
                        }
                        upsertRecord(std::move(indexRecord));
                    }
                }
                else
                {
                    WhatSon::Debug::traceSelf(this,
                                              QStringLiteral("library.all"),
                                              QStringLiteral("index.parseFailed"),
                                              QStringLiteral("path=%1 reason=%2").arg(indexPath, parseError));
                }
            }
            else
            {
                WhatSon::Debug::traceSelf(this,
                                          QStringLiteral("library.all"),
                                          QStringLiteral("index.readFailed"),
                                          QStringLiteral("path=%1 reason=%2").arg(indexPath, readError));
            }
        }
        else
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.all"),
                                      QStringLiteral("index.fileMissing"),
                                      QStringLiteral("path=%1").arg(indexPath));
        }

        QDirIterator iterator(
            libraryRoot,
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            const QString wsnHeadPath = iterator.next();
            upsertRecord(parseRecordFromWsnhead(wsnHeadPath));
        }
    }

    for (LibraryNoteRecord& record : mergedRecords)
    {
        normalizeRecordFallbacks(&record);
        const BodyContentExtract bodyContent = readBodyContent(record);
        overwriteIfNonEmpty(&record.bodyPlainText, bodyContent.plainText);
        overwriteIfNonEmpty(&record.bodyFirstLine, bodyContent.firstLine);
        finalizeRecordContentFields(&record);
    }

    m_sourceWshubPath = normalizedHubPath;
    m_notes = std::move(mergedRecords);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("index.success"),
                              QStringLiteral("path=%1 noteCount=%2").arg(m_sourceWshubPath).arg(m_notes.size()));

    if (WhatSon::Debug::isEnabled())
    {
        for (const LibraryNoteRecord& record : m_notes)
        {
            qWarning().noquote()
                << QStringLiteral(
                    "[wsnindex:all] id=%1 firstLine=%2 created=%3 modified=%4 folders=[%5] progress=%6 bookmarked=%7 preset=%8 head=%9")
                .arg(
                    record.noteId,
                    record.bodyFirstLine,
                    record.createdAt,
                    record.lastModifiedAt,
                    record.folders.join(QStringLiteral(", ")),
                    QString::number(record.progress),
                    boolToText(record.bookmarked),
                    boolToText(record.preset),
                    record.noteHeaderPath);
        }
    }
    return true;
}

void LibraryAll::setIndexedNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes)
{
    m_sourceWshubPath = normalizePath(sourceWshubPath);
    m_notes = std::move(notes);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("setIndexedNotes"),
                              QStringLiteral("path=%1 noteCount=%2").arg(m_sourceWshubPath).arg(m_notes.size()));
}

void LibraryAll::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_notes.size()));
    m_sourceWshubPath.clear();
    m_notes.clear();
}

QString LibraryAll::sourceWshubPath() const
{
    return m_sourceWshubPath;
}

const QVector<LibraryNoteRecord>& LibraryAll::notes() const noexcept
{
    return m_notes;
}
