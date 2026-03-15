#include "SidebarHierarchyLvrsAdapter.hpp"

#include <QMetaMethod>
#include <QVariant>

#include <utility>

namespace
{
    QVariantList invokeDepthItems(QObject* object)
    {
        QVariantList depthItems;
        if (object == nullptr)
        {
            return depthItems;
        }

        QMetaObject::invokeMethod(
            object,
            "depthItems",
            Qt::DirectConnection,
            Q_RETURN_ARG(QVariantList, depthItems));
        return depthItems;
    }

    bool hasInvokable(const QObject* object, const char* signature)
    {
        return object != nullptr && object->metaObject()->indexOfMethod(signature) >= 0;
    }
}

SidebarHierarchyLvrsAdapter::SidebarHierarchyLvrsAdapter(QObject* parent)
    : QObject(parent)
{
}

SidebarHierarchyLvrsAdapter::~SidebarHierarchyLvrsAdapter() = default;

QObject* SidebarHierarchyLvrsAdapter::hierarchyViewModel() const noexcept
{
    return m_hierarchyViewModel;
}

void SidebarHierarchyLvrsAdapter::setHierarchyViewModel(QObject* viewModel)
{
    if (m_hierarchyViewModel == viewModel)
    {
        return;
    }

    disconnectViewModelSignals();
    m_hierarchyViewModel = viewModel;
    connectViewModelSignals();
    rebuild();
    emit hierarchyViewModelChanged();
}

QString SidebarHierarchyLvrsAdapter::searchQuery() const
{
    return m_searchQuery;
}

void SidebarHierarchyLvrsAdapter::setSearchQuery(QString query)
{
    query = query.trimmed();
    if (m_searchQuery == query)
    {
        return;
    }

    m_searchQuery = std::move(query);
    rebuild();
    emit searchQueryChanged();
}

QVariantList SidebarHierarchyLvrsAdapter::nodes() const
{
    return m_nodes;
}

QVariantList SidebarHierarchyLvrsAdapter::flatNodes() const
{
    return m_flatNodes;
}

QString SidebarHierarchyLvrsAdapter::selectedItemKey() const
{
    return m_selectedItemKey;
}

bool SidebarHierarchyLvrsAdapter::editable() const noexcept
{
    return m_editable;
}

bool SidebarHierarchyLvrsAdapter::noteDropEnabled() const noexcept
{
    return m_noteDropEnabled;
}

void SidebarHierarchyLvrsAdapter::activateKey(const QString& itemKey)
{
    if (m_hierarchyViewModel == nullptr)
    {
        return;
    }

    const int sourceIndex = sourceIndexForKey(itemKey);
    if (sourceIndex < 0)
    {
        return;
    }

    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "setSelectedIndex",
        Qt::DirectConnection,
        Q_ARG(int, sourceIndex));
}

bool SidebarHierarchyLvrsAdapter::canRenameKey(const QString& itemKey) const
{
    const int sourceIndex = sourceIndexForKey(itemKey);
    if (sourceIndex < 0 || m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    bool accepted = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "canRenameItem",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, accepted),
        Q_ARG(int, sourceIndex));
    return accepted;
}

QString SidebarHierarchyLvrsAdapter::labelForKey(const QString& itemKey) const
{
    const QString normalizedKey = itemKey.trimmed();
    return m_keyToLabel.value(normalizedKey);
}

bool SidebarHierarchyLvrsAdapter::renameKey(const QString& itemKey, const QString& displayName)
{
    const int sourceIndex = sourceIndexForKey(itemKey);
    if (sourceIndex < 0 || m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    bool renamed = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "renameItem",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, renamed),
        Q_ARG(int, sourceIndex),
        Q_ARG(QString, displayName));
    return renamed;
}

bool SidebarHierarchyLvrsAdapter::commitEditableNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
{
    if (!m_editable || m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    return invokeBooleanMethod("applyHierarchyNodes", hierarchyNodes, activeItemKey);
}

bool SidebarHierarchyLvrsAdapter::canAcceptNoteDrop(const QString& itemKey, const QString& noteId) const
{
    const int sourceIndex = sourceIndexForKey(itemKey);
    if (sourceIndex < 0 || m_hierarchyViewModel == nullptr || !m_noteDropEnabled)
    {
        return false;
    }

    bool accepted = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "canAcceptNoteDrop",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, accepted),
        Q_ARG(int, sourceIndex),
        Q_ARG(QString, noteId.trimmed()));
    return accepted;
}

bool SidebarHierarchyLvrsAdapter::assignNoteToKey(const QString& itemKey, const QString& noteId)
{
    const int sourceIndex = sourceIndexForKey(itemKey);
    if (sourceIndex < 0 || m_hierarchyViewModel == nullptr || !m_noteDropEnabled)
    {
        return false;
    }

    bool assigned = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "assignNoteToFolder",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, assigned),
        Q_ARG(int, sourceIndex),
        Q_ARG(QString, noteId.trimmed()));
    return assigned;
}

void SidebarHierarchyLvrsAdapter::connectViewModelSignals()
{
    if (m_hierarchyViewModel == nullptr)
    {
        return;
    }

    const QMetaObject* metaObject = m_hierarchyViewModel->metaObject();
    const int slotIndex = this->metaObject()->indexOfMethod("rebuildFromViewModelSignal()");
    if (slotIndex < 0)
    {
        return;
    }
    const QMetaMethod slotMethod = this->metaObject()->method(slotIndex);
    const auto connectIfPresent = [this, metaObject, slotMethod](const char* signalSignature)
    {
        const int signalIndex = metaObject->indexOfSignal(signalSignature);
        if (signalIndex < 0)
        {
            return;
        }

        const QMetaMethod signalMethod = metaObject->method(signalIndex);
        m_connections.push_back(QObject::connect(
            m_hierarchyViewModel,
            signalMethod,
            this,
            slotMethod,
            Qt::DirectConnection));
    };

    connectIfPresent("selectedIndexChanged()");
    connectIfPresent("itemCountChanged()");
    connectIfPresent("loadStateChanged()");
    connectIfPresent("noteItemCountChanged()");
}

void SidebarHierarchyLvrsAdapter::disconnectViewModelSignals()
{
    for (const QMetaObject::Connection& connection : std::as_const(m_connections))
    {
        QObject::disconnect(connection);
    }
    m_connections.clear();
}

void SidebarHierarchyLvrsAdapter::rebuild()
{
    QVariantList nextNodes;
    QVariantList nextFlatNodes;
    QString nextSelectedKey;
    bool nextEditable = false;
    bool nextNoteDropEnabled = false;
    QHash<QString, int> nextKeyToSourceIndex;
    QHash<QString, QString> nextKeyToLabel;

    if (m_hierarchyViewModel != nullptr)
    {
        QVector<WhatSon::Sidebar::Lvrs::FlatNode> flatNodes =
            WhatSon::Sidebar::Lvrs::flattenHierarchyNodes(invokeDepthItems(m_hierarchyViewModel));

        for (WhatSon::Sidebar::Lvrs::FlatNode& node : flatNodes)
        {
            bool dragLocked = false;
            if (hasInvokable(m_hierarchyViewModel, "canMoveFolder(int)"))
            {
                bool movable = false;
                QMetaObject::invokeMethod(
                    m_hierarchyViewModel,
                    "canMoveFolder",
                    Qt::DirectConnection,
                    Q_RETURN_ARG(bool, movable),
                    Q_ARG(int, node.sourceIndex));
                dragLocked = !movable;
            }
            node.dragLocked = dragLocked;
        }

        const QVector<WhatSon::Sidebar::Lvrs::FlatNode> filteredNodes =
            WhatSon::Sidebar::Lvrs::filterFlatNodes(flatNodes, m_searchQuery);

        nextNodes = WhatSon::Sidebar::Lvrs::flatNodesToVariantList(filteredNodes);
        nextFlatNodes = nextNodes;
        nextEditable = m_searchQuery.isEmpty()
            && hasInvokable(m_hierarchyViewModel, "applyHierarchyNodes(QVariantList,QString)");
        nextNoteDropEnabled = hasInvokable(m_hierarchyViewModel, "assignNoteToFolder(int,QString)")
            && hasInvokable(m_hierarchyViewModel, "canAcceptNoteDrop(int,QString)");

        for (const WhatSon::Sidebar::Lvrs::FlatNode& node : flatNodes)
        {
            nextKeyToSourceIndex.insert(node.key, node.sourceIndex);
            nextKeyToLabel.insert(node.key, node.label);
        }

        const int selectedIndex = m_hierarchyViewModel->property("selectedIndex").toInt();
        for (const WhatSon::Sidebar::Lvrs::FlatNode& node : flatNodes)
        {
            if (node.sourceIndex == selectedIndex)
            {
                nextSelectedKey = node.key;
                break;
            }
        }
    }

    const bool selectedChanged = m_selectedItemKey != nextSelectedKey;
    const bool capabilitiesDiffer = m_editable != nextEditable || m_noteDropEnabled != nextNoteDropEnabled;
    const bool nodesDiffer = m_nodes != nextNodes || m_flatNodes != nextFlatNodes;

    m_keyToSourceIndex = std::move(nextKeyToSourceIndex);
    m_keyToLabel = std::move(nextKeyToLabel);
    m_nodes = std::move(nextNodes);
    m_flatNodes = std::move(nextFlatNodes);
    m_selectedItemKey = std::move(nextSelectedKey);
    m_editable = nextEditable;
    m_noteDropEnabled = nextNoteDropEnabled;

    if (nodesDiffer)
    {
        emit nodesChanged();
    }
    if (selectedChanged)
    {
        emit selectedItemKeyChanged();
    }
    if (capabilitiesDiffer)
    {
        emit capabilitiesChanged();
    }
}

void SidebarHierarchyLvrsAdapter::rebuildFromViewModelSignal()
{
    rebuild();
}

int SidebarHierarchyLvrsAdapter::sourceIndexForKey(const QString& itemKey) const
{
    return m_keyToSourceIndex.value(itemKey.trimmed(), -1);
}

bool SidebarHierarchyLvrsAdapter::invokeBooleanMethod(
    const char* methodName,
    const QVariantList& nodes,
    const QString& activeItemKey) const
{
    if (m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    bool accepted = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        methodName,
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, accepted),
        Q_ARG(QVariantList, nodes),
        Q_ARG(QString, activeItemKey.trimmed()));
    return accepted;
}

bool SidebarHierarchyLvrsAdapter::invokeBooleanMethod(
    const char* methodName,
    int index,
    const QString& text) const
{
    if (m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    bool accepted = false;
    QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        methodName,
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, accepted),
        Q_ARG(int, index),
        Q_ARG(QString, text));
    return accepted;
}
