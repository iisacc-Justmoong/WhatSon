#include "TagsHierarchyModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

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
}

TagsHierarchyModel::TagsHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("tags.model"), QStringLiteral("ctor"));
}

int TagsHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant TagsHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const TagsHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
    case IdRole:
        return item.id;
    case LabelRole:
        return item.label;
    case DepthRole:
    case IndentLevelRole:
        return item.depth;
    case AccentRole:
        return item.accent;
    case ExpandedRole:
        return item.expanded;
    case ShowChevronRole:
        {
            const int nextIndex = index.row() + 1;
            const bool hasChild = nextIndex < m_items.size()
                && m_items.at(nextIndex).depth > item.depth;
            return hasChild;
        }
    default:
        return {};
    }
}

QHash<int, QByteArray> TagsHierarchyModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {LabelRole, "label"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"}
    };
}

int TagsHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool TagsHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void TagsHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int TagsHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString TagsHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString TagsHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void TagsHierarchyModel::setItems(QVector<TagsHierarchyItem> items)
{
    const int previousCount = m_items.size();
    QVector<TagsHierarchyItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 3);

    for (int index = 0; index < items.size(); ++index)
    {
        TagsHierarchyItem item = std::move(items[index]);
        if (item.depth < 0)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("tags.depth.negative");
            issue.message = QStringLiteral("Depth must be >= 0. Corrected to 0.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalDepth"), item.depth},
                {QStringLiteral("correctedDepth"), 0}
            };
            item.depth = 0;
            issues.push_back(std::move(issue));
        }

        const QString originalId = item.id;
        const QString originalLabel = item.label;
        item.id = item.id.trimmed();
        item.label = item.label.trimmed();

        if (item.label.isEmpty() && !item.id.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("tags.label.empty");
            issue.message = QStringLiteral("Label was empty. Synchronized from id.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalLabel"), originalLabel},
                {QStringLiteral("correctedLabel"), item.id}
            };
            item.label = item.id;
            issues.push_back(std::move(issue));
        }
        if (item.id.isEmpty() && !item.label.isEmpty())
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("tags.id.empty");
            issue.message = QStringLiteral("Id was empty. Synchronized from label.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalId"), originalId},
                {QStringLiteral("correctedId"), item.label}
            };
            item.id = item.label;
            issues.push_back(std::move(issue));
        }
        if (item.id.isEmpty() && item.label.isEmpty())
        {
            const QString fallback = QStringLiteral("Tag%1").arg(index + 1);
            ValidationIssue issue;
            issue.code = QStringLiteral("tags.id_label.empty");
            issue.message = QStringLiteral("Tag id and label were empty. Generated fallback id/label.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("correctedValue"), fallback}
            };
            item.id = fallback;
            item.label = fallback;
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
        QStringLiteral("tags.model"),
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

const QVector<TagsHierarchyItem>& TagsHierarchyModel::items() const noexcept
{
    return m_items;
}

void TagsHierarchyModel::setValidationState(QString code, QString message)
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
