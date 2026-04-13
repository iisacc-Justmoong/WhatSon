#include "ResourcesHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QUrl>
#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kScope = "resources.viewmodel";

    QString resolveWshubPathFromResourcesFile(const QString& resourcesFilePath)
    {
        QFileInfo fileInfo(resourcesFilePath.trimmed());
        QString currentPath = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.absolutePath();
        while (!currentPath.isEmpty())
        {
            const QFileInfo currentInfo(currentPath);
            if (currentInfo.fileName().endsWith(QStringLiteral(".wshub")) && currentInfo.isDir())
            {
                return currentInfo.absoluteFilePath();
            }

            const QDir dir(currentPath);
            const QString parentPath = dir.absolutePath() == dir.rootPath() ? QString() : dir.filePath(
                QStringLiteral(".."));
            const QString normalizedParentPath = QFileInfo(parentPath).absoluteFilePath();
            if (normalizedParentPath.isEmpty() || normalizedParentPath == currentPath)
            {
                break;
            }
            currentPath = normalizedParentPath;
        }

        return {};
    }

    QString encodeXmlAttributeValue(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        return value;
    }

    QString normalizeSourceUrl(const QString& resolvedAssetPath)
    {
        const QString normalizedAssetPath = WhatSon::Resources::normalizePath(resolvedAssetPath.trimmed());
        if (normalizedAssetPath.isEmpty())
        {
            return {};
        }

        if (QFileInfo(normalizedAssetPath).isAbsolute())
        {
            return QUrl::fromLocalFile(QDir::cleanPath(normalizedAssetPath)).toString();
        }

        const QUrl assetUrl(normalizedAssetPath);
        if (assetUrl.isValid() && !assetUrl.scheme().isEmpty())
        {
            if (assetUrl.isLocalFile())
            {
                return QUrl::fromLocalFile(QDir::cleanPath(assetUrl.toLocalFile())).toString();
            }
            return assetUrl.toString();
        }

        return {};
    }

    bool isImageTypeOrFormat(const QString& typeKey, const QString& formatKey)
    {
        if (typeKey == QStringLiteral("image"))
        {
            return true;
        }

        static const QStringList kImageFormatKeys = {
            QStringLiteral(".png"),
            QStringLiteral(".jpeg"),
            QStringLiteral(".jpg"),
            QStringLiteral(".tiff"),
            QStringLiteral(".webp"),
            QStringLiteral(".gif"),
            QStringLiteral(".bmp"),
            QStringLiteral(".svg"),
            QStringLiteral(".heic"),
            QStringLiteral(".avif")
        };
        return !formatKey.isEmpty() && kImageFormatKeys.contains(formatKey);
    }

    QString renderModeForResourceListItem(
        const QString& typeKey,
        const QString& formatKey,
        const QString& sourceUrl)
    {
        if (isImageTypeOrFormat(typeKey, formatKey) && !sourceUrl.isEmpty())
        {
            return QStringLiteral("image");
        }

        if (typeKey == QStringLiteral("video") && !sourceUrl.isEmpty())
        {
            return QStringLiteral("video");
        }

        if ((typeKey == QStringLiteral("audio") || typeKey == QStringLiteral("music")) && !sourceUrl.isEmpty())
        {
            return QStringLiteral("audio");
        }

        if (formatKey == QStringLiteral(".pdf"))
        {
            return QStringLiteral("pdf");
        }

        static const QStringList kTextFormats = {
            QStringLiteral(".txt"),
            QStringLiteral(".md"),
            QStringLiteral(".csv"),
            QStringLiteral(".json"),
            QStringLiteral(".xml"),
            QStringLiteral(".yaml"),
            QStringLiteral(".yml"),
            QStringLiteral(".ini"),
            QStringLiteral(".log"),
            QStringLiteral(".rtf")
        };
        if (kTextFormats.contains(formatKey))
        {
            return QStringLiteral("text");
        }

        return QStringLiteral("document");
    }

    QString buildResourceTagBodyText(
        const QString& typeKey,
        const QString& formatKey,
        const QString& resourcePath)
    {
        const QString normalizedType = typeKey.trimmed().isEmpty() ? QStringLiteral("other") : typeKey.trimmed();
        const QString normalizedFormat = WhatSon::Resources::normalizeFormat(formatKey.trimmed()).isEmpty()
                                             ? QStringLiteral(".bin")
                                             : WhatSon::Resources::normalizeFormat(formatKey.trimmed());
        const QString normalizedPath = WhatSon::Resources::normalizePath(resourcePath.trimmed());
        if (normalizedPath.isEmpty())
        {
            return {};
        }

        return QStringLiteral("<resource type=\"%1\" format=\"%2\" path=\"%3\">")
            .arg(
                encodeXmlAttributeValue(normalizedType),
                encodeXmlAttributeValue(normalizedFormat),
                encodeXmlAttributeValue(normalizedPath));
    }

    bool resourceMatchesSelection(
        const QString& typeKey,
        const QString& formatKey,
        const ResourcesHierarchyItem* selectedItem)
    {
        if (selectedItem == nullptr)
        {
            return true;
        }

        if (selectedItem->kind == QStringLiteral("type"))
        {
            const QString selectedType = WhatSon::Resources::normalizedType(selectedItem->type);
            return selectedType.isEmpty() || typeKey == selectedType;
        }

        if (selectedItem->kind == QStringLiteral("format"))
        {
            const QString selectedType = WhatSon::Resources::normalizedType(selectedItem->type);
            const QString selectedFormat = WhatSon::Resources::normalizedFormatLookupKey(selectedItem->format);
            const bool typeMatched = selectedType.isEmpty() || typeKey == selectedType;
            const bool formatMatched = selectedFormat.isEmpty() || formatKey == selectedFormat;
            return typeMatched && formatMatched;
        }

        return true;
    }

    QString displayNameForResource(
        const WhatSon::Hierarchy::ResourcesSupport::MaterializedResourceEntry& materialized,
        const QString& resolvedPackagePath,
        const QString& resourcePath)
    {
        const QString resolvedAssetName = QFileInfo(materialized.resolvedAssetPath).fileName().trimmed();
        if (!resolvedAssetName.isEmpty())
        {
            return resolvedAssetName;
        }

        const QString packageStem = QFileInfo(resolvedPackagePath).completeBaseName().trimmed();
        if (!packageStem.isEmpty())
        {
            return packageStem;
        }

        if (!materialized.metadata.resourceId.trimmed().isEmpty())
        {
            return materialized.metadata.resourceId.trimmed();
        }

        const QString resourcePathName = QFileInfo(resourcePath).fileName().trimmed();
        if (!resourcePathName.isEmpty())
        {
            return resourcePathName;
        }
        return resourcePath;
    }

    QVector<ResourcesListItem> buildResourceNoteListItems(
        const QStringList& resourcePaths,
        const QStringList& resolutionBasePaths,
        const ResourcesHierarchyItem* selectedItem = nullptr)
    {
        QVector<ResourcesListItem> listItems;
        listItems.reserve(resourcePaths.size());

        for (const QString& rawResourcePath : resourcePaths)
        {
            const QString normalizedResourcePath = WhatSon::Resources::normalizePath(rawResourcePath.trimmed());
            if (normalizedResourcePath.isEmpty())
            {
                continue;
            }

            const WhatSon::Hierarchy::ResourcesSupport::MaterializedResourceEntry materialized =
                WhatSon::Hierarchy::ResourcesSupport::materializeResourceEntry(
                    normalizedResourcePath,
                    resolutionBasePaths);
            const QString typeKey = WhatSon::Hierarchy::ResourcesSupport::typeKeyForMetadata(materialized.metadata);
            QString formatKey = WhatSon::Resources::normalizedFormatLookupKey(materialized.metadata.format);
            if (formatKey.isEmpty())
            {
                formatKey = QStringLiteral(".bin");
            }

            if (!resourceMatchesSelection(typeKey, formatKey, selectedItem))
            {
                continue;
            }

            const QString resolvedPackagePath = WhatSon::Resources::resolvePackageDirectoryFromReference(
                normalizedResourcePath,
                resolutionBasePaths);
            const QString resourceReferencePath = materialized.metadata.resourcePath.trimmed().isEmpty()
                                                     ? normalizedResourcePath
                                                     : WhatSon::Resources::normalizePath(materialized.metadata.resourcePath);
            const QString noteId = !resolvedPackagePath.trimmed().isEmpty()
                                       ? WhatSon::Resources::normalizePath(resolvedPackagePath)
                                       : resourceReferencePath;
            const QString resourceTagBodyText = buildResourceTagBodyText(
                typeKey,
                formatKey,
                resourceReferencePath);
            const QString sourceUrl = normalizeSourceUrl(materialized.resolvedAssetPath);
            const bool imageResource = isImageTypeOrFormat(typeKey, formatKey) && !sourceUrl.isEmpty();

            ResourcesListItem listItem;
            listItem.id = noteId;
            listItem.primaryText = displayNameForResource(materialized, resolvedPackagePath, normalizedResourcePath);
            listItem.searchableText = QStringLiteral("%1 %2 %3 %4")
                                          .arg(
                                              listItem.primaryText,
                                              WhatSon::Resources::normalizeBucket(materialized.metadata.bucket),
                                              typeKey,
                                              formatKey)
                                      + QLatin1Char(' ')
                                      + resourceReferencePath;
            listItem.bodyText = resourceTagBodyText;
            listItem.image = imageResource;
            listItem.imageSource = imageResource ? sourceUrl : QString();
            listItem.displayDate = WhatSon::Resources::normalizeBucket(materialized.metadata.bucket).trimmed();
            listItem.folders = {
                WhatSon::Hierarchy::ResourcesSupport::displayLabelForTypeKey(typeKey)
            };
            listItem.tags = {formatKey};
            listItem.type = typeKey;
            listItem.format = formatKey;
            listItem.resourcePath = resourceReferencePath;
            listItem.resolvedPath = WhatSon::Resources::normalizePath(materialized.resolvedAssetPath);
            listItem.source = sourceUrl;
            listItem.renderMode = renderModeForResourceListItem(typeKey, formatKey, sourceUrl);
            listItem.displayName = listItem.primaryText;
            listItems.push_back(std::move(listItem));
        }

        return listItems;
    }

    QStringList normalizedUniqueResourceNoteIds(const QVariantList& noteIds)
    {
        QStringList normalized;
        normalized.reserve(noteIds.size());
        for (const QVariant& noteIdValue : noteIds)
        {
            const QString normalizedNoteId = WhatSon::Resources::normalizePath(noteIdValue.toString().trimmed());
            if (normalizedNoteId.isEmpty() || normalized.contains(normalizedNoteId))
            {
                continue;
            }
            normalized.push_back(normalizedNoteId);
        }
        return normalized;
    }

    int indexOfHierarchyItemByKey(const QVector<ResourcesHierarchyItem>& items, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (items.at(index).key.trimmed() == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }

    QString selectedHierarchyItemKey(const QVector<ResourcesHierarchyItem>& items, int selectedIndex)
    {
        if (selectedIndex < 0 || selectedIndex >= items.size())
        {
            return {};
        }
        return items.at(selectedIndex).key.trimmed();
    }

    QString resolveResourcePackagePathForNoteId(const QString& noteId, const QStringList& resolutionBasePaths)
    {
        const QString normalizedNoteId = WhatSon::Resources::normalizePath(noteId.trimmed());
        if (normalizedNoteId.isEmpty())
        {
            return {};
        }

        const QFileInfo directInfo(normalizedNoteId);
        if (directInfo.isDir() && WhatSon::Resources::isResourcePackageDirectoryName(directInfo.fileName()))
        {
            return directInfo.absoluteFilePath();
        }

        return WhatSon::Resources::resolvePackageDirectoryFromReference(
            normalizedNoteId,
            resolutionBasePaths);
    }

    QStringList resourceRemovalCandidates(
        const QString& noteId,
        const QString& packagePath,
        const WhatSon::Resources::ResourcePackageMetadata& metadata)
    {
        QStringList candidates;
        const auto appendCandidate = [&candidates](const QString& value)
        {
            const QString normalizedValue = WhatSon::Resources::normalizePath(value.trimmed());
            if (normalizedValue.isEmpty() || candidates.contains(normalizedValue))
            {
                return;
            }
            candidates.push_back(normalizedValue);
        };

        appendCandidate(noteId);
        appendCandidate(metadata.resourcePath);
        appendCandidate(WhatSon::Resources::resourcePathForPackageDirectory(packagePath));
        return candidates;
    }

    QStringList filteredResourcePathsAfterRemoval(
        const QStringList& existingResourcePaths,
        const QStringList& removalCandidates)
    {
        QSet<QString> removalSet;
        for (const QString& removalCandidate : removalCandidates)
        {
            const QString normalizedCandidate = WhatSon::Resources::normalizePath(removalCandidate.trimmed());
            if (!normalizedCandidate.isEmpty())
            {
                removalSet.insert(normalizedCandidate);
            }
        }

        QStringList filtered;
        filtered.reserve(existingResourcePaths.size());
        for (const QString& existingResourcePath : existingResourcePaths)
        {
            const QString normalizedPath = WhatSon::Resources::normalizePath(existingResourcePath.trimmed());
            if (normalizedPath.isEmpty() || removalSet.contains(normalizedPath) || filtered.contains(normalizedPath))
            {
                continue;
            }
            filtered.push_back(normalizedPath);
        }
        return filtered;
    }
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

ResourcesListModel* ResourcesHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
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
    refreshNoteListForSelection();
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
    const int previousSelectedIndex = m_selectedIndex;
    setSelectedIndex(-1);
    if (previousSelectedIndex == -1)
    {
        refreshNoteListForSelection();
    }
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
        if (entry.value(QStringLiteral("key")).toString().trimmed().isEmpty())
        {
            entry.insert(QStringLiteral("key"), QStringLiteral("resources:%1").arg(index));
        }
        entry.insert(QStringLiteral("count"), std::max(0, m_items.at(index).count));
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

bool ResourcesHierarchyViewModel::deleteNoteById(const QString& noteId)
{
    QVariantList noteIds;
    noteIds.push_back(noteId);
    return deleteNotesByIds(noteIds);
}

bool ResourcesHierarchyViewModel::deleteNotesByIds(const QVariantList& noteIds)
{
    const QStringList normalizedNoteIds = normalizedUniqueResourceNoteIds(noteIds);
    if (normalizedNoteIds.isEmpty())
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteNotesByIds.begin"),
                              QStringLiteral("count=%1").arg(normalizedNoteIds.size()));

    QStringList resolvedPackagePaths;
    resolvedPackagePaths.reserve(normalizedNoteIds.size());
    QStringList removalCandidates;
    removalCandidates.reserve(normalizedNoteIds.size() * 3);

    for (const QString& normalizedNoteId : normalizedNoteIds)
    {
        const QString resolvedPackagePath = resolveResourcePackagePathForNoteId(
            normalizedNoteId,
            m_resourceResolutionBasePaths);
        if (resolvedPackagePath.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("deleteNotesByIds.failed"),
                                      QStringLiteral("reason=unresolvedPackage noteId=%1").arg(normalizedNoteId));
            return false;
        }

        WhatSon::Resources::ResourcePackageMetadata metadata;
        QString metadataError;
        if (!WhatSon::Resources::loadResourcePackageMetadata(resolvedPackagePath, &metadata, &metadataError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("deleteNotesByIds.failed"),
                                      QStringLiteral("reason=metadataLoadFailed noteId=%1 error=%2")
                                          .arg(normalizedNoteId, metadataError));
            return false;
        }

        if (!resolvedPackagePaths.contains(resolvedPackagePath))
        {
            resolvedPackagePaths.push_back(resolvedPackagePath);
        }

        const QStringList candidateValues = resourceRemovalCandidates(
            normalizedNoteId,
            resolvedPackagePath,
            metadata);
        for (const QString& candidateValue : candidateValues)
        {
            if (!removalCandidates.contains(candidateValue))
            {
                removalCandidates.push_back(candidateValue);
            }
        }
    }

    const QString normalizedResourcesFilePath = m_resourcesFilePath.trimmed();
    if (normalizedResourcesFilePath.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("deleteNotesByIds.failed"),
                                  QStringLiteral("reason=resourcesFilePathEmpty"));
        return false;
    }

    const QStringList previousResourcePaths = m_resourcePaths;
    const QStringList nextResourcePaths = filteredResourcePathsAfterRemoval(
        previousResourcePaths,
        removalCandidates);

    WhatSonResourcesHierarchyStore stagedStore;
    stagedStore.setResourcePaths(nextResourcePaths);

    QString writeError;
    if (!stagedStore.writeToFile(normalizedResourcesFilePath, &writeError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("deleteNotesByIds.failed"),
                                  QStringLiteral("reason=resourcesFileWriteFailed error=%1").arg(writeError));
        return false;
    }

    for (const QString& resolvedPackagePath : resolvedPackagePaths)
    {
        if (QFileInfo(resolvedPackagePath).exists() && !QDir(resolvedPackagePath).removeRecursively())
        {
            WhatSonResourcesHierarchyStore rollbackStore;
            rollbackStore.setResourcePaths(previousResourcePaths);
            QString rollbackError;
            rollbackStore.writeToFile(normalizedResourcesFilePath, &rollbackError);

            const QString combinedError = rollbackError.trimmed().isEmpty()
                                              ? QString()
                                              : QStringLiteral(" rollback=%1").arg(rollbackError.trimmed());
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("deleteNotesByIds.failed"),
                                      QStringLiteral("reason=packageDeleteFailed path=%1%2")
                                          .arg(resolvedPackagePath, combinedError));
            return false;
        }
    }

    setResourcePaths(nextResourcePaths);
    updateLoadState(true);

    for (const QString& resolvedPackagePath : resolvedPackagePaths)
    {
        emit resourceDeleted(resolvedPackagePath);
    }
    emit hubFilesystemMutated();

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteNotesByIds.success"),
                              QStringLiteral("deleted=%1 remaining=%2")
                                  .arg(resolvedPackagePaths.size())
                                  .arg(m_resourcePaths.size()));
    return true;
}

QString ResourcesHierarchyViewModel::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = WhatSon::Resources::normalizePath(noteId.trimmed());
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const QFileInfo directInfo(normalizedNoteId);
    if (directInfo.isDir() && WhatSon::Resources::isResourcePackageDirectoryName(directInfo.fileName()))
    {
        return directInfo.absoluteFilePath();
    }

    const QString resolvedPackagePath = WhatSon::Resources::resolvePackageDirectoryFromReference(
        normalizedNoteId,
        m_resourceResolutionBasePaths);
    return resolvedPackagePath;
}

void ResourcesHierarchyViewModel::setResourcePaths(QStringList resourcePaths)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setResourcePaths.begin"),
                              QStringLiteral("rawCount=%1").arg(resourcePaths.size()));
    QStringList sanitizedPaths = WhatSon::Hierarchy::ResourcesSupport::sanitizeStringList(std::move(resourcePaths));
    QVector<ResourcesHierarchyItem> nextItems = WhatSon::Hierarchy::ResourcesSupport::buildHierarchyItems(
        sanitizedPaths,
        m_resourceResolutionBasePaths,
        m_items);
    const QString previousSelectedKey = selectedHierarchyItemKey(m_items, m_selectedIndex);

    if (m_resourcePaths == sanitizedPaths
        && WhatSon::Hierarchy::ResourcesSupport::hierarchyItemsEqual(m_items, nextItems))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("setResourcePaths.skipped"),
                                  QStringLiteral("reason=unchangedPayload count=%1").arg(sanitizedPaths.size()));
        return;
    }

    m_resourcePaths = std::move(sanitizedPaths);
    m_store.setResourcePaths(m_resourcePaths);
    m_items = std::move(nextItems);
    syncModel();
    const int previousSelectedIndex = m_selectedIndex;
    const int nextSelectedIndex = previousSelectedKey.isEmpty()
                                      ? -1
                                      : indexOfHierarchyItemByKey(m_items, previousSelectedKey);
    setSelectedIndex(nextSelectedIndex);
    if (previousSelectedIndex == nextSelectedIndex)
    {
        refreshNoteListForSelection();
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

    setResourceResolutionBasePaths({WhatSon::Resources::normalizePath(wshubPath)});

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

    if (aggregated.isEmpty())
    {
        aggregated = WhatSon::Resources::listRelativeResourcePackagePathsForHub(wshubPath);
    }

    setResourceResolutionBasePaths(
        WhatSon::Resources::resourceReferenceBasePathsForResourcesFile(m_resourcesFilePath)
        + QStringList{WhatSon::Resources::normalizePath(wshubPath)});
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

    setResourceResolutionBasePaths(
        WhatSon::Resources::resourceReferenceBasePathsForResourcesFile(m_resourcesFilePath));
    setResourcePaths(std::move(resourcePaths));
    updateLoadState(true);
}

void ResourcesHierarchyViewModel::requestViewModelHook()
{
    if (m_resourcesFilePath.trimmed().isEmpty())
    {
        emit viewModelHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromResourcesFilePath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit viewModelHookRequested();
        return;
    }

    updateLoadState(true);
    emit viewModelHookRequested();
}

bool ResourcesHierarchyViewModel::reloadFromResourcesFilePath(QString* errorMessage)
{
    const QString normalizedFilePath = m_resourcesFilePath.trimmed();
    if (normalizedFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    QStringList refreshedResourcePaths;
    if (QFileInfo(normalizedFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ResourcesSupport::readUtf8File(normalizedFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonResourcesHierarchyStore refreshedStore;
        WhatSonResourcesHierarchyParser parser;
        QString parseError;
        if (!parser.parse(rawText, &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        refreshedResourcePaths = refreshedStore.resourcePaths();
    }

    const QString resolvedWshubPath = resolveWshubPathFromResourcesFile(normalizedFilePath);
    if (refreshedResourcePaths.isEmpty())
    {
        refreshedResourcePaths = WhatSon::Resources::listRelativeResourcePackagePathsForHub(resolvedWshubPath);
    }

    QStringList resolutionBasePaths = WhatSon::Resources::resourceReferenceBasePathsForResourcesFile(normalizedFilePath);
    if (!resolvedWshubPath.trimmed().isEmpty())
    {
        resolutionBasePaths.push_back(WhatSon::Resources::normalizePath(resolvedWshubPath));
    }
    setResourceResolutionBasePaths(std::move(resolutionBasePaths));
    setResourcePaths(std::move(refreshedResourcePaths));
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
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
    emit hierarchyModelChanged();
}

void ResourcesHierarchyViewModel::syncDomainStoreFromItems()
{
    m_resourcePaths = WhatSon::Hierarchy::ResourcesSupport::extractResourcePathsFromItems(m_items);
    m_store.setResourcePaths(m_resourcePaths);
}

void ResourcesHierarchyViewModel::refreshNoteListForSelection()
{
    const ResourcesHierarchyItem* selectedItem = nullptr;
    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
    {
        selectedItem = &m_items.at(m_selectedIndex);
    }

    if (selectedItem == nullptr)
    {
        m_noteListModel.setItems({});
        return;
    }

    QVector<ResourcesListItem> listItems = buildResourceNoteListItems(
        m_resourcePaths,
        m_resourceResolutionBasePaths,
        selectedItem);
    m_noteListModel.setItems(std::move(listItems));
}

void ResourcesHierarchyViewModel::setResourceResolutionBasePaths(QStringList basePaths)
{
    basePaths = WhatSon::Hierarchy::ResourcesSupport::sanitizeStringList(std::move(basePaths));
    if (m_resourceResolutionBasePaths == basePaths)
    {
        return;
    }
    m_resourceResolutionBasePaths = std::move(basePaths);
    refreshNoteListForSelection();
}
