#include "FlatHierarchyModel.hpp"

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

FlatHierarchyModel::FlatHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("hierarchy.flat.model"), QStringLiteral("ctor"));
}

int FlatHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant FlatHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const FlatHierarchyItem& item = m_items.at(index.row());
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
        return item.showChevron;
    default:
        return {};
    }
}

QHash<int, QByteArray> FlatHierarchyModel::roleNames() const
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

int FlatHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool FlatHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void FlatHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int FlatHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString FlatHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString FlatHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void FlatHierarchyModel::setItems(QVector<FlatHierarchyItem> items)
{
    const int previousCount = m_items.size();
    QVector<FlatHierarchyItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 2);

    for (int index = 0; index < items.size(); ++index)
    {
        FlatHierarchyItem item = std::move(items[index]);
        if (item.depth < 0)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("flat.depth.negative");
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
            ValidationIssue issue;
            issue.code = QStringLiteral("flat.label.empty");
            issue.message = QStringLiteral("Label must not be empty. Generated fallback label.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalLabel"), originalLabel},
                {QStringLiteral("correctedLabel"), QStringLiteral("Item%1").arg(index + 1)}
            };
            item.label = QStringLiteral("Item%1").arg(index + 1);
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
        QStringLiteral("hierarchy.flat.model"),
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

const QVector<FlatHierarchyItem>& FlatHierarchyModel::items() const noexcept
{
    return m_items;
}

void FlatHierarchyModel::setValidationState(QString code, QString message)
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
