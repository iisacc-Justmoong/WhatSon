#include "ProjectsHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModelSupport.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

namespace
{
    constexpr auto kScope = "projects.viewmodel";

    QVector<WhatSonFolderDepthEntry> folderEntriesFromItems(const QVector<ProjectsHierarchyItem>& items)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(items.size());
        const int depthOffset =
            (!items.isEmpty() && items.first().accent && items.first().depth == 0) ? 1 : 0;
        QStringList pathStack;

        for (const ProjectsHierarchyItem& item : items)
        {
            const QString label = item.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }
            if (item.accent && item.depth == 0)
            {
                continue;
            }

            int depth = std::max(0, item.depth - depthOffset);
            if (depth > pathStack.size())
            {
                depth = pathStack.size();
            }
            while (pathStack.size() > depth)
            {
                pathStack.removeLast();
            }

            const QString parentPath = (depth > 0 && !pathStack.isEmpty()) ? pathStack.constLast() : QString();
            const QString id = parentPath.isEmpty() ? label : parentPath + QLatin1Char('/') + label;

            WhatSonFolderDepthEntry entry;
            entry.id = id;
            entry.label = label;
            entry.depth = depth;
            entries.push_back(std::move(entry));

            if (pathStack.size() <= depth)
            {
                pathStack.push_back(id);
            }
            else
            {
                pathStack[depth] = id;
                pathStack = pathStack.mid(0, depth + 1);
            }
        }

        return entries;
    }

    QVector<ProjectsHierarchyItem> itemsFromFolderEntries(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        QVector<ProjectsHierarchyItem> items;
        items.reserve(entries.size());

        for (const WhatSonFolderDepthEntry& entry : entries)
        {
            const QString label = entry.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }

            ProjectsHierarchyItem item;
            item.depth = std::max(0, entry.depth);
            item.label = label;
            item.accent = false;
            item.expanded = false;
            item.showChevron = true;
            items.push_back(std::move(item));
        }

        WhatSon::Hierarchy::ProjectsSupport::applyChevronByDepth(&items);
        return items;
    }
}

ProjectsHierarchyViewModel::ProjectsHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &ProjectsHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setProjectNames({});
}

ProjectsHierarchyViewModel::~ProjectsHierarchyViewModel() = default;

ProjectsHierarchyModel* ProjectsHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int ProjectsHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int ProjectsHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool ProjectsHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString ProjectsHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void ProjectsHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::ProjectsSupport::clampSelectionIndex(index, m_itemModel.rowCount());
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

void ProjectsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = WhatSon::Hierarchy::ProjectsSupport::parseDepthItems(depthItems, QStringLiteral("Project"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList ProjectsHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::ProjectsSupport::serializeDepthItems(m_items);
}

QString ProjectsHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ProjectsHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const ProjectsHierarchyItem& item = m_items.at(index);
    return !(item.accent && item.depth == 0);
}

bool ProjectsHierarchyViewModel::renameItem(int index, const QString& displayName)
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

    QVector<ProjectsHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::ProjectsSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=support rejected index=%1 label=%2").arg(index).arg(
                                      displayName));
        return false;
    }

    WhatSonProjectsHierarchyStore stagedStore = m_store;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));

    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_foldersFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_projectNames = m_store.projectNames();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(displayName).arg(
                                  m_items.size()));
    return true;
}

void ProjectsHierarchyViewModel::createFolder()
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

    const int insertIndex = WhatSon::Hierarchy::ProjectsSupport::createHierarchyFolder(
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

void ProjectsHierarchyViewModel::deleteSelectedFolder()
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

    const int nextSelectedIndex =
        WhatSon::Hierarchy::ProjectsSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 nextIndex=%2 itemCount=%3").arg(startIndex).arg(
                                  nextSelectedIndex).arg(m_items.size()));
}

void ProjectsHierarchyViewModel::setProjectNames(QStringList projectNames)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProjectNames.begin"),
                              QStringLiteral("rawCount=%1").arg(projectNames.size()));
    m_projectNames = WhatSon::Hierarchy::ProjectsSupport::sanitizeStringList(std::move(projectNames));
    m_store.setProjectNames(m_projectNames);
    m_items = WhatSon::Hierarchy::ProjectsSupport::buildBucketItems(
        QStringLiteral("Projects"),
        m_projectNames,
        QStringLiteral("Project"));
    m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProjectNames.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_projectNames.size()).arg(
                                  m_items.size()));
}

QStringList ProjectsHierarchyViewModel::projectNames() const
{
    return m_projectNames;
}

bool ProjectsHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool ProjectsHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool ProjectsHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    const ProjectsHierarchyItem& selectedItem = m_items.at(m_selectedIndex);
    return !(selectedItem.accent && selectedItem.depth == 0);
}

bool ProjectsHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_foldersFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProjectsSupport::resolveContentsDirectories(
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

    QVector<WhatSonFolderDepthEntry> aggregatedEntries;
    bool fileFound = false;

    WhatSonProjectsHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Folders.wsfolders"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        fileFound = true;
        if (m_foldersFilePath.isEmpty())
        {
            m_foldersFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProjectsSupport::readUtf8File(filePath, &rawText, &readError))
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
        WhatSonProjectsHierarchyStore parsedStore;
        if (!parser.parse(rawText, &parsedStore, &parseError))
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

        const QVector<WhatSonFolderDepthEntry> parsedEntries = parsedStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            aggregatedEntries.push_back(entry);
        }
    }

    if (m_foldersFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_foldersFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Folders.wsfolders"));
    }

    if (aggregatedEntries.isEmpty())
    {
        setProjectNames({});
    }
    else
    {
        m_store.setFolderEntries(std::move(aggregatedEntries));
        m_projectNames = m_store.projectNames();
        m_items = itemsFromFolderEntries(m_store.folderEntries());
        m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
    }

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3 entryCount=%4")
                              .arg(wshubPath)
                              .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_projectNames.size())
                              .arg(m_store.folderEntries().size()));

    if (WhatSon::Debug::isEnabled())
    {
        qWarning().noquote()
            << QStringLiteral("[projects:index] path=%1 count=%2 values=[%3]")
               .arg(wshubPath)
               .arg(m_projectNames.size())
               .arg(m_projectNames.join(QStringLiteral(", ")));
    }

    updateLoadState(true);
    return true;
}

void ProjectsHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void ProjectsHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void ProjectsHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
}

void ProjectsHierarchyViewModel::syncDomainStoreFromItems()
{
    m_store.setFolderEntries(folderEntriesFromItems(m_items));
    m_projectNames = m_store.projectNames();
}
