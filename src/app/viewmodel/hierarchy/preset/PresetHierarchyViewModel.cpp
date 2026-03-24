#include "PresetHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "preset.viewmodel";

    QString normalizedPresetKeySegment(const PresetHierarchyItem& item, int index)
    {
        const QString normalizedLabel = item.label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }
        return QStringLiteral("preset:%1").arg(index);
    }

    QString presetHierarchyItemKey(const QVector<PresetHierarchyItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedPresetKeySegment(items.at(index), index));

        int expectedDepth = std::max(0, items.at(index).depth);
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const PresetHierarchyItem& candidate = items.at(cursor);
            if (std::max(0, candidate.depth) != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedPresetKeySegment(candidate, cursor));
            expectedDepth = std::max(0, candidate.depth);
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    QSet<QString> expandedPresetItemKeys(const QVector<PresetHierarchyItem>& items)
    {
        QSet<QString> expandedKeys;
        for (int index = 0; index < items.size(); ++index)
        {
            if (!items.at(index).expanded)
            {
                continue;
            }
            expandedKeys.insert(presetHierarchyItemKey(items, index));
        }
        return expandedKeys;
    }

    void restoreExpandedPresetItemKeys(QVector<PresetHierarchyItem>* items, const QSet<QString>& expandedKeys)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            (*items)[index].expanded = expandedKeys.contains(presetHierarchyItemKey(*items, index));
        }
    }

    int selectedPresetIndexForKey(const QVector<PresetHierarchyItem>& items, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (presetHierarchyItemKey(items, index) == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }
}

PresetHierarchyViewModel::PresetHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &PresetHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setPresetNames({});
}

PresetHierarchyViewModel::~PresetHierarchyViewModel() = default;

PresetHierarchyModel* PresetHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int PresetHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int PresetHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool PresetHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString PresetHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void PresetHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::PresetSupport::clampSelectionIndex(index, m_itemModel.rowCount());
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

void PresetHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = WhatSon::Hierarchy::PresetSupport::parseDepthItems(depthItems, QStringLiteral("Preset"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList PresetHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList PresetHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::PresetSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), QStringLiteral("preset:%1").arg(index));
        serialized[index] = entry;
    }
    return serialized;
}

QString PresetHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool PresetHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::PresetSupport::isBucketHeaderItem(m_items.at(index));
}

bool PresetHierarchyViewModel::renameItem(int index, const QString& displayName)
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

    QVector<PresetHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::PresetSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=support rejected index=%1 label=%2").arg(index).arg(
                                      displayName));
        return false;
    }

    const QStringList stagedPresetNames = WhatSon::Hierarchy::PresetSupport::extractDomainLabelsFromItems(stagedItems);
    WhatSonPresetHierarchyStore stagedStore = m_store;
    stagedStore.setPresetNames(stagedPresetNames);

    if (!m_presetFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_presetFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_presetFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_presetNames = m_store.presetNames();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(displayName).arg(
                                  m_items.size()));
    return true;
}

bool PresetHierarchyViewModel::setItemExpanded(int index, bool expanded)
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

void PresetHierarchyViewModel::createFolder()
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

    const int insertIndex = WhatSon::Hierarchy::PresetSupport::createHierarchyFolder(
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

void PresetHierarchyViewModel::deleteSelectedFolder()
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

    const int nextSelectedIndex = WhatSon::Hierarchy::PresetSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 nextIndex=%2 itemCount=%3").arg(startIndex).arg(
                                  nextSelectedIndex).arg(m_items.size()));
}

void PresetHierarchyViewModel::setPresetNames(QStringList presetNames)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setPresetNames.begin"),
                              QStringLiteral("rawCount=%1").arg(presetNames.size()));
    m_presetNames = WhatSon::Hierarchy::PresetSupport::sanitizeStringList(std::move(presetNames));
    m_store.setPresetNames(m_presetNames);
    m_items = WhatSon::Hierarchy::PresetSupport::buildBucketItems(
        QStringLiteral("Preset"),
        m_presetNames,
        QStringLiteral("Preset"));
    m_createdFolderSequence = WhatSon::Hierarchy::PresetSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setPresetNames.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_presetNames.size()).arg(
                                  m_items.size()));
}

QStringList PresetHierarchyViewModel::presetNames() const
{
    return m_presetNames;
}

bool PresetHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool PresetHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool PresetHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool PresetHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_presetFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::PresetSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
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

    WhatSonPresetHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Preset.wspreset"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }
        fileFound = true;

        if (m_presetFilePath.isEmpty())
        {
            m_presetFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::PresetSupport::readUtf8File(filePath, &rawText, &readError))
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

        for (const QString& value : m_store.presetNames())
        {
            aggregated.push_back(value);
        }
    }

    if (m_presetFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_presetFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Preset.wspreset"));
    }

    setPresetNames(aggregated);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3").arg(wshubPath).arg(
                                  fileFound ? QStringLiteral("1") : QStringLiteral("0")).arg(m_presetNames.size()));
    updateLoadState(true);
    return true;
}

void PresetHierarchyViewModel::applyRuntimeSnapshot(
    QStringList presetNames,
    QString presetFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? presetHierarchyItemKey(m_items, m_selectedIndex)
            : QString();
    const QSet<QString> preservedExpandedKeys = expandedPresetItemKeys(m_items);
    m_presetFilePath = presetFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    const QStringList sanitizedPresetNames = WhatSon::Hierarchy::PresetSupport::sanitizeStringList(std::move(
        presetNames));
    if (m_presetNames == sanitizedPresetNames)
    {
        updateLoadState(true);
        return;
    }

    m_presetNames = sanitizedPresetNames;
    m_store.setPresetNames(m_presetNames);
    m_items = WhatSon::Hierarchy::PresetSupport::buildBucketItems(
        QStringLiteral("Preset"),
        m_presetNames,
        QStringLiteral("Preset"));
    restoreExpandedPresetItemKeys(&m_items, preservedExpandedKeys);
    m_createdFolderSequence = WhatSon::Hierarchy::PresetSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(selectedPresetIndexForKey(m_items, preservedSelectionKey));
    updateLoadState(true);
}

void PresetHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void PresetHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void PresetHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}

void PresetHierarchyViewModel::syncDomainStoreFromItems()
{
    m_presetNames = WhatSon::Hierarchy::PresetSupport::extractDomainLabelsFromItems(m_items);
    m_store.setPresetNames(m_presetNames);
}
