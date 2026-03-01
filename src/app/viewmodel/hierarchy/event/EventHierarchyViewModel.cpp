#include "EventHierarchyViewModel.hpp"

#include "file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

EventHierarchyViewModel::EventHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    QObject::connect(
        &m_itemModel,
        &EventHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setEventNames({});
}

EventHierarchyViewModel::~EventHierarchyViewModel() = default;

EventHierarchyModel* EventHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int EventHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int EventHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool EventHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString EventHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void EventHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::EventSupport::clampSelectionIndex(index, m_itemModel.rowCount());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void EventHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::EventSupport::parseDepthItems(depthItems, QStringLiteral("Event"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList EventHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::EventSupport::serializeDepthItems(m_items);
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
    if (WhatSon::Hierarchy::EventSupport::isBucketHeaderItem(m_items.at(index)))
    {
        return false;
    }

    if (!WhatSon::Hierarchy::EventSupport::renameHierarchyItem(&m_items, index, displayName))
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

    const int insertIndex = WhatSon::Hierarchy::EventSupport::createHierarchyFolder(
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

    const int nextSelectedIndex = WhatSon::Hierarchy::EventSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void EventHierarchyViewModel::setEventNames(QStringList eventNames)
{
    m_eventNames = WhatSon::Hierarchy::EventSupport::sanitizeStringList(std::move(eventNames));
    m_store.setEventNames(m_eventNames);
    m_items = WhatSon::Hierarchy::EventSupport::buildBucketItems(
        QStringLiteral("Event"),
        m_eventNames,
        QStringLiteral("Event"));
    m_createdFolderSequence = WhatSon::Hierarchy::EventSupport::nextGeneratedFolderSequence(m_items);
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

    return !WhatSon::Hierarchy::EventSupport::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool EventHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::EventSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
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
        if (!WhatSon::Hierarchy::EventSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            updateLoadState(false, readError);
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            updateLoadState(false, parseError);
            return false;
        }

        for (const QString& value : m_store.eventNames())
        {
            aggregated.push_back(value);
        }
    }

    setEventNames(aggregated);
    updateLoadState(true);
    return true;
}

void EventHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void EventHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    const QString normalizedError = succeeded ? QString() : errorMessage;
    const bool shouldEmit = (m_loadSucceeded != succeeded) || (m_lastLoadError != normalizedError);
    m_loadSucceeded = succeeded;
    m_lastLoadError = normalizedError;
    if (shouldEmit)
    {
        emit loadStateChanged();
    }
}

void EventHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
}

void EventHierarchyViewModel::syncDomainStoreFromItems()
{
    m_eventNames = WhatSon::Hierarchy::EventSupport::extractDomainLabelsFromItems(m_items);
    m_store.setEventNames(m_eventNames);
}
