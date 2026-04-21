#include "app/viewmodel/hierarchy/projects/ProjectsHierarchyModel.hpp"

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

    QString normalizedProjectsKeySegment(const QString& label, int index)
    {
        const QString normalizedLabel = label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }

        return QStringLiteral("item:%1").arg(index);
    }

    QString projectsHierarchyItemKey(const QVector<ProjectsHierarchyItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedProjectsKeySegment(items.at(index).label, index));

        int expectedDepth = items.at(index).depth;
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const ProjectsHierarchyItem& candidate = items.at(cursor);
            if (candidate.depth != expectedDepth - 1)
            {
                continue;
            }

            pathSegments.push_front(normalizedProjectsKeySegment(candidate.label, cursor));
            expectedDepth = candidate.depth;
        }

        return pathSegments.join(QLatin1Char('/'));
    }
}

ProjectsHierarchyModel::ProjectsHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("hierarchy.projects.model"), QStringLiteral("ctor"));
}

int ProjectsHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant ProjectsHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const ProjectsHierarchyItem& item = m_items.at(index.row());
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
    case ItemKeyRole:
        return projectsHierarchyItemKey(m_items, index.row());
    case IconNameRole:
        return projectsHierarchyIconName(item);
    default:
        return {};
    }
}

QHash<int, QByteArray> ProjectsHierarchyModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"},
        {ItemKeyRole, "itemKey"},
        {IconNameRole, "iconName"}
    };
}

int ProjectsHierarchyModel::itemCount() const noexcept
{
    return m_items.size();
}

bool ProjectsHierarchyModel::strictValidation() const noexcept
{
    return m_strictValidation;
}

void ProjectsHierarchyModel::setStrictValidation(bool enabled)
{
    if (m_strictValidation == enabled)
    {
        return;
    }
    m_strictValidation = enabled;
    emit strictValidationChanged();
}

int ProjectsHierarchyModel::correctionCount() const noexcept
{
    return m_correctionCount;
}

QString ProjectsHierarchyModel::lastValidationCode() const
{
    return m_lastValidationCode;
}

QString ProjectsHierarchyModel::lastValidationMessage() const
{
    return m_lastValidationMessage;
}

void ProjectsHierarchyModel::setItems(QVector<ProjectsHierarchyItem> items)
{
    const int previousCount = m_items.size();
    QVector<ProjectsHierarchyItem> sanitized;
    sanitized.reserve(items.size());

    QVector<ValidationIssue> issues;
    issues.reserve(items.size() * 2);

    for (int index = 0; index < items.size(); ++index)
    {
        ProjectsHierarchyItem item = std::move(items[index]);
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
                              QStringLiteral("hierarchy.projects.model"),
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

const QVector<ProjectsHierarchyItem>& ProjectsHierarchyModel::items() const noexcept
{
    return m_items;
}

void ProjectsHierarchyModel::setValidationState(QString code, QString message)
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
