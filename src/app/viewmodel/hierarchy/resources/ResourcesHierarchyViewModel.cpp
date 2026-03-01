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
    QObject::connect(
        &m_itemModel,
        &FlatHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
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

int ResourcesHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool ResourcesHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString ResourcesHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void ResourcesHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_itemModel.rowCount());
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
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList ResourcesHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString ResourcesHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ResourcesHierarchyViewModel::renameItem(int index, const QString& displayName)
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

void ResourcesHierarchyViewModel::createFolder()
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

void ResourcesHierarchyViewModel::deleteSelectedFolder()
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

void ResourcesHierarchyViewModel::setResourcePaths(QStringList resourcePaths)
{
    m_resourcePaths = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(resourcePaths));
    m_store.setResourcePaths(m_resourcePaths);
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Resources"),
        m_resourcePaths,
        QStringLiteral("Resource"));
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

QStringList ResourcesHierarchyViewModel::resourcePaths() const
{
    return m_resourcePaths;
}

bool ResourcesHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool ResourcesHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool ResourcesHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(m_selectedIndex));
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
        updateLoadState(false, resolveError);
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

        for (const QString& value : m_store.resourcePaths())
        {
            aggregated.push_back(value);
        }
    }

    setResourcePaths(aggregated);
    updateLoadState(true);
    return true;
}

void ResourcesHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void ResourcesHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void ResourcesHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
}

void ResourcesHierarchyViewModel::syncDomainStoreFromItems()
{
    m_resourcePaths = WhatSon::Hierarchy::Support::extractDomainLabelsFromItems(m_items);
    m_store.setResourcePaths(m_resourcePaths);
}
