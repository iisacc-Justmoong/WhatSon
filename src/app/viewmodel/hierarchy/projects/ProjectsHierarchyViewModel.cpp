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
}

ProjectsHierarchyViewModel::ProjectsHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::trace(QString::fromLatin1(kScope), QStringLiteral("ctor"));
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
    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void ProjectsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("setDepthItems.begin"),
        QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = WhatSon::Hierarchy::ProjectsSupport::parseDepthItems(depthItems, QStringLiteral("Project"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
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

bool ProjectsHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled())
    {
        return false;
    }
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }
    if (m_items.at(index).accent && m_items.at(index).depth == 0)
    {
        return false;
    }

    if (!WhatSon::Hierarchy::ProjectsSupport::renameHierarchyItem(&m_items, index, displayName))
    {
        return false;
    }

    syncDomainStoreFromItems();
    syncModel();
    return true;
}

void ProjectsHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::ProjectsSupport::createHierarchyFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
}

void ProjectsHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int nextSelectedIndex =
        WhatSon::Hierarchy::ProjectsSupport::deleteHierarchySubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void ProjectsHierarchyViewModel::setProjectNames(QStringList projectNames)
{
    m_projectNames = WhatSon::Hierarchy::ProjectsSupport::sanitizeStringList(std::move(projectNames));
    m_store.setProjectNames(m_projectNames);
    m_items = WhatSon::Hierarchy::ProjectsSupport::buildBucketItems(
        QStringLiteral("Projects"),
        m_projectNames,
        QStringLiteral("Project"));
    m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
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
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProjectsSupport::resolveContentsDirectories(
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

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProjectsSupport::readUtf8File(filePath, &rawText, &readError))
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

        for (const QString& value : m_store.projectNames())
        {
            aggregated.push_back(value);
        }
    }

    setProjectNames(aggregated);

    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("loadFromWshub"),
        QStringLiteral("path=%1 fileFound=%2 count=%3")
        .arg(wshubPath)
        .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_projectNames.size()));

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
    m_projectNames = WhatSon::Hierarchy::ProjectsSupport::extractDomainLabelsFromItems(m_items);
    m_store.setProjectNames(m_projectNames);
}
