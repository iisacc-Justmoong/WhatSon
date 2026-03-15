#include "ProgressHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <utility>

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
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void ProgressHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant count=%1").arg(depthItems.size()));
    Q_UNUSED(depthItems);
}

QVariantList ProgressHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList ProgressHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::ProgressSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), QStringLiteral("progress:%1").arg(index));
        serialized[index] = entry;
    }
    return serialized;
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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant index=%1 label=%2").arg(index).arg(
                                  displayName));
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    return false;
}

void ProgressHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant"));
    return;
}

void ProgressHierarchyViewModel::deleteSelectedFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant"));
    return;
}

void ProgressHierarchyViewModel::setProgressState(int progressValue, QStringList progressStates)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProgressState.begin"),
                              QStringLiteral("value=%1 rawCount=%2").arg(progressValue).arg(progressStates.size()));
    m_progressValue = progressValue;
    m_progressStates = WhatSon::Hierarchy::ProgressSupport::sanitizeStringList(std::move(progressStates));
    rebuildItems();
    syncProgressStore();
    m_createdFolderSequence = WhatSon::Hierarchy::ProgressSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProgressState.success"),
                              QStringLiteral("value=%1 states=%2 itemCount=%3").arg(m_progressValue).arg(
                                  m_progressStates.size()).arg(m_items.size()));
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
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
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
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
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
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.defaultParse"),
                                      QStringLiteral("path=%1 reason=%2").arg(wshubPath, parseError));
            updateLoadState(false, parseError);
            return false;
        }
    }

    setProgressState(m_store.progressValue(), m_store.progressStates());

    WhatSon::Debug::traceSelf(this,
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

void ProgressHierarchyViewModel::applyRuntimeSnapshot(
    int progressValue,
    QStringList progressStates,
    QString progressFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_progressFilePath = progressFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    setProgressState(progressValue, std::move(progressStates));
    updateLoadState(true);
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
    emit hierarchyModelChanged();
}
