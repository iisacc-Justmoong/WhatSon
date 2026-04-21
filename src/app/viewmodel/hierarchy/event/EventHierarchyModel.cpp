#include "EventHierarchyModel.hpp"

#include "models/file/WhatSonDebugTrace.hpp"

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

EventHierarchyModel::EventHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("hierarchy.event.model"), QStringLiteral("ctor"));
}

int EventHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant EventHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const EventHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
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

QHash<int, QByteArray> EventHierarchyModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"}
    };
}

int EventHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool EventHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void EventHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int EventHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString EventHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString EventHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void EventHierarchyModel::setItems(QVector<EventHierarchyItem> items)
{
    const int previousCount = m_items.size();
    QVector<EventHierarchyItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 2);

    for (int index = 0; index < items.size(); ++index)
    {
        EventHierarchyItem item = std::move(items[index]);
        if (item.depth < 0)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("hierarchy.depth.negative");
            issue.message = QStringLiteral("Depth must be >= 0. Corrected to 0.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalDepth"), item.depth},
                {QStringLiteral("correctedDepth"), 0}
            };
            item.depth = 0;
            issues.push_back(std::move(issue));
        }

        const QString originalLabel = item.label;
        item.label = item.label.trimmed();
        if (item.label.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("hierarchy.model"),
                                      QStringLiteral("setItems.emptyLabelKept"),
                                      QStringLiteral("index=%1 originalLabel=%2").arg(index).arg(originalLabel));
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
                              QStringLiteral("hierarchy.event.model"),
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

const QVector<EventHierarchyItem>& EventHierarchyModel::items() const noexcept
{
    return m_items;
}

void EventHierarchyModel::setValidationState(QString code, QString message)
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
