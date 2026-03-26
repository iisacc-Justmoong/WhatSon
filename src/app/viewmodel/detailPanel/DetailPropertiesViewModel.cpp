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

void DetailPropertiesViewModel::applyHeader(const WhatSonNoteHeaderStore& header)
{
    m_folderItems = header.folders();
    m_tagItems = header.tags();
    emit metadataChanged();
}

void DetailPropertiesViewModel::clearHeader()
{
    m_folderItems.clear();
    m_tagItems.clear();
    emit metadataChanged();
}
