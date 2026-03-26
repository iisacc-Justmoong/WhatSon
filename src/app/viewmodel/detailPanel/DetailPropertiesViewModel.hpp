#pragma once

#include "DetailContentSectionViewModel.hpp"
#include "file/note/WhatSonNoteHeaderStore.hpp"

#include <QStringList>

class DetailPropertiesViewModel final : public DetailContentSectionViewModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList folderItems READ folderItems NOTIFY metadataChanged)
    Q_PROPERTY(QStringList tagItems READ tagItems NOTIFY metadataChanged)
    Q_PROPERTY(int activeFolderIndex READ activeFolderIndex WRITE setActiveFolderIndex NOTIFY metadataChanged)
    Q_PROPERTY(int activeTagIndex READ activeTagIndex WRITE setActiveTagIndex NOTIFY metadataChanged)
    Q_PROPERTY(QString currentProject READ currentProject NOTIFY metadataChanged)
    Q_PROPERTY(QString currentBookmark READ currentBookmark NOTIFY metadataChanged)
    Q_PROPERTY(int currentProgress READ currentProgress NOTIFY metadataChanged)

public:
    explicit DetailPropertiesViewModel(QObject* parent = nullptr);

    QStringList folderItems() const;
    QStringList tagItems() const;
    int activeFolderIndex() const noexcept;
    int activeTagIndex() const noexcept;
    Q_INVOKABLE void setActiveFolderIndex(int index);
    Q_INVOKABLE void setActiveTagIndex(int index);
    QString currentProject() const;
    QString currentBookmark() const;
    int currentProgress() const noexcept;
    void applyHeader(const WhatSonNoteHeaderStore& header);
    void clearHeader();

signals:
    void metadataChanged();

private:
    static int normalizeMetadataIndex(int requestedIndex, int itemCount) noexcept;

    QStringList m_folderItems;
    QStringList m_tagItems;
    int m_activeFolderIndex = -1;
    int m_activeTagIndex = -1;
    QString m_currentProject;
    QString m_currentBookmark;
    int m_currentProgress = 0;
};
