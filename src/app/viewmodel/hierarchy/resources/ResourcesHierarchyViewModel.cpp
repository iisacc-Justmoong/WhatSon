#include "ResourcesHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <utility>

namespace
{
    constexpr auto kScope = "resources.viewmodel";
}

ResourcesHierarchyViewModel::ResourcesHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void ResourcesHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = WhatSon::Hierarchy::ResourcesSupport::parseDepthItems(depthItems, QStringLiteral("Resource"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList ResourcesHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList ResourcesHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::ResourcesSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), QStringLiteral("resources:%1").arg(index));
        serialized[index] = entry;
    }
    return serialized;
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
    Q_UNUSED(index);
    return false;
}

bool ResourcesHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=resources taxonomy is read-only index=%1").arg(index));
    return false;
}

bool ResourcesHierarchyViewModel::setItemExpanded(int index, bool expanded)
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

void ResourcesHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=resources taxonomy is read-only"));
}

void ResourcesHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=resources taxonomy is read-only selectedIndex=%1").arg(
                                  startIndex));
}

void ResourcesHierarchyViewModel::setResourcePaths(QStringList resourcePaths)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setResourcePaths.begin"),
                              QStringLiteral("rawCount=%1").arg(resourcePaths.size()));
    m_resourcePaths = WhatSon::Hierarchy::ResourcesSupport::sanitizeStringList(std::move(resourcePaths));
    m_store.setResourcePaths(m_resourcePaths);
    if (m_items.isEmpty())
    {
        m_items = WhatSon::Hierarchy::ResourcesSupport::buildSupportedTypeItems();
        m_createdFolderSequence = WhatSon::Hierarchy::ResourcesSupport::nextGeneratedFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
    }
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setResourcePaths.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_resourcePaths.size()).arg(
                                  m_items.size()));
}

QStringList ResourcesHierarchyViewModel::resourcePaths() const
{
    return m_resourcePaths;
}

bool ResourcesHierarchyViewModel::renameEnabled() const noexcept
{
    return false;
}

bool ResourcesHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool ResourcesHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool ResourcesHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
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
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    QStringList aggregated;
    bool fileFound = false;

    WhatSonResourcesHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Resources.wsresources"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }
        fileFound = true;

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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3").arg(wshubPath).arg(
                                  fileFound ? QStringLiteral("1") : QStringLiteral("0")).arg(m_resourcePaths.size()));
    updateLoadState(true);
    return true;
}

void ResourcesHierarchyViewModel::applyRuntimeSnapshot(
    QStringList resourcePaths,
    QString resourcesFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_resourcesFilePath = resourcesFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    setResourcePaths(std::move(resourcePaths));
    updateLoadState(true);
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
    emit hierarchyModelChanged();
}

void ResourcesHierarchyViewModel::syncDomainStoreFromItems()
{
    m_resourcePaths = WhatSon::Hierarchy::ResourcesSupport::extractDomainLabelsFromItems(m_items);
    m_store.setResourcePaths(m_resourcePaths);
}
