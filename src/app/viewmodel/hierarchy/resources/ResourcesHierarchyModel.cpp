#include "app/viewmodel/hierarchy/resources/ResourcesHierarchyModel.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

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

ResourcesHierarchyModel::ResourcesHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("hierarchy.resources.model"), QStringLiteral("ctor"));
}

int ResourcesHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant ResourcesHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const ResourcesHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
    case LabelRole:
        return item.label;
    case CountRole:
        return item.count;
    case DepthRole:
    case IndentLevelRole:
        return item.depth;
    case AccentRole:
        return item.accent;
    case ExpandedRole:
        return item.expanded;
    case ShowChevronRole:
        return item.showChevron;
    case IconNameRole:
        return resourcesHierarchyIconName(item);
    case KeyRole:
        return item.key;
    case KindRole:
        return item.kind;
    case BucketRole:
        return item.bucket;
    case TypeRole:
        return item.type;
    case FormatRole:
        return item.format;
    case ResourceIdRole:
        return item.resourceId;
    case ResourcePathRole:
        return item.resourcePath;
    case AssetPathRole:
        return item.assetPath;
    default:
        return {};
    }
}

QHash<int, QByteArray> ResourcesHierarchyModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {CountRole, "count"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"},
        {IconNameRole, "iconName"},
        {KeyRole, "key"},
        {KindRole, "kind"},
        {BucketRole, "bucket"},
        {TypeRole, "type"},
        {FormatRole, "format"},
        {ResourceIdRole, "resourceId"},
        {ResourcePathRole, "resourcePath"},
        {AssetPathRole, "assetPath"}
    };
}

int ResourcesHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool ResourcesHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void ResourcesHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int ResourcesHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString ResourcesHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString ResourcesHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void ResourcesHierarchyModel::setItems(QVector<ResourcesHierarchyItem> items)
{
    const int previousCount = m_items.size();
    QVector<ResourcesHierarchyItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 2);

    for (int index = 0; index < items.size(); ++index)
    {
        ResourcesHierarchyItem item = std::move(items[index]);
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
                              QStringLiteral("hierarchy.resources.model"),
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

const QVector<ResourcesHierarchyItem>& ResourcesHierarchyModel::items() const noexcept
{
    return m_items;
}

void ResourcesHierarchyModel::setValidationState(QString code, QString message)
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
