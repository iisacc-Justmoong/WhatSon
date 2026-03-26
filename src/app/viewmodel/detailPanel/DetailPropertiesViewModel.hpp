#pragma once

#include "DetailContentSectionViewModel.hpp"
#include "file/note/WhatSonNoteHeaderStore.hpp"

#include <QStringList>

class DetailPropertiesViewModel final : public DetailContentSectionViewModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList folderItems READ folderItems NOTIFY metadataChanged)
    Q_PROPERTY(QStringList tagItems READ tagItems NOTIFY metadataChanged)
    Q_PROPERTY(QString currentProject READ currentProject NOTIFY metadataChanged)
    Q_PROPERTY(QString currentBookmark READ currentBookmark NOTIFY metadataChanged)
    Q_PROPERTY(int currentProgress READ currentProgress NOTIFY metadataChanged)

public:
    explicit DetailPropertiesViewModel(QObject* parent = nullptr);

    QStringList folderItems() const;
    QStringList tagItems() const;
    QString currentProject() const;
    QString currentBookmark() const;
    int currentProgress() const noexcept;
    void applyHeader(const WhatSonNoteHeaderStore& header);
    void clearHeader();

signals:
    void metadataChanged();

private:
    QStringList m_folderItems;
    QStringList m_tagItems;
    QString m_currentProject;
    QString m_currentBookmark;
    int m_currentProgress = 0;
};
