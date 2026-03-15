#include "HierarchyDragDropBridge.hpp"

#include <QMetaObject>
#include <QMetaProperty>

namespace
{
    constexpr auto kApplyHierarchyNodesSignature = "applyHierarchyNodes(QVariantList,QString)";
    constexpr auto kAssignNoteToFolderSignature = "assignNoteToFolder(int,QString)";
    constexpr auto kCanAcceptNoteDropSignature = "canAcceptNoteDrop(int,QString)";
}

HierarchyDragDropBridge::HierarchyDragDropBridge(QObject* parent)
    : QObject(parent)
{
}

HierarchyDragDropBridge::~HierarchyDragDropBridge() = default;

QObject* HierarchyDragDropBridge::hierarchyViewModel() const noexcept
{
    return m_hierarchyViewModel;
}

void HierarchyDragDropBridge::setHierarchyViewModel(QObject* viewModel)
{
    if (m_hierarchyViewModel == viewModel)
    {
        return;
    }

    disconnectHierarchyViewModel();
    m_hierarchyViewModel = viewModel;

    if (m_hierarchyViewModel != nullptr)
    {
        m_hierarchyViewModelDestroyedConnection = connect(
            m_hierarchyViewModel,
            &QObject::destroyed,
            this,
            &HierarchyDragDropBridge::handleHierarchyViewModelDestroyed);
        m_hierarchyModelChangedConnection = connect(
            m_hierarchyViewModel,
            SIGNAL(hierarchyModelChanged()),
            this,
            SLOT(handleHierarchyModelChanged()));
        m_selectedIndexChangedConnection = connect(
            m_hierarchyViewModel,
            SIGNAL(selectedIndexChanged()),
            this,
            SLOT(handleSelectedIndexChanged()));
    }

    emit hierarchyViewModelChanged();
    refreshContractState();
    refreshSelectedItemKey();
}

bool HierarchyDragDropBridge::reorderContractAvailable() const noexcept
{
    return m_reorderContractAvailable;
}

bool HierarchyDragDropBridge::noteDropContractAvailable() const noexcept
{
    return m_noteDropContractAvailable;
}

QString HierarchyDragDropBridge::selectedItemKey() const
{
    return m_selectedItemKey;
}

bool HierarchyDragDropBridge::applyHierarchyReorder(
    const QVariantList& hierarchyNodes,
    const QString& activeItemKey)
{
    if (m_hierarchyViewModel == nullptr || !m_reorderContractAvailable || hierarchyNodes.isEmpty())
    {
        return false;
    }

    bool applied = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "applyHierarchyNodes",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, applied),
        Q_ARG(QVariantList, hierarchyNodes),
        Q_ARG(QString, resolvedActiveItemKey(activeItemKey)));
    return invoked && applied;
}

bool HierarchyDragDropBridge::canAcceptNoteDrop(int index, const QString& noteId) const
{
    if (m_hierarchyViewModel == nullptr || !m_noteDropContractAvailable)
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (index < 0 || normalizedNoteId.isEmpty())
    {
        return false;
    }

    bool accepted = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "canAcceptNoteDrop",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, accepted),
        Q_ARG(int, index),
        Q_ARG(QString, normalizedNoteId));
    return invoked && accepted;
}

bool HierarchyDragDropBridge::assignNoteToFolder(int index, const QString& noteId)
{
    if (m_hierarchyViewModel == nullptr || !m_noteDropContractAvailable)
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (index < 0 || normalizedNoteId.isEmpty())
    {
        return false;
    }

    bool assigned = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_hierarchyViewModel,
        "assignNoteToFolder",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, assigned),
        Q_ARG(int, index),
        Q_ARG(QString, normalizedNoteId));
    return invoked && assigned;
}

void HierarchyDragDropBridge::handleHierarchyModelChanged()
{
    refreshSelectedItemKey();
}

void HierarchyDragDropBridge::handleSelectedIndexChanged()
{
    refreshSelectedItemKey();
}

void HierarchyDragDropBridge::handleHierarchyViewModelDestroyed()
{
    disconnectHierarchyViewModel();
    m_hierarchyViewModel = nullptr;
    emit hierarchyViewModelChanged();
    refreshContractState();
    refreshSelectedItemKey();
}

bool HierarchyDragDropBridge::hasReadableProperty(const QObject* object, const char* propertyName)
{
    if (object == nullptr || propertyName == nullptr)
    {
        return false;
    }

    const int propertyIndex = object->metaObject()->indexOfProperty(propertyName);
    if (propertyIndex < 0)
    {
        return false;
    }

    return object->metaObject()->property(propertyIndex).isReadable();
}

bool HierarchyDragDropBridge::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QVariantList HierarchyDragDropBridge::readHierarchyModel() const
{
    if (!hasReadableProperty(m_hierarchyViewModel, "hierarchyModel"))
    {
        return {};
    }

    return m_hierarchyViewModel->property("hierarchyModel").toList();
}

QString HierarchyDragDropBridge::resolvedActiveItemKey(const QString& preferredActiveItemKey) const
{
    const QString normalizedPreferred = preferredActiveItemKey.trimmed();
    if (!normalizedPreferred.isEmpty())
    {
        return normalizedPreferred;
    }

    return m_selectedItemKey;
}

void HierarchyDragDropBridge::refreshContractState()
{
    const bool nextReorderContractAvailable =
        hasInvokableMethod(m_hierarchyViewModel, kApplyHierarchyNodesSignature);
    if (m_reorderContractAvailable != nextReorderContractAvailable)
    {
        m_reorderContractAvailable = nextReorderContractAvailable;
        emit reorderContractAvailableChanged();
    }

    const bool nextNoteDropContractAvailable =
        hasInvokableMethod(m_hierarchyViewModel, kAssignNoteToFolderSignature)
        && hasInvokableMethod(m_hierarchyViewModel, kCanAcceptNoteDropSignature);
    if (m_noteDropContractAvailable != nextNoteDropContractAvailable)
    {
        m_noteDropContractAvailable = nextNoteDropContractAvailable;
        emit noteDropContractAvailableChanged();
    }
}

void HierarchyDragDropBridge::refreshSelectedItemKey()
{
    QString nextSelectedItemKey;
    if (m_hierarchyViewModel != nullptr && hasReadableProperty(m_hierarchyViewModel, "selectedIndex"))
    {
        bool converted = false;
        const int selectedIndex = m_hierarchyViewModel->property("selectedIndex").toInt(&converted);
        if (converted)
        {
            const QVariantList hierarchyModel = readHierarchyModel();
            if (selectedIndex >= 0 && selectedIndex < hierarchyModel.size())
            {
                nextSelectedItemKey = hierarchyModel.at(selectedIndex)
                                                    .toMap()
                                                    .value(QStringLiteral("key"))
                                                    .toString()
                                                    .trimmed();
            }
        }
    }

    if (m_selectedItemKey == nextSelectedItemKey)
    {
        return;
    }

    m_selectedItemKey = nextSelectedItemKey;
    emit selectedItemKeyChanged();
}

void HierarchyDragDropBridge::disconnectHierarchyViewModel()
{
    if (m_hierarchyViewModelDestroyedConnection)
    {
        disconnect(m_hierarchyViewModelDestroyedConnection);
        m_hierarchyViewModelDestroyedConnection = QMetaObject::Connection();
    }
    if (m_hierarchyModelChangedConnection)
    {
        disconnect(m_hierarchyModelChangedConnection);
        m_hierarchyModelChangedConnection = QMetaObject::Connection();
    }
    if (m_selectedIndexChangedConnection)
    {
        disconnect(m_selectedIndexChangedConnection);
        m_selectedIndexChangedConnection = QMetaObject::Connection();
    }
}
