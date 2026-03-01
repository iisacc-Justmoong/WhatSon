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
    syncModel();
    setSelectedIndex(-1);
}

QVariantList PresetHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

void PresetHierarchyViewModel::setPresetNames(QStringList presetNames)
{
    m_presetNames = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(presetNames));
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Preset"),
        m_presetNames,
        QStringLiteral("Preset"));
    syncModel();
    setSelectedIndex(-1);
}

QStringList PresetHierarchyViewModel::presetNames() const
{
    return m_presetNames;
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

        WhatSonPresetHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : store.presetNames())
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
