#include "ResourcesHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    constexpr auto kScope = "resources.viewmodel";
}

ResourcesHierarchyViewModel::ResourcesHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::trace(QString::fromLatin1(kScope), QStringLiteral("ctor"));
    setResourcePaths({});
}

ResourcesHierarchyViewModel::~ResourcesHierarchyViewModel() = default;

FlatHierarchyModel* ResourcesHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int ResourcesHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void ResourcesHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void ResourcesHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Resource"));
    syncModel();
    setSelectedIndex(-1);
}

QVariantList ResourcesHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

void ResourcesHierarchyViewModel::setResourcePaths(QStringList resourcePaths)
{
    m_resourcePaths = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(resourcePaths));
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Resources"),
        m_resourcePaths,
        QStringLiteral("Resource"));
    syncModel();
    setSelectedIndex(-1);
}

QStringList ResourcesHierarchyViewModel::resourcePaths() const
{
    return m_resourcePaths;
}

bool ResourcesHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
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

    WhatSonResourcesHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Resources.wsresources"));
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

        WhatSonResourcesHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : store.resourcePaths())
        {
            aggregated.push_back(value);
        }
    }

    setResourcePaths(aggregated);
    return true;
}

void ResourcesHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}
