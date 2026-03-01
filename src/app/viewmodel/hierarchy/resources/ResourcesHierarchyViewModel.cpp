#include "ResourcesHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp"

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
        &ResourcesHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setResourcePaths({});
}

ResourcesHierarchyViewModel::~ResourcesHierarchyViewModel() = default;

ResourcesHierarchyModel* ResourcesHierarchyViewModel::itemModel() noexcept
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
    const int clamped = WhatSon::Hierarchy::ResourcesSupport::clampSelectionIndex(index, m_itemModel.rowCount());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void ResourcesHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::ResourcesSupport::parseDepthItems(depthItems, QStringLiteral("Resource"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList ResourcesHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::ResourcesSupport::serializeDepthItems(m_items);
}

QString ResourcesHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ResourcesHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::ResourcesSupport::isBucketHeaderItem(m_items.at(index));
}

bool ResourcesHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!canRenameItem(index))
    {
        return false;
    }

    QVector<ResourcesHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::ResourcesSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        return false;
    }

    const QStringList stagedResourcePaths =
        WhatSon::Hierarchy::ResourcesSupport::extractDomainLabelsFromItems(stagedItems);
    WhatSonResourcesHierarchyStore stagedStore = m_store;
    stagedStore.setResourcePaths(stagedResourcePaths);

    if (!m_resourcesFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_resourcesFilePath, &writeError))
        {
            WhatSon::Debug::trace(
                QString::fromLatin1(kScope),
                QStringLiteral("renameItem.writeFailed"),
                QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(m_resourcesFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_resourcePaths = m_store.resourcePaths();
    syncModel();
    return true;
}

void ResourcesHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::ResourcesSupport::createHierarchyFolder(
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

    const int nextSelectedIndex = WhatSon::Hierarchy::ResourcesSupport::deleteHierarchySubtree(
        &m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void ResourcesHierarchyViewModel::setResourcePaths(QStringList resourcePaths)
{
    m_resourcePaths = WhatSon::Hierarchy::ResourcesSupport::sanitizeStringList(std::move(resourcePaths));
    m_store.setResourcePaths(m_resourcePaths);
    m_items = WhatSon::Hierarchy::ResourcesSupport::buildBucketItems(
        QStringLiteral("Resources"),
        m_resourcePaths,
        QStringLiteral("Resource"));
    m_createdFolderSequence = WhatSon::Hierarchy::ResourcesSupport::nextGeneratedFolderSequence(m_items);
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

    return !WhatSon::Hierarchy::ResourcesSupport::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool ResourcesHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_resourcesFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ResourcesSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
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

        if (m_resourcesFilePath.isEmpty())
        {
            m_resourcesFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ResourcesSupport::readUtf8File(filePath, &rawText, &readError))
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

    if (m_resourcesFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_resourcesFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Resources.wsresources"));
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
    m_resourcePaths = WhatSon::Hierarchy::ResourcesSupport::extractDomainLabelsFromItems(m_items);
    m_store.setResourcePaths(m_resourcePaths);
}
