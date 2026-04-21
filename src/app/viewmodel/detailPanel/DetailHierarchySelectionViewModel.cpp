#include "app/viewmodel/detailPanel/DetailHierarchySelectionViewModel.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QVariantMap>

namespace
{
    constexpr auto kScope = "detail.hierarchy.selector.viewmodel";
}

DetailHierarchySelectionViewModel::DetailHierarchySelectionViewModel(
    const QString& objectName,
    QObject* parent)
    : QObject(parent)
{
    setObjectName(objectName.trimmed().isEmpty() ? QStringLiteral("DetailHierarchySelectionViewModel") : objectName);
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        QStringLiteral("ctor"),
        QStringLiteral("objectName=%1").arg(this->objectName()));
}

QVariantList DetailHierarchySelectionViewModel::hierarchyModel() const
{
    return m_hierarchyModel;
}

int DetailHierarchySelectionViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int DetailHierarchySelectionViewModel::itemCount() const noexcept
{
    return m_hierarchyModel.size();
}

QObject* DetailHierarchySelectionViewModel::sourceViewModel() const noexcept
{
    return m_sourceViewModel.data();
}

QString DetailHierarchySelectionViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_hierarchyModel.size())
    {
        return {};
    }

    return m_hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
}

void DetailHierarchySelectionViewModel::setSelectedIndex(int index)
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

void DetailHierarchySelectionViewModel::setSourceViewModel(QObject* sourceViewModel)
{
    if (m_sourceViewModel == sourceViewModel)
    {
        return;
    }

    disconnectSourceSignals();
    m_sourceViewModel = sourceViewModel;

    if (m_sourceViewModel)
    {
        m_sourceHierarchyModelChangedConnection = QObject::connect(
            m_sourceViewModel.data(),
            SIGNAL(hierarchyModelChanged()),
            this,
            SLOT(synchronizeFromSourceChange()));
        m_sourceSelectedIndexChangedConnection = QObject::connect(
            m_sourceViewModel.data(),
            SIGNAL(selectedIndexChanged()),
            this,
            SLOT(synchronizeSelectionFromSourceChange()));
        m_sourceDestroyedConnection = QObject::connect(
            m_sourceViewModel.data(),
            &QObject::destroyed,
            this,
            [this]()
            {
                disconnectSourceSignals();
                m_sourceViewModel = nullptr;
                emit sourceViewModelChanged();
                synchronizeFromSource(false);
            });
    }

    emit sourceViewModelChanged();
    synchronizeFromSource(true);
}

void DetailHierarchySelectionViewModel::disconnectSourceSignals()
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

void DetailHierarchySelectionViewModel::synchronizeFromSource(bool initializeSelectionFromSource)
{
    const QVariantList nextHierarchyModel = resolveHierarchyModelFromSource(m_sourceViewModel.data());
    const int previousItemCount = m_hierarchyModel.size();
    const QString preservedSelectionKey = initializeSelectionFromSource
        ? selectionKeyForIndex(nextHierarchyModel, resolveSelectedIndexFromSource(m_sourceViewModel.data()))
        : selectionKeyForIndex(m_hierarchyModel, m_selectedIndex);

    int nextSelectedIndex = -1;
    if (!preservedSelectionKey.isEmpty())
    {
        nextSelectedIndex = indexForSelectionKey(nextHierarchyModel, preservedSelectionKey);
    }
    else if (initializeSelectionFromSource)
    {
        nextSelectedIndex = normalizedIndexForModel(nextHierarchyModel, resolveSelectedIndexFromSource(m_sourceViewModel.data()));
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

QVariantList DetailHierarchySelectionViewModel::resolveHierarchyModelFromSource(const QObject* sourceViewModel)
{
    if (sourceViewModel == nullptr)
    {
        return {};
    }

    const QVariant sourceHierarchyModel = sourceViewModel->property("hierarchyModel");
    if (!sourceHierarchyModel.isValid())
    {
        return {};
    }

    return sourceHierarchyModel.toList();
}

int DetailHierarchySelectionViewModel::resolveSelectedIndexFromSource(const QObject* sourceViewModel)
{
    if (sourceViewModel == nullptr)
    {
        return -1;
    }

    bool ok = false;
    const int selectedIndex = sourceViewModel->property("selectedIndex").toInt(&ok);
    return ok ? selectedIndex : -1;
}

QString DetailHierarchySelectionViewModel::selectionKeyForIndex(const QVariantList& hierarchyModel, int index)
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

int DetailHierarchySelectionViewModel::indexForSelectionKey(const QVariantList& hierarchyModel, const QString& selectionKey)
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

int DetailHierarchySelectionViewModel::normalizedIndexForModel(const QVariantList& hierarchyModel, int index) noexcept
{
    if (index < 0 || index >= hierarchyModel.size())
    {
        return -1;
    }

    return index;
}
