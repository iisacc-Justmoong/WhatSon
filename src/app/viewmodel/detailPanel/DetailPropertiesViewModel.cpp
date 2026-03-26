#include "DetailPropertiesViewModel.hpp"

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
    m_folderItems = header.folders();
    m_tagItems = header.tags();
    m_currentProject = header.project().trimmed();
    m_currentBookmark = header.bookmarkColors().isEmpty() ? QString() : header.bookmarkColors().first().trimmed();
    m_currentProgress = header.progress();
    emit metadataChanged();
}

void DetailPropertiesViewModel::clearHeader()
{
    m_folderItems.clear();
    m_tagItems.clear();
    m_currentProject.clear();
    m_currentBookmark.clear();
    m_currentProgress = 0;
    emit metadataChanged();
}
