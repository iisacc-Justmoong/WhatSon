#include "EventHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "event.viewmodel";

    QString normalizedEventKeySegment(const EventHierarchyItem& item, int index)
    {
        const QString normalizedLabel = item.label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }
        return QStringLiteral("event:%1").arg(index);
    }

    QString eventHierarchyItemKey(const QVector<EventHierarchyItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedEventKeySegment(items.at(index), index));

        int expectedDepth = std::max(0, items.at(index).depth);
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const EventHierarchyItem& candidate = items.at(cursor);
            if (std::max(0, candidate.depth) != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedEventKeySegment(candidate, cursor));
            expectedDepth = std::max(0, candidate.depth);
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    QSet<QString> expandedEventItemKeys(const QVector<EventHierarchyItem>& items)
    {
        QSet<QString> expandedKeys;
        for (int index = 0; index < items.size(); ++index)
        {
            if (!items.at(index).expanded)
            {
                continue;
            }
            expandedKeys.insert(eventHierarchyItemKey(items, index));
        }
        return expandedKeys;
    }

    void restoreExpandedEventItemKeys(QVector<EventHierarchyItem>* items, const QSet<QString>& expandedKeys)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            (*items)[index].expanded = expandedKeys.contains(eventHierarchyItemKey(*items, index));
        }
    }

    int selectedEventIndexForKey(const QVector<EventHierarchyItem>& items, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (eventHierarchyItemKey(items, index) == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }
} 

EventHierarchyViewModel::EventHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void EventHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = WhatSon::Hierarchy::EventSupport::parseDepthItems(depthItems, QStringLiteral("Event"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList EventHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList EventHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::EventSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), QStringLiteral("event:%1").arg(index));
        entry.insert(QStringLiteral("count"), 0);
        serialized[index] = entry;
    }
    return serialized;
}

QString EventHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool EventHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::EventSupport::isBucketHeaderItem(m_items.at(index));
}

bool EventHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    QVector<EventHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::EventSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=support rejected index=%1 label=%2").arg(index).arg(
                                      displayName));
        return false;
    }

    const QStringList stagedEventNames = WhatSon::Hierarchy::EventSupport::extractDomainLabelsFromItems(stagedItems);
    WhatSonEventHierarchyStore stagedStore = m_store;
    stagedStore.setEventNames(stagedEventNames);

    if (!m_eventFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_eventFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_eventFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_eventNames = m_store.eventNames();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(displayName).arg(
                                  m_items.size()));
    return true;
}

bool EventHierarchyViewModel::setItemExpanded(int index, bool expanded)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    if (!m_items.at(index).showChevron)
    {
        return false;
    }

    if (m_items.at(index).expanded == expanded)
    {
        return true;
    }

    m_items[index].expanded = expanded;
    syncModel();
    return true;
}

void EventHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    if (!createFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=createFolderEnabled false"));
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::EventSupport::createHierarchyFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=insertIndex invalid"));
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.success"),
                              QStringLiteral("insertIndex=%1 itemCount=%2").arg(insertIndex).arg(m_items.size()));
}

void EventHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (!deleteFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=deleteFolderEnabled false selectedIndex=%1").arg(startIndex));
        return;
    }

    const int nextSelectedIndex = WhatSon::Hierarchy::EventSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 nextIndex=%2 itemCount=%3").arg(startIndex).arg(
                                  nextSelectedIndex).arg(m_items.size()));
}

void EventHierarchyViewModel::setEventNames(QStringList eventNames)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setEventNames.begin"),
                              QStringLiteral("rawCount=%1").arg(eventNames.size()));
    m_eventNames = WhatSon::Hierarchy::EventSupport::sanitizeStringList(std::move(eventNames));
    m_store.setEventNames(m_eventNames);
    m_items = WhatSon::Hierarchy::EventSupport::buildBucketItems(
        QStringLiteral("Event"),
        m_eventNames,
        QStringLiteral("Event"));
    m_createdFolderSequence = WhatSon::Hierarchy::EventSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setEventNames.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_eventNames.size()).arg(
                                  m_items.size()));
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
    return false;
}

bool EventHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool EventHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_eventFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::EventSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    QStringList aggregated;
    bool fileFound = false;

    WhatSonEventHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Event.wsevent"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }
        fileFound = true;

        if (m_eventFilePath.isEmpty())
        {
            m_eventFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::EventSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.read"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, readError));
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
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.parse"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, parseError));
            updateLoadState(false, parseError);
            return false;
        }

        for (const QString& value : m_store.eventNames())
        {
            aggregated.push_back(value);
        }
    }

    if (m_eventFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_eventFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Event.wsevent"));
    }

    setEventNames(aggregated);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3").arg(wshubPath).arg(
                                  fileFound ? QStringLiteral("1") : QStringLiteral("0")).arg(m_eventNames.size()));
    updateLoadState(true);
    return true;
}

void EventHierarchyViewModel::applyRuntimeSnapshot(
    QStringList eventNames,
    QString eventFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? eventHierarchyItemKey(m_items, m_selectedIndex)
            : QString();
    const QSet<QString> preservedExpandedKeys = expandedEventItemKeys(m_items);
    m_eventFilePath = eventFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    const QStringList sanitizedEventNames = WhatSon::Hierarchy::EventSupport::sanitizeStringList(std::move(eventNames));
    if (m_eventNames == sanitizedEventNames)
    {
        updateLoadState(true);
        return;
    }

    m_eventNames = sanitizedEventNames;
    m_store.setEventNames(m_eventNames);
    m_items = WhatSon::Hierarchy::EventSupport::buildBucketItems(
        QStringLiteral("Event"),
        m_eventNames,
        QStringLiteral("Event"));
    restoreExpandedEventItemKeys(&m_items, preservedExpandedKeys);
    m_createdFolderSequence = WhatSon::Hierarchy::EventSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(selectedEventIndexForKey(m_items, preservedSelectionKey));
    updateLoadState(true);
}

void EventHierarchyViewModel::requestViewModelHook()
{
    if (m_eventFilePath.trimmed().isEmpty())
    {
        emit viewModelHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromEventFilePath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit viewModelHookRequested();
        return;
    }

    updateLoadState(true);
    emit viewModelHookRequested();
}

bool EventHierarchyViewModel::reloadFromEventFilePath(QString* errorMessage)
{
    const QString normalizedFilePath = m_eventFilePath.trimmed();
    if (normalizedFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    QStringList refreshedEventNames;
    if (QFileInfo(normalizedFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::EventSupport::readUtf8File(normalizedFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonEventHierarchyStore refreshedStore;
        WhatSonEventHierarchyParser parser;
        QString parseError;
        if (!parser.parse(rawText, &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        refreshedEventNames = refreshedStore.eventNames();
    }

    applyRuntimeSnapshot(std::move(refreshedEventNames), normalizedFilePath, true);
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
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
    emit hierarchyModelChanged();
}

void EventHierarchyViewModel::syncDomainStoreFromItems()
{
    m_eventNames = WhatSon::Hierarchy::EventSupport::extractDomainLabelsFromItems(m_items);
    m_store.setEventNames(m_eventNames);
}
