#include "LibraryNoteListModel.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "models/file/note/WhatSonBookmarkColorPalette.hpp"

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
    constexpr int kMaxNoteListPrimaryTextLines = 5;
    const QRegularExpression kSearchWhitespacePattern(QStringLiteral("\\s+"));

    struct ValidationIssue final
    {
        QString code;
        QString message;
        QVariantMap context;
    };

    QString truncateToMaxLines(const QString& value, int maxLines)
    {
        if (maxLines <= 0)
        {
            return {};
        }

        const QStringList lines = value.split(QLatin1Char('\n'));
        if (lines.size() <= maxLines)
        {
            return value;
        }

        QStringList truncated;
        truncated.reserve(maxLines);
        for (int i = 0; i < maxLines; ++i)
        {
            truncated.push_back(lines.at(i));
        }
        return truncated.join(QLatin1Char('\n'));
    }

    bool isValidHexColor(const QString& value)
    {
        static const QRegularExpression kHexColorPattern(QStringLiteral("^#(?:[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$"));
        return kHexColorPattern.match(value).hasMatch();
    }

    QString normalizePrimaryText(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        value = truncateToMaxLines(value, kMaxNoteListPrimaryTextLines);
        return value.trimmed();
    }

    QString normalizeSearchableText(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        value = value.trimmed().toCaseFolded();
        value.replace(kSearchWhitespacePattern, QStringLiteral(" "));
        return value.trimmed();
    }

    QString normalizeBodyText(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return value;
    }

    QString normalizeTimestamp(QString value)
    {
        return value.trimmed();
    }

    QDateTime parseNoteTimestamp(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        static const QStringList kFormats = {
            QStringLiteral("yyyy-MM-dd-HH-mm-ss"),
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"),
            QStringLiteral("yyyy-MM-dd")
        };

        for (const QString& format : kFormats)
        {
            const QDateTime parsed = QDateTime::fromString(trimmed, format);
            if (parsed.isValid())
            {
                return parsed;
            }
        }

        const QDateTime isoWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoWithMs.isValid())
        {
            return isoWithMs;
        }

        const QDateTime iso = QDateTime::fromString(trimmed, Qt::ISODate);
        if (iso.isValid())
        {
            return iso;
        }

        return {};
    }

    qint64 effectiveSortTimestamp(const LibraryNoteListItem& item)
    {
        const QDateTime lastModified = parseNoteTimestamp(item.lastModifiedAt);
        if (lastModified.isValid())
        {
            return lastModified.toMSecsSinceEpoch();
        }

        const QDateTime created = parseNoteTimestamp(item.createdAt);
        if (created.isValid())
        {
            return created.toMSecsSinceEpoch();
        }

        return std::numeric_limits<qint64>::min();
    }

    bool sameNoteListItem(const LibraryNoteListItem& lhs, const LibraryNoteListItem& rhs)
    {
        return lhs.id == rhs.id
            && lhs.primaryText == rhs.primaryText
            && lhs.searchableText == rhs.searchableText
            && lhs.bodyText == rhs.bodyText
            && lhs.createdAt == rhs.createdAt
            && lhs.lastModifiedAt == rhs.lastModifiedAt
            && lhs.image == rhs.image
            && lhs.imageSource == rhs.imageSource
            && lhs.displayDate == rhs.displayDate
            && lhs.folders == rhs.folders
            && lhs.tags == rhs.tags
            && lhs.bookmarked == rhs.bookmarked
            && lhs.bookmarkColor == rhs.bookmarkColor;
    }

    bool sameNoteListItems(
        const QVector<LibraryNoteListItem>& lhs,
        const QVector<LibraryNoteListItem>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            if (!sameNoteListItem(lhs.at(index), rhs.at(index)))
            {
                return false;
            }
        }

        return true;
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

    QString buildFallbackSearchableText(const LibraryNoteListItem& item)
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
        for (const QString& folder : item.folders)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }
        for (const QString& tag : item.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
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

    bool itemMatchesSearch(const LibraryNoteListItem& item, const QStringList& terms)
    {
        if (terms.isEmpty())
        {
            return true;
        }

        const QString searchableText = item.searchableText.isEmpty()
                                           ? normalizeSearchableText(buildFallbackSearchableText(item))
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

    QStringList sanitizeMetadataList(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());
        for (QString value : values)
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

    QString itemIdAt(const QVector<LibraryNoteListItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }
        return items.at(index).id;
    }

    QString itemBodyTextAt(const QVector<LibraryNoteListItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }
        return items.at(index).bodyText;
    }

    int indexOfItemById(const QVector<LibraryNoteListItem>& items, const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (items.at(index).id.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }

        return -1;
    }
}

LibraryNoteListModel::LibraryNoteListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.notelist.model"), QStringLiteral("ctor"));
}

int LibraryNoteListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant LibraryNoteListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const LibraryNoteListItem& item = m_items.at(index.row());
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
    default:
        return {};
    }
}

QHash<int, QByteArray> LibraryNoteListModel::roleNames() const
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
        {BookmarkColorRole, "bookmarkColor"}
    };
}

int LibraryNoteListModel::itemCount() const noexcept
{
    return m_items.size();
}

int LibraryNoteListModel::currentIndex() const noexcept
{
    return m_currentIndex;
}

QString LibraryNoteListModel::currentNoteId() const
{
    return itemIdAt(m_items, m_currentIndex);
}

QString LibraryNoteListModel::currentBodyText() const
{
    return itemBodyTextAt(m_items, m_currentIndex);
}

void LibraryNoteListModel::setCurrentIndex(int index)
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
}

QString LibraryNoteListModel::searchText() const
{
    return m_searchText;
}

void LibraryNoteListModel::setSearchText(const QString& text)
{
    if (m_searchText == text)
    {
        return;
    }

    m_searchText = text;
    applySearchFilter();
    emit searchTextChanged();
}

bool LibraryNoteListModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void LibraryNoteListModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int LibraryNoteListModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString LibraryNoteListModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString LibraryNoteListModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void LibraryNoteListModel::setItems(QVector<LibraryNoteListItem> items)
{
    QVector<LibraryNoteListItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 7);

    for (int index = 0; index < items.size(); ++index)
    {
        LibraryNoteListItem item = std::move(items[index]);
        const QString originalPrimaryText = item.primaryText;
        const QString originalDisplayDate = item.displayDate;
        const QStringList originalFolders = item.folders;
        const QStringList originalTags = item.tags;
        const QString originalColor = item.bookmarkColor;
        const QString originalImageSource = item.imageSource;

        item.id = item.id.trimmed();
        item.primaryText = normalizePrimaryText(std::move(item.primaryText));
        item.searchableText = normalizeSearchableText(std::move(item.searchableText));
        item.bodyText = normalizeBodyText(std::move(item.bodyText));
        item.createdAt = normalizeTimestamp(std::move(item.createdAt));
        item.lastModifiedAt = normalizeTimestamp(std::move(item.lastModifiedAt));
        item.imageSource = normalizeImageSource(std::move(item.imageSource));
        item.displayDate = item.displayDate.trimmed();
        item.bookmarkColor = item.bookmarkColor.trimmed();
        item.folders = sanitizeMetadataList(std::move(item.folders));
        item.tags = sanitizeMetadataList(std::move(item.tags));
        if (item.searchableText.isEmpty())
        {
            item.searchableText = normalizeSearchableText(buildFallbackSearchableText(item));
        }

        if (item.primaryText.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.notelist.model"),
                                      QStringLiteral("setItems.emptyPrimaryTextKept"),
                                      QStringLiteral("index=%1 originalPrimaryText=%2")
                                      .arg(index)
                                      .arg(originalPrimaryText));
        }

        if (item.folders != originalFolders)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.folders.sanitized");
            issue.message = QStringLiteral("Folders list was sanitized.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalCount"), originalFolders.size()},
                {QStringLiteral("sanitizedCount"), item.folders.size()}
            };
            issues.push_back(std::move(issue));
        }

        if (item.tags != originalTags)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.tags.sanitized");
            issue.message = QStringLiteral("Tags list was sanitized.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalCount"), originalTags.size()},
                {QStringLiteral("sanitizedCount"), item.tags.size()}
            };
            issues.push_back(std::move(issue));
        }

        if (item.displayDate != originalDisplayDate)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.displayDate.trimmed");
            issue.message = QStringLiteral("Display date was trimmed.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalDisplayDate"), originalDisplayDate},
                {QStringLiteral("correctedDisplayDate"), item.displayDate}
            };
            issues.push_back(std::move(issue));
        }

        if (item.image && item.imageSource != originalImageSource.trimmed())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.imageSource.normalized");
            issue.message = QStringLiteral("Image source was normalized.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalImageSource"), originalImageSource},
                {QStringLiteral("normalizedImageSource"), item.imageSource}
            };
            issues.push_back(std::move(issue));
        }

        if (!item.image && !item.imageSource.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.imageSource.cleared");
            issue.message = QStringLiteral("Image source was cleared because image=false.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalImageSource"), originalImageSource},
                {QStringLiteral("correctedImageSource"), QString()}
            };
            item.imageSource.clear();
            issues.push_back(std::move(issue));
        }

        const bool colorValid = isValidHexColor(item.bookmarkColor);
        if (item.bookmarked && !colorValid)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.bookmarkColor.invalid");
            issue.message = QStringLiteral("Bookmark color was invalid. Cleared.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalColor"), originalColor},
                {QStringLiteral("correctedColor"), QString()}
            };
            item.bookmarkColor.clear();
            issues.push_back(std::move(issue));
        }
        else if (!item.bookmarked && !item.bookmarkColor.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.bookmarkColor.cleared");
            issue.message = QStringLiteral("Bookmark color was cleared because bookmarked=false.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalColor"), originalColor},
                {QStringLiteral("correctedColor"), QString()}
            };
            item.bookmarkColor.clear();
            issues.push_back(std::move(issue));
        }

        sanitized.push_back(std::move(item));
    }

    std::stable_sort(
        sanitized.begin(),
        sanitized.end(),
        [](const LibraryNoteListItem& lhs, const LibraryNoteListItem& rhs)
        {
            const qint64 lhsTimestamp = effectiveSortTimestamp(lhs);
            const qint64 rhsTimestamp = effectiveSortTimestamp(rhs);
            if (lhsTimestamp == rhsTimestamp)
            {
                return false;
            }
            return lhsTimestamp > rhsTimestamp;
        });

    if (m_strictValidation && !issues.isEmpty())
    {
        const ValidationIssue& first = issues.constFirst();
        setValidationState(first.code, first.message);
        emit validationIssueRaised(first.code, first.message, first.context);
        throw std::runtime_error(first.message.toStdString());
    }

    if (sameNoteListItems(m_sourceItems, sanitized))
    {
        return;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.notelist.model"),
                              QStringLiteral("setItems"),
                              QStringLiteral("count=%1").arg(sanitized.size()));
    m_sourceItems = std::move(sanitized);
    applySearchFilter();
    if (!issues.isEmpty())
    {
        m_correctionCount += issues.size();
        emit correctionCountChanged();
        const ValidationIssue& last = issues.constLast();
        setValidationState(last.code, last.message);
        for (const ValidationIssue& issue : std::as_const(issues))
        {
            emit validationIssueRaised(issue.code, issue.message, issue.context);
            emit itemCorrected(issue.code, issue.context);
        }
    }
}

const QVector<LibraryNoteListItem>& LibraryNoteListModel::items() const noexcept
{
    return m_items;
}

void LibraryNoteListModel::applySearchFilter()
{
    const QString previousNoteId = currentNoteId();
    const QString previousBodyText = currentBodyText();
    const int previousIndex = m_currentIndex;
    const int previousCount = m_items.size();
    const QStringList terms = searchTerms(m_searchText);

    QVector<LibraryNoteListItem> filtered;
    if (terms.isEmpty())
    {
        filtered = m_sourceItems;
    }
    else
    {
        filtered.reserve(m_sourceItems.size());
        for (const LibraryNoteListItem& item : std::as_const(m_sourceItems))
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
    emit itemsChanged();
}

void LibraryNoteListModel::setValidationState(QString code, QString message)
{
    code = code.trimmed();
    message = message.trimmed();
    if (m_lastValidationCode == code && m_lastValidationMessage == message)
    {
        return;
    }
    m_lastValidationCode = std::move(code);
    m_lastValidationMessage = std::move(message);
    emit validationStateChanged();
}
