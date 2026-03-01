#include "EventHierarchyViewModel.hpp"

#include "file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

EventHierarchyViewModel::EventHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    setEventNames({});
}

EventHierarchyViewModel::~EventHierarchyViewModel() = default;

FlatHierarchyModel* EventHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int EventHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void EventHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void EventHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Event"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList EventHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString EventHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool EventHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled())
    {
        return false;
    }
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }
    if (WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(index)))
    {
        return false;
    }

    if (!WhatSon::Hierarchy::Support::renameFlatItem(&m_items, index, displayName))
    {
        return false;
    }

    syncDomainStoreFromItems();
    syncModel();
    return true;
}

void EventHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::Support::createFlatFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
}

void EventHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int nextSelectedIndex = WhatSon::Hierarchy::Support::deleteFlatSubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void EventHierarchyViewModel::setEventNames(QStringList eventNames)
{
    m_eventNames = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(eventNames));
    m_store.setEventNames(m_eventNames);
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Event"),
        m_eventNames,
        QStringLiteral("Event"));
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

QStringList EventHierarchyViewModel::eventNames() const
{
    return m_eventNames;
}

bool EventHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool EventHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool EventHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool EventHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::Support::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        return false;
    }

    QStringList aggregated;

    WhatSonEventHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Event.wsevent"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::Support::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : m_store.eventNames())
        {
            aggregated.push_back(value);
        }
    }

    setEventNames(aggregated);
    return true;
}

void EventHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}

void EventHierarchyViewModel::syncDomainStoreFromItems()
{
    m_eventNames = WhatSon::Hierarchy::Support::extractDomainLabelsFromItems(m_items);
    m_store.setEventNames(m_eventNames);
}
