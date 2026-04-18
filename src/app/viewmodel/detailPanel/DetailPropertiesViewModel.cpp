#include "DetailPropertiesViewModel.hpp"

#include "file/note/WhatSonNoteFolderSemantics.hpp"

namespace
{
    int normalizeIndex(const int requestedIndex, const int itemCount) noexcept
    {
        if (itemCount <= 0)
        {
            return -1;
        }
        if (requestedIndex < 0)
        {
            return -1;
        }
        if (requestedIndex >= itemCount)
        {
            return itemCount - 1;
        }
        return requestedIndex;
    }

    QStringList displayFolderItems(const QStringList& folderPaths)
    {
        QStringList items;
        items.reserve(folderPaths.size());

        for (const QString& folderPath : folderPaths)
        {
            const QString leafName = WhatSon::NoteFolders::leafFolderName(folderPath);
            if (!leafName.isEmpty())
            {
                items.push_back(leafName);
                continue;
            }

            items.push_back(WhatSon::NoteFolders::displayFolderPath(folderPath));
        }

        return items;
    }
}

DetailPropertiesViewModel::DetailPropertiesViewModel(QObject* parent)
    : DetailContentSectionViewModel(DetailContentState::Properties, parent)
{
}

QStringList DetailPropertiesViewModel::folderItems() const
{
    return m_folderItems;
}

QStringList DetailPropertiesViewModel::tagItems() const
{
    return m_tagItems;
}

int DetailPropertiesViewModel::activeFolderIndex() const noexcept
{
    return m_activeFolderIndex;
}

int DetailPropertiesViewModel::activeTagIndex() const noexcept
{
    return m_activeTagIndex;
}

void DetailPropertiesViewModel::setActiveFolderIndex(int index)
{
    const int normalizedIndex = normalizeMetadataIndex(index, m_folderItems.size());
    if (m_activeFolderIndex == normalizedIndex)
    {
        return;
    }

    m_activeFolderIndex = normalizedIndex;
    emit metadataChanged();
}

void DetailPropertiesViewModel::setActiveTagIndex(int index)
{
    const int normalizedIndex = normalizeMetadataIndex(index, m_tagItems.size());
    if (m_activeTagIndex == normalizedIndex)
    {
        return;
    }

    m_activeTagIndex = normalizedIndex;
    emit metadataChanged();
}

QString DetailPropertiesViewModel::currentProject() const
{
    return m_currentProject;
}

QString DetailPropertiesViewModel::currentBookmark() const
{
    return m_currentBookmark;
}

int DetailPropertiesViewModel::currentProgress() const noexcept
{
    return m_currentProgress;
}

void DetailPropertiesViewModel::applyHeader(const WhatSonNoteHeaderStore& header)
{
    m_folderItems = displayFolderItems(header.folders());
    m_tagItems = header.tags();
    m_activeFolderIndex = normalizeMetadataIndex(m_activeFolderIndex, m_folderItems.size());
    m_activeTagIndex = normalizeMetadataIndex(m_activeTagIndex, m_tagItems.size());
    m_currentProject = header.project().trimmed();
    m_currentBookmark = header.bookmarkColors().isEmpty() ? QString() : header.bookmarkColors().first().trimmed();
    m_currentProgress = header.progress();
    emit metadataChanged();
}

void DetailPropertiesViewModel::clearHeader()
{
    m_folderItems.clear();
    m_tagItems.clear();
    m_activeFolderIndex = -1;
    m_activeTagIndex = -1;
    m_currentProject.clear();
    m_currentBookmark.clear();
    m_currentProgress = -1;
    emit metadataChanged();
}

int DetailPropertiesViewModel::normalizeMetadataIndex(const int requestedIndex, const int itemCount) noexcept
{
    return normalizeIndex(requestedIndex, itemCount);
}
