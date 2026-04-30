#include "app/models/detailPanel/DetailHierarchySelectionController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QVariantMap>

namespace
{
    constexpr auto kScope = "detail.hierarchy.selector.controller";
}

DetailHierarchySelectionController::DetailHierarchySelectionController(
    const QString& objectName,
    QObject* parent)
    : QObject(parent)
{
    setObjectName(objectName.trimmed().isEmpty() ? QStringLiteral("DetailHierarchySelectionController") : objectName);
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        QStringLiteral("ctor"),
        QStringLiteral("objectName=%1").arg(this->objectName()));
}

QVariantList DetailHierarchySelectionController::hierarchyModel() const
{
    return m_hierarchyModel;
}

int DetailHierarchySelectionController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int DetailHierarchySelectionController::itemCount() const noexcept
{
    return m_hierarchyModel.size();
}

QObject* DetailHierarchySelectionController::sourceController() const noexcept
{
    return m_sourceController.data();
}

QString DetailHierarchySelectionController::itemLabel(int index) const
{
    if (index < 0 || index >= m_hierarchyModel.size())
    {
        return {};
    }

    return m_hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
}

void DetailHierarchySelectionController::setSelectedIndex(int index)
{
    const int clamped = normalizedIndexForModel(m_hierarchyModel, index);
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("objectName=%1 value=%2").arg(objectName()).arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void DetailHierarchySelectionController::setSourceController(QObject* sourceController)
{
    if (sourceController != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("DetailHierarchySelectionController::setSourceController")))
    {
        return;
    }

    if (sourceController == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailHierarchySelectionController::setSourceController")))
    {
        return;
    }

    if (m_sourceController == sourceController)
    {
        return;
    }

    disconnectSourceSignals();
    m_sourceController = sourceController;

    if (m_sourceController)
    {
        m_sourceHierarchyModelChangedConnection = QObject::connect(
            m_sourceController.data(),
            SIGNAL(hierarchyModelChanged()),
            this,
            SLOT(synchronizeFromSourceChange()));
        m_sourceSelectedIndexChangedConnection = QObject::connect(
            m_sourceController.data(),
            SIGNAL(selectedIndexChanged()),
            this,
            SLOT(synchronizeSelectionFromSourceChange()));
        m_sourceDestroyedConnection = QObject::connect(
            m_sourceController.data(),
            &QObject::destroyed,
            this,
            [this]()
            {
                disconnectSourceSignals();
                m_sourceController = nullptr;
                emit sourceControllerChanged();
                synchronizeFromSource(false);
            });
    }

    emit sourceControllerChanged();
    synchronizeFromSource(true);
}

void DetailHierarchySelectionController::disconnectSourceSignals()
{
    if (m_sourceHierarchyModelChangedConnection)
    {
        QObject::disconnect(m_sourceHierarchyModelChangedConnection);
        m_sourceHierarchyModelChangedConnection = QMetaObject::Connection();
    }
    if (m_sourceSelectedIndexChangedConnection)
    {
        QObject::disconnect(m_sourceSelectedIndexChangedConnection);
        m_sourceSelectedIndexChangedConnection = QMetaObject::Connection();
    }
    if (m_sourceDestroyedConnection)
    {
        QObject::disconnect(m_sourceDestroyedConnection);
        m_sourceDestroyedConnection = QMetaObject::Connection();
    }
}

void DetailHierarchySelectionController::synchronizeFromSource(bool initializeSelectionFromSource)
{
    const QVariantList nextHierarchyModel = resolveHierarchyModelFromSource(m_sourceController.data());
    const int previousItemCount = m_hierarchyModel.size();
    const QString preservedSelectionKey = initializeSelectionFromSource
        ? selectionKeyForIndex(nextHierarchyModel, resolveSelectedIndexFromSource(m_sourceController.data()))
        : selectionKeyForIndex(m_hierarchyModel, m_selectedIndex);

    int nextSelectedIndex = -1;
    if (!preservedSelectionKey.isEmpty())
    {
        nextSelectedIndex = indexForSelectionKey(nextHierarchyModel, preservedSelectionKey);
    }
    else if (initializeSelectionFromSource)
    {
        nextSelectedIndex = normalizedIndexForModel(nextHierarchyModel, resolveSelectedIndexFromSource(m_sourceController.data()));
    }
    else
    {
        nextSelectedIndex = normalizedIndexForModel(nextHierarchyModel, m_selectedIndex);
    }

    const bool hierarchyChanged = m_hierarchyModel != nextHierarchyModel;
    const bool itemCountDidChange = previousItemCount != nextHierarchyModel.size();
    const bool selectionChanged = m_selectedIndex != nextSelectedIndex;

    m_hierarchyModel = nextHierarchyModel;
    m_selectedIndex = nextSelectedIndex;

    if (hierarchyChanged)
    {
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kScope),
            QStringLiteral("synchronizeFromSource"),
            QStringLiteral("objectName=%1 itemCount=%2 selectedIndex=%3")
                .arg(objectName())
                .arg(m_hierarchyModel.size())
                .arg(m_selectedIndex));
        emit hierarchyModelChanged();
    }
    if (itemCountDidChange)
    {
        emit itemCountChanged();
    }
    if (selectionChanged)
    {
        emit selectedIndexChanged();
    }
}

QVariantList DetailHierarchySelectionController::resolveHierarchyModelFromSource(const QObject* sourceController)
{
    if (sourceController == nullptr)
    {
        return {};
    }

    const QVariant sourceHierarchyModel = sourceController->property("hierarchyModel");
    if (!sourceHierarchyModel.isValid())
    {
        return {};
    }

    return sourceHierarchyModel.toList();
}

int DetailHierarchySelectionController::resolveSelectedIndexFromSource(const QObject* sourceController)
{
    if (sourceController == nullptr)
    {
        return -1;
    }

    bool ok = false;
    const int selectedIndex = sourceController->property("selectedIndex").toInt(&ok);
    return ok ? selectedIndex : -1;
}

QString DetailHierarchySelectionController::selectionKeyForIndex(const QVariantList& hierarchyModel, int index)
{
    if (index < 0 || index >= hierarchyModel.size())
    {
        return {};
    }

    const QVariantMap entry = hierarchyModel.at(index).toMap();
    const QString key = entry.value(QStringLiteral("key")).toString().trimmed();
    if (!key.isEmpty())
    {
        return QStringLiteral("key:%1").arg(key);
    }

    const QString itemId = entry.value(QStringLiteral("itemId")).toString().trimmed();
    if (!itemId.isEmpty())
    {
        return QStringLiteral("itemId:%1").arg(itemId);
    }

    const QString label = entry.value(QStringLiteral("label")).toString().trimmed();
    if (!label.isEmpty())
    {
        return QStringLiteral("label:%1").arg(label);
    }

    return {};
}

int DetailHierarchySelectionController::indexForSelectionKey(const QVariantList& hierarchyModel, const QString& selectionKey)
{
    if (selectionKey.trimmed().isEmpty())
    {
        return -1;
    }

    for (int index = 0; index < hierarchyModel.size(); ++index)
    {
        if (selectionKeyForIndex(hierarchyModel, index) == selectionKey)
        {
            return index;
        }
    }

    return -1;
}

int DetailHierarchySelectionController::normalizedIndexForModel(const QVariantList& hierarchyModel, int index) noexcept
{
    if (index < 0 || index >= hierarchyModel.size())
    {
        return -1;
    }

    return index;
}
