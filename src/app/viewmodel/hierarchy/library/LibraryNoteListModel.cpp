#include "LibraryNoteListModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QRegularExpression>
#include <stdexcept>
#include <utility>

namespace
{
    struct ValidationIssue final
    {
        QString code;
        QString message;
        QVariantMap context;
    };

    bool isValidHexColor(const QString& value)
    {
        static const QRegularExpression kHexColorPattern(QStringLiteral("^#(?:[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$"));
        return kHexColorPattern.match(value).hasMatch();
    }
}

LibraryNoteListModel::LibraryNoteListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("library.notelist.model"), QStringLiteral("ctor"));
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
    case NoteIdRole:
        return item.noteId;
    case TitleTextRole:
        return item.titleText;
    case SummaryTextRole:
        return item.summaryText;
    case FoldersTextRole:
        return item.foldersText;
    case BookmarkedRole:
        return item.bookmarked;
    case BookmarkColorHexRole:
        return item.bookmarkColorHex;
    case HighlightedRole:
        return item.highlighted;
    default:
        return {};
    }
}

QHash<int, QByteArray> LibraryNoteListModel::roleNames() const
{
    return {
        {NoteIdRole, "noteId"},
        {TitleTextRole, "titleText"},
        {SummaryTextRole, "summaryText"},
        {FoldersTextRole, "foldersText"},
        {BookmarkedRole, "bookmarked"},
        {BookmarkColorHexRole, "bookmarkColorHex"},
        {HighlightedRole, "highlighted"}
    };
}

int LibraryNoteListModel::itemCount() const noexcept
{
    return m_items.size();
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
    const int previousCount = m_items.size();
    QVector<LibraryNoteListItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 4);

    for (int index = 0; index < items.size(); ++index)
    {
        LibraryNoteListItem item = std::move(items[index]);
        const QString originalTitle = item.titleText;
        const QString originalSummary = item.summaryText;
        const QString originalFolders = item.foldersText;
        const QString originalColor = item.bookmarkColorHex;

        item.noteId = item.noteId.trimmed();
        item.titleText = item.titleText.trimmed();
        item.summaryText = item.summaryText.trimmed();
        item.foldersText = item.foldersText.trimmed();
        item.bookmarkColorHex = item.bookmarkColorHex.trimmed();

        if (item.titleText.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.title.empty");
            issue.message = QStringLiteral("Title was empty. Generated fallback title.");
            const QString fallbackTitle = !item.noteId.isEmpty()
                                              ? item.noteId
                                              : QStringLiteral("Untitled Note");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalTitle"), originalTitle},
                {QStringLiteral("correctedTitle"), fallbackTitle}
            };
            item.titleText = fallbackTitle;
            issues.push_back(std::move(issue));
        }
        if (item.summaryText.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.summary.empty");
            issue.message = QStringLiteral("Summary was empty. Replaced with default message.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalSummary"), originalSummary},
                {QStringLiteral("correctedSummary"), QStringLiteral("No contents")}
            };
            item.summaryText = QStringLiteral("No contents");
            issues.push_back(std::move(issue));
        }
        if (item.foldersText.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.folders.empty");
            issue.message = QStringLiteral("Folders text was empty. Replaced with default message.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalFolders"), originalFolders},
                {QStringLiteral("correctedFolders"), QStringLiteral("No Folder")}
            };
            item.foldersText = QStringLiteral("No Folder");
            issues.push_back(std::move(issue));
        }

        const bool colorValid = isValidHexColor(item.bookmarkColorHex);
        if (item.bookmarked && !colorValid)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.bookmarkColor.invalid");
            issue.message = QStringLiteral("Bookmark color was invalid. Replaced with default palette color.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalColor"), originalColor},
                {QStringLiteral("correctedColor"), WhatSon::Bookmarks::defaultBookmarkColorHex()}
            };
            item.bookmarkColorHex = WhatSon::Bookmarks::defaultBookmarkColorHex();
            issues.push_back(std::move(issue));
        }
        else if (!item.bookmarked && !item.bookmarkColorHex.isEmpty() && !colorValid)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("notelist.color.invalid");
            issue.message = QStringLiteral("Non-bookmark color was invalid. Cleared color.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalColor"), originalColor},
                {QStringLiteral("correctedColor"), QString()}
            };
            item.bookmarkColorHex.clear();
            issues.push_back(std::move(issue));
        }

        sanitized.push_back(std::move(item));
    }

    if (m_strictValidation && !issues.isEmpty())
    {
        const ValidationIssue& first = issues.constFirst();
        setValidationState(first.code, first.message);
        emit validationIssueRaised(first.code, first.message, first.context);
        throw std::runtime_error(first.message.toStdString());
    }

    WhatSon::Debug::trace(
        QStringLiteral("library.notelist.model"),
        QStringLiteral("setItems"),
        QStringLiteral("count=%1").arg(sanitized.size()));
    beginResetModel();
    m_items = std::move(sanitized);
    endResetModel();
    const int nextCount = m_items.size();
    if (nextCount != previousCount)
    {
        emit itemCountChanged(nextCount);
    }
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
    emit itemsChanged();
}

const QVector<LibraryNoteListItem>& LibraryNoteListModel::items() const noexcept
{
    return m_items;
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
