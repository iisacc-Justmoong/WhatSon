#include "WhatSonNoteHeaderStore.hpp"

#include "WhatSonBookmarkColorPalette.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QRegularExpression>
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
    m_modifiedBy.clear();
    m_folders.clear();
    m_project.clear();
    m_bookmarked = false;
    m_bookmarkColors.clear();
    m_tags.clear();
    m_progress = 0;
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
    const int rawCount = folders.size();
    m_folders = sanitizeStringList(std::move(folders), QStringLiteral("folder"));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.store"),
                              QStringLiteral("setFolders"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_folders.size())
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

int WhatSonNoteHeaderStore::progress() const noexcept
{
    return m_progress;
}

void WhatSonNoteHeaderStore::setProgress(int progress) noexcept
{
    m_progress = std::max(progress, 0);
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
