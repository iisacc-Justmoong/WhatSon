#include "ProgressHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

#include <algorithm>

namespace
{
    constexpr auto kScope = "progress.viewmodel";

    QString progressHeaderLabel(int progressValue, const QStringList& progressStates)
    {
        if (progressValue >= 0 && progressValue < progressStates.size())
        {
            return QStringLiteral("Progress (%1)").arg(progressStates.at(progressValue));
        }

        return QStringLiteral("Progress (%1)").arg(progressValue);
    }
} // namespace

ProgressHierarchyViewModel::ProgressHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    setProgressState(0, {
                         QStringLiteral("Ready"),
                         QStringLiteral("Pending"),
                         QStringLiteral("InProgress"),
                         QStringLiteral("Done")
                     });
}

ProgressHierarchyViewModel::~ProgressHierarchyViewModel() = default;

FlatHierarchyModel* ProgressHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int ProgressHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void ProgressHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void ProgressHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Progress"));
    syncProgressStatesFromItems();
    syncProgressStore();
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

QVariantList ProgressHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString ProgressHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ProgressHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled() || index <= 0 || index >= m_items.size())
    {
        return false;
    }

    const QString trimmed = displayName.trimmed();
    if (trimmed.isEmpty())
    {
        return false;
    }

    const int stateIndex = index - 1;
    if (stateIndex < 0 || stateIndex >= m_progressStates.size())
    {
        return false;
    }

    m_progressStates[stateIndex] = trimmed;
    rebuildItems();
    syncProgressStore();
    syncModel();
    setSelectedIndex(index);
    return true;
}

void ProgressHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    m_progressStates.push_back(QStringLiteral("Folder%1").arg(m_createdFolderSequence++));
    rebuildItems();
    syncProgressStore();
    syncModel();
    setSelectedIndex(m_items.size() - 1);
}

void ProgressHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int stateIndex = m_selectedIndex - 1;
    if (stateIndex < 0 || stateIndex >= m_progressStates.size())
    {
        return;
    }

    m_progressStates.removeAt(stateIndex);
    if (m_progressValue >= m_progressStates.size())
    {
        m_progressValue = std::max(0, static_cast<int>(m_progressStates.size()) - 1);
    }
    rebuildItems();
    syncProgressStore();
    syncModel();
    setSelectedIndex(std::min(m_selectedIndex, static_cast<int>(m_items.size()) - 1));
}

void ProgressHierarchyViewModel::setProgressState(int progressValue, QStringList progressStates)
{
    m_progressValue = progressValue;
    m_progressStates = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(progressStates));
    rebuildItems();
    syncProgressStore();
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

int ProgressHierarchyViewModel::progressValue() const noexcept
{
    return m_progressValue;
}

QStringList ProgressHierarchyViewModel::progressStates() const
{
    return m_progressStates;
}

bool ProgressHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool ProgressHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool ProgressHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return m_selectedIndex > 0;
}

bool ProgressHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
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

    WhatSonProgressHierarchyParser parser;

    bool fileFound = false;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Progress.wsprogress"));
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

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }
        break;
    }

    if (!fileFound)
    {
        QString parseError;
        if (!parser.parse(QString(), &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }
    }

    setProgressState(m_store.progressValue(), m_store.progressStates());

    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("loadFromWshub"),
        QStringLiteral("path=%1 fileFound=%2 value=%3 states=%4")
        .arg(wshubPath)
        .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_progressValue)
        .arg(m_progressStates.size()));

    return true;
}

void ProgressHierarchyViewModel::rebuildItems()
{
    QVector<FlatHierarchyItem> rebuilt;
    rebuilt.reserve(m_progressStates.size() + 1);

    FlatHierarchyItem bucket;
    bucket.depth = 0;
    bucket.accent = true;
    bucket.expanded = true;
    bucket.label = progressHeaderLabel(m_progressValue, m_progressStates);
    bucket.showChevron = !m_progressStates.isEmpty();
    rebuilt.push_back(std::move(bucket));

    for (int index = 0; index < m_progressStates.size(); ++index)
    {
        FlatHierarchyItem state;
        state.depth = 1;
        state.accent = index == m_progressValue;
        state.expanded = false;
        state.label = m_progressStates.at(index);
        state.showChevron = false;
        rebuilt.push_back(std::move(state));
    }

    m_items = std::move(rebuilt);
}

void ProgressHierarchyViewModel::syncProgressStore()
{
    m_store.setProgressValue(m_progressValue);
    m_store.setProgressStates(m_progressStates);
}

void ProgressHierarchyViewModel::syncProgressStatesFromItems()
{
    m_progressStates.clear();
    for (int index = 1; index < m_items.size(); ++index)
    {
        const QString label = m_items.at(index).label.trimmed();
        if (label.isEmpty() || m_progressStates.contains(label))
        {
            continue;
        }
        m_progressStates.push_back(label);
    }
    if (m_progressValue >= m_progressStates.size())
    {
        m_progressValue = std::max(0, static_cast<int>(m_progressStates.size()) - 1);
    }
}

void ProgressHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}
