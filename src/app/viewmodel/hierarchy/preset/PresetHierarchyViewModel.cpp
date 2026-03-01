#include "PresetHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    constexpr auto kScope = "preset.viewmodel";
}

PresetHierarchyViewModel::PresetHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
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
    emit selectedIndexChanged();
}

void PresetHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::PresetSupport::parseDepthItems(depthItems, QStringLiteral("Preset"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList PresetHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::PresetSupport::serializeDepthItems(m_items);
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
    if (!canRenameItem(index))
    {
        return false;
    }

    QVector<PresetHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::PresetSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
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
            WhatSon::Debug::trace(
                QString::fromLatin1(kScope),
                QStringLiteral("renameItem.writeFailed"),
                QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(m_presetFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_presetNames = m_store.presetNames();
    syncModel();
    return true;
}

void PresetHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::PresetSupport::createHierarchyFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
}

void PresetHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int nextSelectedIndex = WhatSon::Hierarchy::PresetSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void PresetHierarchyViewModel::setPresetNames(QStringList presetNames)
{
    m_presetNames = WhatSon::Hierarchy::PresetSupport::sanitizeStringList(std::move(presetNames));
    m_store.setPresetNames(m_presetNames);
    m_items = WhatSon::Hierarchy::PresetSupport::buildBucketItems(
        QStringLiteral("Preset"),
        m_presetNames,
        QStringLiteral("Preset"));
    m_createdFolderSequence = WhatSon::Hierarchy::PresetSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
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
    return true;
}

bool PresetHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::PresetSupport::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool PresetHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_presetFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::PresetSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
        return false;
    }

    QStringList aggregated;

    WhatSonPresetHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Preset.wspreset"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

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
    updateLoadState(true);
    return true;
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
}

void PresetHierarchyViewModel::syncDomainStoreFromItems()
{
    m_presetNames = WhatSon::Hierarchy::PresetSupport::extractDomainLabelsFromItems(m_items);
    m_store.setPresetNames(m_presetNames);
}
