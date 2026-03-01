#include "LibraryNoteListModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QRegularExpression>
#include <stdexcept>
#include <utility>

namespace
{
    constexpr int kMaxNoteListDescLines = 5;

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

    QString normalizeDesc(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        value = truncateToMaxLines(value, kMaxNoteListDescLines);
        return value.trimmed();
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
        return item.id;
    case TitleRole:
        return item.title;
    case DescRole:
        return item.desc;
    case FoldersRole:
        return item.folders;
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
        {TitleRole, "title"},
        {DescRole, "desc"},
        {FoldersRole, "folders"},
        {BookmarkedRole, "bookmarked"},
        {BookmarkColorRole, "bookmarkColor"}
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
        const QString originalTitle = item.title;
        const QString originalDesc = item.desc;
        const QStringList originalFolders = item.folders;
        const QString originalColor = item.bookmarkColor;

        item.id = item.id.trimmed();
        item.title = item.title.trimmed();
        item.desc = normalizeDesc(std::move(item.desc));
        item.bookmarkColor = item.bookmarkColor.trimmed();

        QStringList folders;
        folders.reserve(item.folders.size());
        for (QString folder : item.folders)
        {
            folder = folder.trimmed();
            if (folder.isEmpty() || folders.contains(folder))
            {
                continue;
            }
            folders.push_back(std::move(folder));
        }
        item.folders = std::move(folders);

        if (item.desc.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.notelist.model"),
                                      QStringLiteral("setItems.emptyDescKept"),
                                      QStringLiteral("index=%1 originalDesc=%2").arg(index).arg(originalDesc));
        }

        if (item.title.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.notelist.model"),
                                      QStringLiteral("setItems.emptyTitleKept"),
                                      QStringLiteral("index=%1 originalTitle=%2").arg(index).arg(originalTitle));
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

    if (m_strictValidation && !issues.isEmpty())
    {
        const ValidationIssue& first = issues.constFirst();
        setValidationState(first.code, first.message);
        emit validationIssueRaised(first.code, first.message, first.context);
        throw std::runtime_error(first.message.toStdString());
    }

    WhatSon::Debug::traceSelf(this,
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
