#include "app/models/hierarchy/WhatSonHierarchyModel.hpp"

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

    QVariant firstNonEmptyStringValue(const QVariantMap& item, const QStringList& keys)
    {
        for (const QString& key : keys)
        {
            const QString value = item.value(key).toString().trimmed();
            if (!value.isEmpty())
            {
                return value;
            }
        }
        return {};
    }

    QString storageKeyForRole(int role)
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case WhatSonHierarchyModel::LabelRole:
            return QStringLiteral("label");
        case WhatSonHierarchyModel::CountRole:
            return QStringLiteral("count");
        case WhatSonHierarchyModel::DepthRole:
        case WhatSonHierarchyModel::IndentLevelRole:
            return QStringLiteral("depth");
        case WhatSonHierarchyModel::AccentRole:
            return QStringLiteral("accent");
        case WhatSonHierarchyModel::ExpandedRole:
            return QStringLiteral("expanded");
        case WhatSonHierarchyModel::ShowChevronRole:
            return QStringLiteral("showChevron");
        case WhatSonHierarchyModel::IconNameRole:
            return QStringLiteral("iconName");
        case WhatSonHierarchyModel::IconSourceRole:
            return QStringLiteral("iconSource");
        case WhatSonHierarchyModel::ItemKeyRole:
            return QStringLiteral("itemKey");
        case WhatSonHierarchyModel::KeyRole:
            return QStringLiteral("key");
        case WhatSonHierarchyModel::IdRole:
            return QStringLiteral("id");
        case WhatSonHierarchyModel::UuidRole:
            return QStringLiteral("uuid");
        case WhatSonHierarchyModel::KindRole:
            return QStringLiteral("kind");
        case WhatSonHierarchyModel::BucketRole:
            return QStringLiteral("bucket");
        case WhatSonHierarchyModel::TypeRole:
            return QStringLiteral("type");
        case WhatSonHierarchyModel::FormatRole:
            return QStringLiteral("format");
        case WhatSonHierarchyModel::ResourceIdRole:
            return QStringLiteral("resourceId");
        case WhatSonHierarchyModel::ResourcePathRole:
            return QStringLiteral("resourcePath");
        case WhatSonHierarchyModel::AssetPathRole:
            return QStringLiteral("assetPath");
        case WhatSonHierarchyModel::DraggableRole:
            return QStringLiteral("draggable");
        case WhatSonHierarchyModel::DragAllowedRole:
            return QStringLiteral("dragAllowed");
        case WhatSonHierarchyModel::MovableRole:
            return QStringLiteral("movable");
        case WhatSonHierarchyModel::DragLockedRole:
            return QStringLiteral("dragLocked");
        case WhatSonHierarchyModel::ProgressValueRole:
            return QStringLiteral("progressValue");
        case WhatSonHierarchyModel::ParentKeyRole:
            return QStringLiteral("parentKey");
        case WhatSonHierarchyModel::ParentItemKeyRole:
            return QStringLiteral("parentItemKey");
        default:
            return {};
        }
    }

    QList<int> changedRolesForRole(int role)
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case WhatSonHierarchyModel::LabelRole:
            return {Qt::DisplayRole, Qt::EditRole, WhatSonHierarchyModel::LabelRole};
        case WhatSonHierarchyModel::DepthRole:
        case WhatSonHierarchyModel::IndentLevelRole:
            return {WhatSonHierarchyModel::DepthRole, WhatSonHierarchyModel::IndentLevelRole};
        default:
            return {role};
        }
    }
}

WhatSonHierarchyModel::WhatSonHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("hierarchy.model"), QStringLiteral("ctor"));
}

int WhatSonHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant WhatSonHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const QVariantMap& item = m_items.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case LabelRole:
        return item.value(QStringLiteral("label"));
    case CountRole:
        return item.value(QStringLiteral("count"));
    case DepthRole:
    case IndentLevelRole:
        return item.value(QStringLiteral("depth"));
    case AccentRole:
        return item.value(QStringLiteral("accent"));
    case ExpandedRole:
        return item.value(QStringLiteral("expanded"));
    case ShowChevronRole:
        return item.value(QStringLiteral("showChevron"));
    case IconNameRole:
        return item.value(QStringLiteral("iconName"));
    case IconSourceRole:
        return item.value(QStringLiteral("iconSource"));
    case ItemKeyRole:
        return firstNonEmptyStringValue(
            item,
            {QStringLiteral("itemKey"), QStringLiteral("key"), QStringLiteral("resolvedItemKey")});
    case KeyRole:
        return firstNonEmptyStringValue(
            item,
            {QStringLiteral("key"), QStringLiteral("itemKey"), QStringLiteral("resolvedItemKey")});
    case IdRole:
        return item.value(QStringLiteral("id"));
    case UuidRole:
        return item.value(QStringLiteral("uuid"));
    case KindRole:
        return item.value(QStringLiteral("kind"));
    case BucketRole:
        return item.value(QStringLiteral("bucket"));
    case TypeRole:
        return item.value(QStringLiteral("type"));
    case FormatRole:
        return item.value(QStringLiteral("format"));
    case ResourceIdRole:
        return item.value(QStringLiteral("resourceId"));
    case ResourcePathRole:
        return item.value(QStringLiteral("resourcePath"));
    case AssetPathRole:
        return item.value(QStringLiteral("assetPath"));
    case DraggableRole:
        return item.value(QStringLiteral("draggable"));
    case DragAllowedRole:
        return item.value(QStringLiteral("dragAllowed"));
    case MovableRole:
        return item.value(QStringLiteral("movable"));
    case DragLockedRole:
        return item.value(QStringLiteral("dragLocked"));
    case ProgressValueRole:
        return item.value(QStringLiteral("progressValue"));
    case ParentKeyRole:
        return item.value(QStringLiteral("parentKey"));
    case ParentItemKeyRole:
        return item.value(QStringLiteral("parentItemKey"));
    default:
        return {};
    }
}

bool WhatSonHierarchyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size() || index.column() != 0)
    {
        return false;
    }

    const QString key = storageKeyForRole(role);
    if (key.isEmpty())
    {
        return false;
    }

    QVariantMap& item = m_items[index.row()];
    if (item.value(key) == value)
    {
        return true;
    }

    item.insert(key, value);
    emit dataChanged(index, index, changedRolesForRole(role));
    emit itemsChanged();
    return true;
}

Qt::ItemFlags WhatSonHierarchyModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlags = QAbstractListModel::flags(index);
    if (index.isValid() && index.row() >= 0 && index.row() < m_items.size() && index.column() == 0)
    {
        itemFlags |= Qt::ItemIsEditable;
    }
    return itemFlags;
}

bool WhatSonHierarchyModel::moveRows(
    const QModelIndex& sourceParent,
    int sourceRow,
    int count,
    const QModelIndex& destinationParent,
    int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid())
    {
        return false;
    }
    if (count <= 0 || sourceRow < 0 || sourceRow + count > m_items.size())
    {
        return false;
    }
    if (destinationChild < 0 || destinationChild > m_items.size())
    {
        return false;
    }
    if (destinationChild >= sourceRow && destinationChild <= sourceRow + count)
    {
        return false;
    }

    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    QVector<QVariantMap> movedItems;
    movedItems.reserve(count);
    for (int index = 0; index < count; ++index)
    {
        movedItems.push_back(m_items.at(sourceRow + index));
    }
    m_items.erase(m_items.begin() + sourceRow, m_items.begin() + sourceRow + count);
    const int insertionRow = destinationChild > sourceRow ? destinationChild - count : destinationChild;
    for (int index = 0; index < movedItems.size(); ++index)
    {
        m_items.insert(insertionRow + index, std::move(movedItems[index]));
    }
    endMoveRows();
    emit itemsChanged();
    return true;
}

QHash<int, QByteArray> WhatSonHierarchyModel::roleNames() const
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
        {IconSourceRole, "iconSource"},
        {ItemKeyRole, "itemKey"},
        {KeyRole, "key"},
        {IdRole, "id"},
        {UuidRole, "uuid"},
        {KindRole, "kind"},
        {BucketRole, "bucket"},
        {TypeRole, "type"},
        {FormatRole, "format"},
        {ResourceIdRole, "resourceId"},
        {ResourcePathRole, "resourcePath"},
        {AssetPathRole, "assetPath"},
        {DraggableRole, "draggable"},
        {DragAllowedRole, "dragAllowed"},
        {MovableRole, "movable"},
        {DragLockedRole, "dragLocked"},
        {ProgressValueRole, "progressValue"},
        {ParentKeyRole, "parentKey"},
        {ParentItemKeyRole, "parentItemKey"},
    };
}

int WhatSonHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool WhatSonHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void WhatSonHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int WhatSonHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString WhatSonHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString WhatSonHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void WhatSonHierarchyModel::setItems(const QVariantList& items)
{
    const int previousCount = m_items.size();
    QVector<QVariantMap> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 2);

    for (int index = 0; index < items.size(); ++index)
    {
        QVariantMap item = items.at(index).toMap();
        const int originalDepth = item.value(QStringLiteral("depth")).toInt();
        if (originalDepth < 0)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("hierarchy.depth.negative");
            issue.message = QStringLiteral("Depth must be >= 0. Corrected to 0.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("originalDepth"), originalDepth},
                {QStringLiteral("correctedDepth"), 0}
            };
            item.insert(QStringLiteral("depth"), 0);
            issues.push_back(std::move(issue));
        }

        if (item.contains(QStringLiteral("label")))
        {
            item.insert(QStringLiteral("label"), item.value(QStringLiteral("label")).toString().trimmed());
        }

        const int depth = item.value(QStringLiteral("depth")).toInt();
        if (item.value(QStringLiteral("accent")).toBool() && depth > 0)
        {
            ValidationIssue issue;
            issue.code = QStringLiteral("hierarchy.accent.invalidDepth");
            issue.message = QStringLiteral("Accent flag is only valid on depth 0. Corrected to false.");
            issue.context = QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("depth"), depth},
                {QStringLiteral("correctedAccent"), false}
            };
            item.insert(QStringLiteral("accent"), false);
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

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("hierarchy.model"),
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

bool WhatSonHierarchyModel::setItemExpanded(int index, bool expanded)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    QVariantMap& item = m_items[index];
    if (item.value(QStringLiteral("expanded")).toBool() == expanded)
    {
        return true;
    }

    item.insert(QStringLiteral("expanded"), expanded);
    const QModelIndex changedIndex = this->index(index, 0);
    emit dataChanged(changedIndex, changedIndex, {ExpandedRole});
    emit itemsChanged();
    return true;
}

QVariantList WhatSonHierarchyModel::items() const
{
    QVariantList result;
    result.reserve(m_items.size());
    for (const QVariantMap& item : m_items)
    {
        result.push_back(item);
    }
    return result;
}

void WhatSonHierarchyModel::setValidationState(QString code, QString message)
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
