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

    QString title() const;
    void setTitle(QString title);

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

    QString project() const;
    void setProject(QString project);

    bool isBookmarked() const noexcept;
    void setBookmarked(bool bookmarked) noexcept;
    QStringList bookmarkColors() const;
    void setBookmarkColors(QStringList colors);

    QStringList tags() const;
    void setTags(QStringList tags);

    int progress() const noexcept;
    void setProgress(int progress) noexcept;

    bool isPreset() const noexcept;
    void setPreset(bool preset) noexcept;

private:
    QString m_noteId;
    QString m_title;
    QString m_createdAt;
    QString m_author;
    QString m_lastModifiedAt;
    QString m_modifiedBy;
    QStringList m_folders;
    QString m_project;
    bool m_bookmarked = false;
    QStringList m_bookmarkColors;
    QStringList m_tags;
    int m_progress = 0;
    bool m_preset = false;
};
