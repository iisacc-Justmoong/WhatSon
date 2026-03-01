#include "ProgressHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"

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
    QObject::connect(
        &m_itemModel,
        &ProgressHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setProgressState(0, {
                         QStringLiteral("Ready"),
                         QStringLiteral("Pending"),
                         QStringLiteral("InProgress"),
                         QStringLiteral("Done")
                     });
}

ProgressHierarchyViewModel::~ProgressHierarchyViewModel() = default;

ProgressHierarchyModel* ProgressHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int ProgressHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int ProgressHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool ProgressHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString ProgressHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void ProgressHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::ProgressSupport::clampSelectionIndex(index, m_itemModel.rowCount());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void ProgressHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    Q_UNUSED(depthItems);
}

QVariantList ProgressHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::ProgressSupport::serializeDepthItems(m_items);
}

QString ProgressHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ProgressHierarchyViewModel::canRenameItem(int index) const
{
    Q_UNUSED(index);
    return false;
}

bool ProgressHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    return false;
}

void ProgressHierarchyViewModel::createFolder()
{
    return;
}

void ProgressHierarchyViewModel::deleteSelectedFolder()
{
    return;
}

void ProgressHierarchyViewModel::setProgressState(int progressValue, QStringList progressStates)
{
    m_progressValue = progressValue;
    m_progressStates = WhatSon::Hierarchy::ProgressSupport::sanitizeStringList(std::move(progressStates));
    rebuildItems();
    syncProgressStore();
    m_createdFolderSequence = WhatSon::Hierarchy::ProgressSupport::nextGeneratedFolderSequence(m_items);
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
    return false;
}

bool ProgressHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool ProgressHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool ProgressHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_progressFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProgressSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
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
        m_progressFilePath = filePath;

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProgressSupport::readUtf8File(filePath, &rawText, &readError))
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
        break;
    }

    if (m_progressFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_progressFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Progress.wsprogress"));
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
            updateLoadState(false, parseError);
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

    updateLoadState(true);
    return true;
}

void ProgressHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void ProgressHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void ProgressHierarchyViewModel::rebuildItems()
{
    QVector<ProgressHierarchyItem> rebuilt;
    rebuilt.reserve(m_progressStates.size() + 1);

    ProgressHierarchyItem bucket;
    bucket.depth = 0;
    bucket.accent = true;
    bucket.expanded = true;
    bucket.label = progressHeaderLabel(m_progressValue, m_progressStates);
    bucket.showChevron = !m_progressStates.isEmpty();
    rebuilt.push_back(std::move(bucket));

    for (int index = 0; index < m_progressStates.size(); ++index)
    {
        ProgressHierarchyItem state;
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
    updateItemCount();
}
