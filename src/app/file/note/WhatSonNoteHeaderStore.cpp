#include "WhatSonNoteHeaderStore.hpp"

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

    QString sanitizeText(QString value)
    {
        return value.trimmed();
    }

    QStringList sanitizeStringList(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());

        for (QString& value : values)
        {
            value = value.trimmed();
            if (value.isEmpty() || isTemplateToken(value))
            {
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
    m_noteId.clear();
    m_title.clear();
    m_createdAt.clear();
    m_author.clear();
    m_lastModifiedAt.clear();
    m_modifiedBy.clear();
    m_folders.clear();
    m_project.clear();
    m_bookmarked = false;
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
    m_noteId = sanitizeText(std::move(noteId));
}

QString WhatSonNoteHeaderStore::title() const
{
    return m_title;
}

void WhatSonNoteHeaderStore::setTitle(QString title)
{
    m_title = sanitizeText(std::move(title));
}

QString WhatSonNoteHeaderStore::createdAt() const
{
    return m_createdAt;
}

void WhatSonNoteHeaderStore::setCreatedAt(QString createdAt)
{
    m_createdAt = sanitizeText(std::move(createdAt));
}

QString WhatSonNoteHeaderStore::author() const
{
    return m_author;
}

void WhatSonNoteHeaderStore::setAuthor(QString author)
{
    m_author = sanitizeText(std::move(author));
}

QString WhatSonNoteHeaderStore::lastModifiedAt() const
{
    return m_lastModifiedAt;
}

void WhatSonNoteHeaderStore::setLastModifiedAt(QString lastModifiedAt)
{
    m_lastModifiedAt = sanitizeText(std::move(lastModifiedAt));
}

QString WhatSonNoteHeaderStore::modifiedBy() const
{
    return m_modifiedBy;
}

void WhatSonNoteHeaderStore::setModifiedBy(QString modifiedBy)
{
    m_modifiedBy = sanitizeText(std::move(modifiedBy));
}

QStringList WhatSonNoteHeaderStore::folders() const
{
    return m_folders;
}

void WhatSonNoteHeaderStore::setFolders(QStringList folders)
{
    m_folders = sanitizeStringList(std::move(folders));
}

QString WhatSonNoteHeaderStore::project() const
{
    return m_project;
}

void WhatSonNoteHeaderStore::setProject(QString project)
{
    m_project = sanitizeText(std::move(project));
}

bool WhatSonNoteHeaderStore::isBookmarked() const noexcept
{
    return m_bookmarked;
}

void WhatSonNoteHeaderStore::setBookmarked(bool bookmarked) noexcept
{
    m_bookmarked = bookmarked;
}

QStringList WhatSonNoteHeaderStore::tags() const
{
    return m_tags;
}

void WhatSonNoteHeaderStore::setTags(QStringList tags)
{
    m_tags = sanitizeStringList(std::move(tags));
}

int WhatSonNoteHeaderStore::progress() const noexcept
{
    return m_progress;
}

void WhatSonNoteHeaderStore::setProgress(int progress) noexcept
{
    m_progress = std::max(progress, 0);
}

bool WhatSonNoteHeaderStore::isPreset() const noexcept
{
    return m_preset;
}

void WhatSonNoteHeaderStore::setPreset(bool preset) noexcept
{
    m_preset = preset;
}
