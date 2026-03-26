#pragma once

#include "file/note/WhatSonNoteHeaderStore.hpp"

#include <QObject>
#include <QHash>
#include <QString>

class WhatSonNoteHeaderSessionStore final : public QObject
{
    Q_OBJECT

public:
    struct Entry final
    {
        QString noteId;
        QString noteDirectoryPath;
        QString headerFilePath;
        WhatSonNoteHeaderStore header;
        bool loaded = false;
        bool dirty = false;
    };

    explicit WhatSonNoteHeaderSessionStore(QObject* parent = nullptr);

    bool ensureLoaded(const QString& noteId, const QString& noteDirectoryPath, QString* errorMessage = nullptr);
    bool hasEntry(const QString& noteId) const;
    Entry entry(const QString& noteId) const;

    QString noteDirectoryPath(const QString& noteId) const;
    QString headerFilePath(const QString& noteId) const;
    WhatSonNoteHeaderStore header(const QString& noteId) const;

    bool updateProject(const QString& noteId, const QString& project, QString* errorMessage = nullptr);
    bool updateBookmarked(const QString& noteId, bool bookmarked, QStringList bookmarkColors, QString* errorMessage = nullptr);
    bool updateProgress(const QString& noteId, int progress, QString* errorMessage = nullptr);

signals:
    void entryChanged(const QString& noteId);

private:
    static QString resolveHeaderFilePath(const QString& noteDirectoryPath);
    Entry* findEntry(const QString& noteId);
    const Entry* findEntry(const QString& noteId) const;
    bool persistEntry(Entry& entry, QString* errorMessage = nullptr);

    QHash<QString, Entry> m_entries;
};
