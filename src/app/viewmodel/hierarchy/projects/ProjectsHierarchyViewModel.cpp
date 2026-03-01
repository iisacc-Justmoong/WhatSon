#include "ProjectsHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

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
    setProjectNames({});
}

ProjectsHierarchyViewModel::~ProjectsHierarchyViewModel() = default;

FlatHierarchyModel* ProjectsHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int ProjectsHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void ProjectsHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
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
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Project"));
    syncModel();
    setSelectedIndex(-1);
}

QVariantList ProjectsHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

void ProjectsHierarchyViewModel::setProjectNames(QStringList projectNames)
{
    m_projectNames = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(projectNames));
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Projects"),
        m_projectNames,
        QStringLiteral("Project"));
    syncModel();
    setSelectedIndex(-1);
}

QStringList ProjectsHierarchyViewModel::projectNames() const
{
    return m_projectNames;
}

bool ProjectsHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
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
        if (!WhatSon::Hierarchy::Support::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonProjectsHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : store.projectNames())
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

    return true;
}

void ProjectsHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}
