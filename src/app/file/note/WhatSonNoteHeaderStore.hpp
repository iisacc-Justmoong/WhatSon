#pragma once

#include <QString>
#include <QStringList>

class WhatSonNoteHeaderStore
{
public:
    WhatSonNoteHeaderStore();
    ~WhatSonNoteHeaderStore();

    void clear();

    QString noteId() const;
    void setNoteId(QString noteId);

    QString createdAt() const;
    void setCreatedAt(QString createdAt);

    QString author() const;
    void setAuthor(QString author);

    QString lastModifiedAt() const;
    void setLastModifiedAt(QString lastModifiedAt);

    QString modifiedBy() const;
    void setModifiedBy(QString modifiedBy);

    QStringList folders() const;
    void setFolders(QStringList folders);
    QStringList folderUuids() const;
    void setFolderUuids(QStringList folderUuids);
    void setFolderBindings(QStringList folders, QStringList folderUuids);

    QString project() const;
    void setProject(QString project);

    bool isBookmarked() const noexcept;
    void setBookmarked(bool bookmarked) noexcept;
    QStringList bookmarkColors() const;
    void setBookmarkColors(QStringList colors);

    QStringList tags() const;
    void setTags(QStringList tags);

    QStringList progressEnums() const;
    void setProgressEnums(QStringList progressEnums);

    int progress() const noexcept;
    void setProgress(int progress) noexcept;

    bool isPreset() const noexcept;
    void setPreset(bool preset) noexcept;

private:
    QString m_noteId;
    QString m_createdAt;
    QString m_author;
    QString m_lastModifiedAt;
    QString m_modifiedBy;
    QStringList m_folders;
    QStringList m_folderUuids;
    QString m_project;
    bool m_bookmarked = false;
    QStringList m_bookmarkColors;
    QStringList m_tags;
    QStringList m_progressEnums;
    int m_progress = -1;
    bool m_preset = false;
};
