#pragma once

#include "DetailContentSectionViewModel.hpp"
#include "file/note/WhatSonNoteHeaderStore.hpp"

#include <QStringList>

class DetailPropertiesViewModel final : public DetailContentSectionViewModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList folderItems READ folderItems NOTIFY metadataChanged)
    Q_PROPERTY(QStringList tagItems READ tagItems NOTIFY metadataChanged)

public:
    explicit DetailPropertiesViewModel(QObject* parent = nullptr);

    QStringList folderItems() const;
    QStringList tagItems() const;
    void applyHeader(const WhatSonNoteHeaderStore& header);
    void clearHeader();

signals:
    void metadataChanged();

private:
    QStringList m_folderItems;
    QStringList m_tagItems;
};
