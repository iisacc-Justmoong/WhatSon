#include "ResourcesListModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>
#include <algorithm>

namespace
{
    const QRegularExpression kSearchWhitespacePattern(QStringLiteral("\\s+"));

    QString normalizeSearchableText(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        value = value.trimmed().toCaseFolded();
        value.replace(kSearchWhitespacePattern, QStringLiteral(" "));
        return value.trimmed();
    }

    QString normalizeImageSource(QString value)
    {
        value = value.trimmed();
        if (value.isEmpty())
        {
            return {};
        }

        if (QFileInfo(value).isAbsolute())
        {
            return QUrl::fromLocalFile(QDir::cleanPath(value)).toString();
        }

        const QUrl url(value);
        if (url.isValid() && !url.scheme().isEmpty())
        {
            if (url.isLocalFile())
            {
                return QUrl::fromLocalFile(QDir::cleanPath(url.toLocalFile())).toString();
            }
            return url.toString();
        }

        return value;
    }

    QStringList sanitizeMetadataList(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());
        for (QString value : std::as_const(values))
        {
            value = value.trimmed();
            if (value.isEmpty() || sanitized.contains(value))
            {
                continue;
            }
            sanitized.push_back(std::move(value));
        }
        return sanitized;
    }

    QString itemIdAt(const QVector<ResourcesListItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }
        return items.at(index).id;
    }

    QString itemBodyTextAt(const QVector<ResourcesListItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }
        return items.at(index).bodyText;
    }

    QVariantMap itemResourceEntryAt(const QVector<ResourcesListItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        const ResourcesListItem& item = items.at(index);
        QVariantMap entry;
        entry.insert(QStringLiteral("noteId"), item.id);
        entry.insert(QStringLiteral("type"), item.type);
        entry.insert(QStringLiteral("format"), item.format);
        entry.insert(QStringLiteral("resourcePath"), item.resourcePath);
        entry.insert(QStringLiteral("resolvedPath"), item.resolvedPath);
        entry.insert(QStringLiteral("source"), item.source);
        entry.insert(QStringLiteral("renderMode"), item.renderMode);
        entry.insert(QStringLiteral("displayName"), item.displayName);
        entry.insert(QStringLiteral("previewText"), item.previewText);
        return entry;
    }

    int indexOfItemById(const QVector<ResourcesListItem>& items, const QString& id)
    {
        const QString normalizedId = id.trimmed();
        if (normalizedId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (items.at(index).id.trimmed() == normalizedId)
            {
                return index;
            }
        }
        return -1;
    }

    bool sameResourceListItem(const ResourcesListItem& lhs, const ResourcesListItem& rhs)
    {
        return lhs.id == rhs.id
            && lhs.primaryText == rhs.primaryText
            && lhs.searchableText == rhs.searchableText
            && lhs.bodyText == rhs.bodyText
            && lhs.displayDate == rhs.displayDate
            && lhs.folders == rhs.folders
            && lhs.tags == rhs.tags
            && lhs.image == rhs.image
            && lhs.imageSource == rhs.imageSource
            && lhs.bookmarked == rhs.bookmarked
            && lhs.bookmarkColor == rhs.bookmarkColor
            && lhs.type == rhs.type
            && lhs.format == rhs.format
            && lhs.resourcePath == rhs.resourcePath
            && lhs.resolvedPath == rhs.resolvedPath
            && lhs.source == rhs.source
            && lhs.renderMode == rhs.renderMode
            && lhs.displayName == rhs.displayName
            && lhs.previewText == rhs.previewText;
    }

    bool sameResourceListItems(
        const QVector<ResourcesListItem>& lhs,
        const QVector<ResourcesListItem>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            if (!sameResourceListItem(lhs.at(index), rhs.at(index)))
            {
                return false;
            }
        }

        return true;
    }

    QString fallbackSearchableText(const ResourcesListItem& item)
    {
        QStringList parts;
        if (!item.primaryText.trimmed().isEmpty())
        {
            parts.push_back(item.primaryText.trimmed());
        }
        if (!item.bodyText.trimmed().isEmpty())
        {
            parts.push_back(item.bodyText.trimmed());
        }
        if (!item.type.trimmed().isEmpty())
        {
            parts.push_back(item.type.trimmed());
        }
        if (!item.format.trimmed().isEmpty())
        {
            parts.push_back(item.format.trimmed());
        }
        if (!item.resourcePath.trimmed().isEmpty())
        {
            parts.push_back(item.resourcePath.trimmed());
        }
        for (const QString& folder : item.folders)
        {
            if (!folder.trimmed().isEmpty())
            {
                parts.push_back(folder.trimmed());
            }
        }
        for (const QString& tag : item.tags)
        {
            if (!tag.trimmed().isEmpty())
            {
                parts.push_back(tag.trimmed());
            }
        }
        return parts.join(QLatin1Char('\n'));
    }

    QStringList searchTerms(const QString& searchText)
    {
        const QString normalized = normalizeSearchableText(searchText);
        if (normalized.isEmpty())
        {
            return {};
        }
        return normalized.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }

    bool itemMatchesSearch(const ResourcesListItem& item, const QStringList& terms)
    {
        if (terms.isEmpty())
        {
            return true;
        }

        const QString searchableText = item.searchableText.isEmpty()
                                           ? normalizeSearchableText(fallbackSearchableText(item))
                                           : item.searchableText;
        for (const QString& term : terms)
        {
            if (!searchableText.contains(term))
            {
                return false;
            }
        }
        return true;
    }
} // namespace

ResourcesListModel::ResourcesListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("resources.notelist.model"), QStringLiteral("ctor"));
}

int ResourcesListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_items.size();
}

bool ResourcesListModel::noteBacked() const noexcept
{
    return false;
}

QVariant ResourcesListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const ResourcesListItem& item = m_items.at(index.row());
    switch (role)
    {
    case IdRole:
    case NoteIdRole:
        return item.id;
    case PrimaryTextRole:
        return item.primaryText;
    case BodyTextRole:
        return item.bodyText;
    case ImageRole:
        return item.image;
    case ImageSourceRole:
        return item.imageSource;
    case DisplayDateRole:
        return item.displayDate;
    case FoldersRole:
        return item.folders;
    case TagsRole:
        return item.tags;
    case BookmarkedRole:
        return item.bookmarked;
    case BookmarkColorRole:
        return item.bookmarkColor;
    case TypeRole:
        return item.type;
    case FormatRole:
        return item.format;
    case ResourcePathRole:
        return item.resourcePath;
    case ResolvedPathRole:
        return item.resolvedPath;
    case SourceRole:
        return item.source;
    case RenderModeRole:
        return item.renderMode;
    case DisplayNameRole:
        return item.displayName;
    case PreviewTextRole:
        return item.previewText;
    default:
        return {};
    }
}

QHash<int, QByteArray> ResourcesListModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {NoteIdRole, "noteId"},
        {PrimaryTextRole, "primaryText"},
        {BodyTextRole, "bodyText"},
        {ImageRole, "image"},
        {ImageSourceRole, "imageSource"},
        {DisplayDateRole, "displayDate"},
        {FoldersRole, "folders"},
        {TagsRole, "tags"},
        {BookmarkedRole, "bookmarked"},
        {BookmarkColorRole, "bookmarkColor"},
        {TypeRole, "type"},
        {FormatRole, "format"},
        {ResourcePathRole, "resourcePath"},
        {ResolvedPathRole, "resolvedPath"},
        {SourceRole, "source"},
        {RenderModeRole, "renderMode"},
        {DisplayNameRole, "displayName"},
        {PreviewTextRole, "previewText"}
    };
}

int ResourcesListModel::itemCount() const noexcept
{
    return m_items.size();
}

int ResourcesListModel::currentIndex() const noexcept
{
    return m_currentIndex;
}

QString ResourcesListModel::currentNoteId() const
{
    return itemIdAt(m_items, m_currentIndex);
}

QString ResourcesListModel::currentBodyText() const
{
    return itemBodyTextAt(m_items, m_currentIndex);
}

QVariantMap ResourcesListModel::currentResourceEntry() const
{
    return itemResourceEntryAt(m_items, m_currentIndex);
}

void ResourcesListModel::setCurrentIndex(int index)
{
    int nextIndex = index;
    if (m_items.isEmpty())
    {
        nextIndex = -1;
    }
    else
    {
        nextIndex = std::clamp(index, -1, static_cast<int>(m_items.size()) - 1);
    }

    if (m_currentIndex == nextIndex)
    {
        return;
    }

    const QString previousNoteId = currentNoteId();
    const QString previousBodyText = currentBodyText();
    const QVariantMap previousResourceEntry = currentResourceEntry();

    m_currentIndex = nextIndex;
    emit currentIndexChanged();
    if (currentNoteId() != previousNoteId)
    {
        emit currentNoteIdChanged();
    }
    if (currentBodyText() != previousBodyText)
    {
        emit currentBodyTextChanged();
    }
    if (currentResourceEntry() != previousResourceEntry)
    {
        emit currentResourceEntryChanged();
    }
}

QString ResourcesListModel::searchText() const
{
    return m_searchText;
}

void ResourcesListModel::setSearchText(const QString& text)
{
    if (m_searchText == text)
    {
        return;
    }

    m_searchText = text;
    applySearchFilter();
    emit searchTextChanged();
}

void ResourcesListModel::setItems(QVector<ResourcesListItem> items)
{
    QVector<ResourcesListItem> sanitized;
    sanitized.reserve(items.size());

    for (ResourcesListItem& sourceItem : items)
    {
        ResourcesListItem item = std::move(sourceItem);
        item.id = item.id.trimmed();
        item.primaryText = item.primaryText.trimmed();
        item.bodyText = item.bodyText;
        item.displayDate = item.displayDate.trimmed();
        item.folders = sanitizeMetadataList(std::move(item.folders));
        item.tags = sanitizeMetadataList(std::move(item.tags));
        item.imageSource = item.image ? normalizeImageSource(item.imageSource) : QString();
        item.bookmarkColor = item.bookmarkColor.trimmed();
        item.type = item.type.trimmed();
        item.format = item.format.trimmed();
        item.resourcePath = item.resourcePath.trimmed();
        item.resolvedPath = item.resolvedPath.trimmed();
        item.source = item.source.trimmed();
        item.renderMode = item.renderMode.trimmed();
        item.displayName = item.displayName.trimmed();
        item.previewText = item.previewText.trimmed();
        item.searchableText = normalizeSearchableText(item.searchableText);
        if (item.searchableText.isEmpty())
        {
            item.searchableText = normalizeSearchableText(fallbackSearchableText(item));
        }
        sanitized.push_back(std::move(item));
    }

    if (sameResourceListItems(m_sourceItems, sanitized))
    {
        return;
    }

    m_sourceItems = std::move(sanitized);
    applySearchFilter();
}

const QVector<ResourcesListItem>& ResourcesListModel::items() const noexcept
{
    return m_items;
}

void ResourcesListModel::applySearchFilter()
{
    const QString previousNoteId = currentNoteId();
    const QString previousBodyText = currentBodyText();
    const QVariantMap previousResourceEntry = currentResourceEntry();
    const int previousIndex = m_currentIndex;
    const int previousCount = m_items.size();
    const QStringList terms = searchTerms(m_searchText);

    QVector<ResourcesListItem> filtered;
    if (terms.isEmpty())
    {
        filtered = m_sourceItems;
    }
    else
    {
        filtered.reserve(m_sourceItems.size());
        for (const ResourcesListItem& item : std::as_const(m_sourceItems))
        {
            if (itemMatchesSearch(item, terms))
            {
                filtered.push_back(item);
            }
        }
    }

    beginResetModel();
    m_items = std::move(filtered);
    endResetModel();

    int nextCurrentIndex = indexOfItemById(m_items, previousNoteId);
    if (nextCurrentIndex < 0 && previousIndex >= 0 && !m_items.isEmpty())
    {
        nextCurrentIndex = std::clamp(previousIndex, 0, static_cast<int>(m_items.size()) - 1);
    }
    m_currentIndex = nextCurrentIndex;

    const int nextCount = m_items.size();
    if (nextCount != previousCount)
    {
        emit itemCountChanged(nextCount);
    }
    if (previousIndex != m_currentIndex)
    {
        emit currentIndexChanged();
    }
    if (currentNoteId() != previousNoteId)
    {
        emit currentNoteIdChanged();
    }
    if (currentBodyText() != previousBodyText)
    {
        emit currentBodyTextChanged();
    }
    if (currentResourceEntry() != previousResourceEntry)
    {
        emit currentResourceEntryChanged();
    }
    emit itemsChanged();
}
