#include "app/models/file/note/WhatSonNoteHeaderStore.hpp"

#include "app/models/file/hierarchy/WhatSonFolderIdentity.hpp"
#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"

#include <QRegularExpression>
#include <QSet>
#include <QUuid>

#include <algorithm>
#include <utility>

namespace
{
    bool isTemplateToken(const QString& token)
    {
        const QString trimmed = token.trimmed();
        return (trimmed.startsWith(QStringLiteral("${")) && trimmed.endsWith(QLatin1Char('}')))
            || (trimmed.startsWith(QStringLiteral("%{")) && trimmed.endsWith(QLatin1Char('}')));
    }

    QString generatedNoteId()
    {
        return QStringLiteral("note-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8));
    }

    bool isDatePlaceholderToken(const QString& value)
    {
        const QString normalized = value.trimmed().toCaseFolded();
        return normalized == QStringLiteral("yyyy-mm-dd-hh-mm-ss");
    }

    QString templatePayload(QString token)
    {
        token = token.trimmed();
        if (isTemplateToken(token) && token.size() >= 3)
        {
            token = token.mid(2, token.size() - 3).trimmed();
        }

        const int separatorIndex = token.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0)
        {
            token = token.left(separatorIndex).trimmed();
        }

        if ((token.startsWith(QLatin1Char('"')) && token.endsWith(QLatin1Char('"')) && token.size() >= 2)
            || (token.startsWith(QLatin1Char('\'')) && token.endsWith(QLatin1Char('\'')) && token.size() >= 2))
        {
            token = token.mid(1, token.size() - 2).trimmed();
        }

        token.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
        token.remove(QRegularExpression(QStringLiteral("[^A-Za-z0-9._/-]")));
        return token;
    }

    bool isGenericIdToken(const QString& token)
    {
        const QString normalized = token.trimmed().toCaseFolded();
        return normalized.isEmpty()
            || normalized == QStringLiteral("id")
            || normalized == QStringLiteral("note-id")
            || normalized == QStringLiteral("noteid");
    }

    QString sanitizeText(QString value)
    {
        return value.trimmed();
    }

    QStringList sanitizeStringList(QStringList values, const QString& fallbackPrefix)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());
        Q_UNUSED(fallbackPrefix);

        for (QString& value : values)
        {
            value = value.trimmed();
            if (value.isEmpty())
            {
                continue;
            }

            if (isTemplateToken(value))
            {
                QString resolved = templatePayload(value);
                if (resolved.isEmpty())
                {
                    WhatSon::Debug::trace(
                        QStringLiteral("note.header.store"),
                        QStringLiteral("sanitizeStringList.dropEmptyTemplateToken"));
                    continue;
                }
                sanitized.push_back(resolved);
                continue;
            }
            sanitized.push_back(value);
        }

        return sanitized;
    }

    struct SanitizedFolderBindings final
    {
        QStringList folders;
        QStringList folderUuids;
    };

    SanitizedFolderBindings sanitizeFolderBindings(QStringList values, QStringList folderUuids)
    {
        SanitizedFolderBindings sanitized;
        sanitized.folders.reserve(values.size());
        sanitized.folderUuids.reserve(values.size());
        QSet<QString> seenBindings;

        for (int index = 0; index < values.size(); ++index)
        {
            QString& value = values[index];
            value = value.trimmed();
            if (value.isEmpty())
            {
                continue;
            }

            if (isTemplateToken(value))
            {
                value = templatePayload(value);
                if (value.isEmpty())
                {
                    WhatSon::Debug::trace(
                        QStringLiteral("note.header.store"),
                        QStringLiteral("sanitizeFolderList.dropEmptyTemplateToken"));
                    continue;
                }
            }

            if (WhatSon::NoteFolders::usesReservedTodayFolderSegment(value))
            {
                WhatSon::Debug::trace(
                    QStringLiteral("note.header.store"),
                    QStringLiteral("sanitizeFolderList.dropReservedTodayToken"),
                    QStringLiteral("value=%1").arg(value));
                continue;
            }

            const QString folderUuid = index < folderUuids.size()
                                           ? WhatSon::FolderIdentity::normalizeFolderUuid(folderUuids.at(index))
                                           : QString();
            const QString bindingKey = !folderUuid.isEmpty()
                                           ? QStringLiteral("uuid:%1").arg(folderUuid)
                                           : QStringLiteral(
                                               "path:%1").arg(WhatSon::NoteFolders::normalizeFolderPath(value)
                                                                  .toCaseFolded());
            if (bindingKey.endsWith(QLatin1Char(':')) || seenBindings.contains(bindingKey))
            {
                continue;
            }

            seenBindings.insert(bindingKey);
            sanitized.folders.push_back(value);
            sanitized.folderUuids.push_back(folderUuid);
        }

        return sanitized;
    }

    int sanitizeCountValue(const int value) noexcept
    {
        return std::max(value, 0);
    }
} // namespace

WhatSonNoteHeaderStore::WhatSonNoteHeaderStore() = default;

WhatSonNoteHeaderStore::~WhatSonNoteHeaderStore() = default;

void WhatSonNoteHeaderStore::clear()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("note.header.store"), QStringLiteral("clear"));
    m_noteId.clear();
    m_createdAt.clear();
    m_author.clear();
    m_lastModifiedAt.clear();
    m_lastOpenedAt.clear();
    m_modifiedBy.clear();
    m_folders.clear();
    m_folderUuids.clear();
    m_project.clear();
    m_bookmarked = false;
    m_bookmarkColors.clear();
    m_tags.clear();
    m_totalFolders = 0;
    m_totalTags = 0;
    m_letterCount = 0;
    m_wordCount = 0;
    m_sentenceCount = 0;
    m_paragraphCount = 0;
    m_spaceCount = 0;
    m_indentCount = 0;
    m_lineCount = 0;
    m_openCount = 0;
    m_modifiedCount = 0;
    m_backlinkToCount = 0;
    m_backlinkByCount = 0;
    m_includedResourceCount = 0;
    m_progressEnums.clear();
    m_progress = -1;
    m_preset = false;
}

QString WhatSonNoteHeaderStore::noteId() const
{
    return m_noteId;
}

void WhatSonNoteHeaderStore::setNoteId(QString noteId)
{
    QString value = sanitizeText(std::move(noteId));
    if (value.isEmpty())
    {
        value = generatedNoteId();
    }
    else if (isTemplateToken(value))
    {
        const QString resolved = templatePayload(value);
        value = isGenericIdToken(resolved) ? generatedNoteId() : resolved;
    }

    m_noteId = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setNoteId"),
                              QStringLiteral("value=%1").arg(m_noteId));
}

QString WhatSonNoteHeaderStore::createdAt() const
{
    return m_createdAt;
}

void WhatSonNoteHeaderStore::setCreatedAt(QString createdAt)
{
    QString value = sanitizeText(std::move(createdAt));
    if (value.isEmpty() || isTemplateToken(value) || isDatePlaceholderToken(value))
    {
        value.clear();
    }

    m_createdAt = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setCreatedAt"),
                              QStringLiteral("value=%1").arg(m_createdAt));
}

QString WhatSonNoteHeaderStore::author() const
{
    return m_author;
}

void WhatSonNoteHeaderStore::setAuthor(QString author)
{
    QString value = sanitizeText(std::move(author));
    if (value.isEmpty())
    {
        value.clear();
    }
    else if (isTemplateToken(value))
    {
        const QString resolved = templatePayload(value);
        value = resolved;
    }

    m_author = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setAuthor"),
                              QStringLiteral("value=%1").arg(m_author));
}

QString WhatSonNoteHeaderStore::lastModifiedAt() const
{
    return m_lastModifiedAt;
}

void WhatSonNoteHeaderStore::setLastModifiedAt(QString lastModifiedAt)
{
    QString value = sanitizeText(std::move(lastModifiedAt));
    if (value.isEmpty() || isTemplateToken(value) || isDatePlaceholderToken(value))
    {
        value.clear();
    }

    m_lastModifiedAt = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setLastModifiedAt"),
                              QStringLiteral("value=%1").arg(m_lastModifiedAt));
}

QString WhatSonNoteHeaderStore::lastOpenedAt() const
{
    return m_lastOpenedAt;
}

void WhatSonNoteHeaderStore::setLastOpenedAt(QString lastOpenedAt)
{
    QString value = sanitizeText(std::move(lastOpenedAt));
    if (value.isEmpty() || isTemplateToken(value) || isDatePlaceholderToken(value))
    {
        value.clear();
    }

    m_lastOpenedAt = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setLastOpenedAt"),
                              QStringLiteral("value=%1").arg(m_lastOpenedAt));
}

QString WhatSonNoteHeaderStore::modifiedBy() const
{
    return m_modifiedBy;
}

void WhatSonNoteHeaderStore::setModifiedBy(QString modifiedBy)
{
    QString value = sanitizeText(std::move(modifiedBy));
    if (value.isEmpty())
    {
        value.clear();
    }
    else if (isTemplateToken(value))
    {
        const QString resolved = templatePayload(value);
        value = resolved;
    }

    m_modifiedBy = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setModifiedBy"),
                              QStringLiteral("value=%1").arg(m_modifiedBy));
}

QStringList WhatSonNoteHeaderStore::folders() const
{
    return m_folders;
}

void WhatSonNoteHeaderStore::setFolders(QStringList folders)
{
    setFolderBindings(std::move(folders), m_folderUuids);
}

QStringList WhatSonNoteHeaderStore::folderUuids() const
{
    return m_folderUuids;
}

void WhatSonNoteHeaderStore::setFolderUuids(QStringList folderUuids)
{
    setFolderBindings(m_folders, std::move(folderUuids));
}

void WhatSonNoteHeaderStore::setFolderBindings(QStringList folders, QStringList folderUuids)
{
    const int rawCount = folders.size();
    const SanitizedFolderBindings sanitized = sanitizeFolderBindings(
        std::move(folders),
        std::move(folderUuids));
    m_folders = sanitized.folders;
    m_folderUuids = sanitized.folderUuids;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setFolderBindings"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 uuidCount=%3 values=[%4]")
                              .arg(rawCount)
                              .arg(m_folders.size())
                              .arg(m_folderUuids.size())
                              .arg(m_folders.join(QStringLiteral(", "))));
}

QString WhatSonNoteHeaderStore::project() const
{
    return m_project;
}

void WhatSonNoteHeaderStore::setProject(QString project)
{
    QString value = sanitizeText(std::move(project));
    if (value.isEmpty())
    {
        value.clear();
    }
    else if (isTemplateToken(value))
    {
        const QString resolved = templatePayload(value);
        value = resolved;
    }

    m_project = value;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setProject"),
                              QStringLiteral("value=%1").arg(m_project));
}

bool WhatSonNoteHeaderStore::isBookmarked() const noexcept
{
    return m_bookmarked;
}

void WhatSonNoteHeaderStore::setBookmarked(bool bookmarked) noexcept
{
    m_bookmarked = bookmarked;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setBookmarked"),
                              QStringLiteral("value=%1").arg(
                                  m_bookmarked ? QStringLiteral("true") : QStringLiteral("false")));
}

QStringList WhatSonNoteHeaderStore::bookmarkColors() const
{
    return m_bookmarkColors;
}

void WhatSonNoteHeaderStore::setBookmarkColors(QStringList colors)
{
    const int rawCount = colors.size();
    QStringList sanitized;
    sanitized.reserve(rawCount);

    for (const QString& color : colors)
    {
        const QString canonical = WhatSon::Bookmarks::canonicalBookmarkColorToken(color);
        if (canonical.isEmpty() || sanitized.contains(canonical))
        {
            continue;
        }
        sanitized.push_back(canonical);
    }

    m_bookmarkColors = std::move(sanitized);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setBookmarkColors"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_bookmarkColors.size())
                              .arg(m_bookmarkColors.join(QStringLiteral(", "))));
}

QStringList WhatSonNoteHeaderStore::tags() const
{
    return m_tags;
}

void WhatSonNoteHeaderStore::setTags(QStringList tags)
{
    const int rawCount = tags.size();
    m_tags = sanitizeStringList(std::move(tags), QStringLiteral("tag"));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setTags"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_tags.size())
                              .arg(m_tags.join(QStringLiteral(", "))));
}

int WhatSonNoteHeaderStore::totalFolders() const noexcept
{
    return m_totalFolders;
}

void WhatSonNoteHeaderStore::setTotalFolders(int totalFolders) noexcept
{
    m_totalFolders = sanitizeCountValue(totalFolders);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setTotalFolders"),
                              QStringLiteral("value=%1").arg(m_totalFolders));
}

int WhatSonNoteHeaderStore::totalTags() const noexcept
{
    return m_totalTags;
}

void WhatSonNoteHeaderStore::setTotalTags(int totalTags) noexcept
{
    m_totalTags = sanitizeCountValue(totalTags);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setTotalTags"),
                              QStringLiteral("value=%1").arg(m_totalTags));
}

int WhatSonNoteHeaderStore::letterCount() const noexcept
{
    return m_letterCount;
}

void WhatSonNoteHeaderStore::setLetterCount(int letterCount) noexcept
{
    m_letterCount = sanitizeCountValue(letterCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setLetterCount"),
                              QStringLiteral("value=%1").arg(m_letterCount));
}

int WhatSonNoteHeaderStore::wordCount() const noexcept
{
    return m_wordCount;
}

void WhatSonNoteHeaderStore::setWordCount(int wordCount) noexcept
{
    m_wordCount = sanitizeCountValue(wordCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setWordCount"),
                              QStringLiteral("value=%1").arg(m_wordCount));
}

int WhatSonNoteHeaderStore::sentenceCount() const noexcept
{
    return m_sentenceCount;
}

void WhatSonNoteHeaderStore::setSentenceCount(int sentenceCount) noexcept
{
    m_sentenceCount = sanitizeCountValue(sentenceCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setSentenceCount"),
                              QStringLiteral("value=%1").arg(m_sentenceCount));
}

int WhatSonNoteHeaderStore::paragraphCount() const noexcept
{
    return m_paragraphCount;
}

void WhatSonNoteHeaderStore::setParagraphCount(int paragraphCount) noexcept
{
    m_paragraphCount = sanitizeCountValue(paragraphCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setParagraphCount"),
                              QStringLiteral("value=%1").arg(m_paragraphCount));
}

int WhatSonNoteHeaderStore::spaceCount() const noexcept
{
    return m_spaceCount;
}

void WhatSonNoteHeaderStore::setSpaceCount(int spaceCount) noexcept
{
    m_spaceCount = sanitizeCountValue(spaceCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setSpaceCount"),
                              QStringLiteral("value=%1").arg(m_spaceCount));
}

int WhatSonNoteHeaderStore::indentCount() const noexcept
{
    return m_indentCount;
}

void WhatSonNoteHeaderStore::setIndentCount(int indentCount) noexcept
{
    m_indentCount = sanitizeCountValue(indentCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setIndentCount"),
                              QStringLiteral("value=%1").arg(m_indentCount));
}

int WhatSonNoteHeaderStore::lineCount() const noexcept
{
    return m_lineCount;
}

void WhatSonNoteHeaderStore::setLineCount(int lineCount) noexcept
{
    m_lineCount = sanitizeCountValue(lineCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setLineCount"),
                              QStringLiteral("value=%1").arg(m_lineCount));
}

int WhatSonNoteHeaderStore::openCount() const noexcept
{
    return m_openCount;
}

void WhatSonNoteHeaderStore::setOpenCount(int openCount) noexcept
{
    m_openCount = sanitizeCountValue(openCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setOpenCount"),
                              QStringLiteral("value=%1").arg(m_openCount));
}

void WhatSonNoteHeaderStore::incrementOpenCount() noexcept
{
    setOpenCount(m_openCount + 1);
}

int WhatSonNoteHeaderStore::modifiedCount() const noexcept
{
    return m_modifiedCount;
}

void WhatSonNoteHeaderStore::setModifiedCount(int modifiedCount) noexcept
{
    m_modifiedCount = sanitizeCountValue(modifiedCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setModifiedCount"),
                              QStringLiteral("value=%1").arg(m_modifiedCount));
}

void WhatSonNoteHeaderStore::incrementModifiedCount() noexcept
{
    setModifiedCount(m_modifiedCount + 1);
}

int WhatSonNoteHeaderStore::backlinkToCount() const noexcept
{
    return m_backlinkToCount;
}

void WhatSonNoteHeaderStore::setBacklinkToCount(int backlinkToCount) noexcept
{
    m_backlinkToCount = sanitizeCountValue(backlinkToCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setBacklinkToCount"),
                              QStringLiteral("value=%1").arg(m_backlinkToCount));
}

int WhatSonNoteHeaderStore::backlinkByCount() const noexcept
{
    return m_backlinkByCount;
}

void WhatSonNoteHeaderStore::setBacklinkByCount(int backlinkByCount) noexcept
{
    m_backlinkByCount = sanitizeCountValue(backlinkByCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setBacklinkByCount"),
                              QStringLiteral("value=%1").arg(m_backlinkByCount));
}

int WhatSonNoteHeaderStore::includedResourceCount() const noexcept
{
    return m_includedResourceCount;
}

void WhatSonNoteHeaderStore::setIncludedResourceCount(int includedResourceCount) noexcept
{
    m_includedResourceCount = sanitizeCountValue(includedResourceCount);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setIncludedResourceCount"),
                              QStringLiteral("value=%1").arg(m_includedResourceCount));
}

QStringList WhatSonNoteHeaderStore::progressEnums() const
{
    return m_progressEnums;
}

void WhatSonNoteHeaderStore::setProgressEnums(QStringList progressEnums)
{
    const int rawCount = progressEnums.size();
    m_progressEnums = sanitizeStringList(std::move(progressEnums), QStringLiteral("progress"));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setProgressEnums"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_progressEnums.size())
                              .arg(m_progressEnums.join(QStringLiteral(", "))));
}

int WhatSonNoteHeaderStore::progress() const noexcept
{
    return m_progress;
}

void WhatSonNoteHeaderStore::setProgress(int progress) noexcept
{
    m_progress = std::max(progress, -1);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setProgress"),
                              QStringLiteral("value=%1").arg(m_progress));
}

bool WhatSonNoteHeaderStore::isPreset() const noexcept
{
    return m_preset;
}

void WhatSonNoteHeaderStore::setPreset(bool preset) noexcept
{
    m_preset = preset;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setPreset"),
                              QStringLiteral("value=%1").arg(
                                  m_preset ? QStringLiteral("true") : QStringLiteral("false")));
}
