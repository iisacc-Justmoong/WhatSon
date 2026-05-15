#include "app/models/hierarchy/preset/PresetHierarchyController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "app/models/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "app/models/hierarchy/preset/PresetHierarchyControllerSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "preset.controller";
    const QString kKeyPrefix = QStringLiteral("preset");
}

PresetHierarchyController::PresetHierarchyController(QObject* parent)
    : IHierarchyController(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &WhatSonHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setPresetNames({});
}

PresetHierarchyController::~PresetHierarchyController() = default;

WhatSonHierarchyModel* PresetHierarchyController::itemModel() noexcept
{
    return &m_itemModel;
}

int PresetHierarchyController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int PresetHierarchyController::itemCount() const noexcept
{
    return m_itemCount;
}

bool PresetHierarchyController::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString PresetHierarchyController::lastLoadError() const
{
    return m_lastLoadError;
}

void PresetHierarchyController::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(
        index,
        m_itemModel.rowCount());
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

void PresetHierarchyController::setDepthItems(const QVariantList& depthItems)
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

QVariantList PresetHierarchyController::hierarchyModel() const
{
    return depthItems();
}

QVariantList PresetHierarchyController::depthItems() const
{
    return WhatSon::Hierarchy::NamedStringSupport::lvrsDepthItems(m_items, kKeyPrefix);
}

QString PresetHierarchyController::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool PresetHierarchyController::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::PresetSupport::isBucketHeaderItem(m_items.at(index));
}

bool PresetHierarchyController::renameItem(int index, const QString& displayName)
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

bool PresetHierarchyController::setItemExpanded(int index, bool expanded)
{
    return setHierarchyItemExpanded(
        &m_items,
        index,
        expanded,
        [this](int changedIndex, bool changedExpanded)
        {
            m_itemModel.setItemExpanded(changedIndex, changedExpanded);
            emit hierarchyModelChanged();
        });
}

void PresetHierarchyController::createFolder()
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

void PresetHierarchyController::deleteSelectedFolder()
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

void PresetHierarchyController::setPresetNames(QStringList presetNames)
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

QStringList PresetHierarchyController::presetNames() const
{
    return m_presetNames;
}

bool PresetHierarchyController::renameEnabled() const noexcept
{
    return true;
}

bool PresetHierarchyController::createFolderEnabled() const noexcept
{
    return false;
}

bool PresetHierarchyController::deleteFolderEnabled() const noexcept
{
    return false;
}

bool PresetHierarchyController::loadFromWshub(const QString& wshubPath, QString* errorMessage)
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

void PresetHierarchyController::applyRuntimeSnapshot(
    QStringList presetNames,
    QString presetFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? WhatSon::Hierarchy::NamedStringSupport::itemKey(m_items, m_selectedIndex, kKeyPrefix)
            : QString();
    const QSet<QString> preservedExpandedKeys =
        WhatSon::Hierarchy::NamedStringSupport::expandedItemKeys(m_items, kKeyPrefix);
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
    WhatSon::Hierarchy::NamedStringSupport::restoreExpandedItemKeys(&m_items, kKeyPrefix, preservedExpandedKeys);
    m_createdFolderSequence = WhatSon::Hierarchy::PresetSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(WhatSon::Hierarchy::NamedStringSupport::selectedIndexForKey(
        m_items,
        kKeyPrefix,
        preservedSelectionKey));
    updateLoadState(true);
}

void PresetHierarchyController::requestControllerHook()
{
    if (m_presetFilePath.trimmed().isEmpty())
    {
        emit controllerHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromPresetFilePath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit controllerHookRequested();
        return;
    }

    updateLoadState(true);
    emit controllerHookRequested();
}

bool PresetHierarchyController::reloadFromPresetFilePath(QString* errorMessage)
{
    const QString normalizedFilePath = m_presetFilePath.trimmed();
    if (normalizedFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    QStringList refreshedPresetNames;
    if (QFileInfo(normalizedFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::PresetSupport::readUtf8File(normalizedFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonPresetHierarchyStore refreshedStore;
        WhatSonPresetHierarchyParser parser;
        QString parseError;
        if (!parser.parse(rawText, &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        refreshedPresetNames = refreshedStore.presetNames();
    }

    applyRuntimeSnapshot(std::move(refreshedPresetNames), normalizedFilePath, true);
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void PresetHierarchyController::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void PresetHierarchyController::updateLoadState(bool succeeded, QString errorMessage)
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

void PresetHierarchyController::syncModel()
{
    m_itemModel.setItems(depthItems());
    updateItemCount();
    emit hierarchyModelChanged();
}

void PresetHierarchyController::syncDomainStoreFromItems()
{
    m_presetNames = WhatSon::Hierarchy::PresetSupport::extractDomainLabelsFromItems(m_items);
    m_store.setPresetNames(m_presetNames);
}
