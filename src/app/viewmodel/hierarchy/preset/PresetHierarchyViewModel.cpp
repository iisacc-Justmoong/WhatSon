#include "PresetHierarchyViewModel.hpp"

#include "file/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

PresetHierarchyViewModel::PresetHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    setPresetNames({});
}

PresetHierarchyViewModel::~PresetHierarchyViewModel() = default;

FlatHierarchyModel* PresetHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int PresetHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void PresetHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void PresetHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Preset"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList PresetHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString PresetHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool PresetHierarchyViewModel::renameItem(int index, const QString& displayName)
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

void PresetHierarchyViewModel::createFolder()
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

void PresetHierarchyViewModel::deleteSelectedFolder()
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

void PresetHierarchyViewModel::setPresetNames(QStringList presetNames)
{
    m_presetNames = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(presetNames));
    m_store.setPresetNames(m_presetNames);
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Preset"),
        m_presetNames,
        QStringLiteral("Preset"));
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
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

    return !WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool PresetHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
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

    WhatSonPresetHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Preset.wspreset"));
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

        for (const QString& value : m_store.presetNames())
        {
            aggregated.push_back(value);
        }
    }

    setPresetNames(aggregated);
    return true;
}

void PresetHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}

void PresetHierarchyViewModel::syncDomainStoreFromItems()
{
    m_presetNames = WhatSon::Hierarchy::Support::extractDomainLabelsFromItems(m_items);
    m_store.setPresetNames(m_presetNames);
}
