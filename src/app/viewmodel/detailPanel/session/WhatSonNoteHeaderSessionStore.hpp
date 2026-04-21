#pragma once

#include "app/viewmodel/detailPanel/session/IWhatSonNoteHeaderSessionStore.hpp"

#include <QHash>

class WhatSonNoteHeaderSessionStore final : public IWhatSonNoteHeaderSessionStore
{
    Q_OBJECT

public:
    explicit WhatSonNoteHeaderSessionStore(QObject* parent = nullptr);

    bool ensureLoaded(
        const QString& noteId,
        const QString& noteDirectoryPath,
        QString* errorMessage = nullptr,
        bool forceReload = false) override;
    bool hasEntry(const QString& noteId) const override;
    Entry entry(const QString& noteId) const override;

    QString noteDirectoryPath(const QString& noteId) const override;
    QString headerFilePath(const QString& noteId) const override;
    WhatSonNoteHeaderStore header(const QString& noteId) const override;

    bool updateProject(const QString& noteId, const QString& project, QString* errorMessage = nullptr) override;
    bool updateBookmarked(
        const QString& noteId,
        bool bookmarked,
        QStringList bookmarkColors,
        QString* errorMessage = nullptr) override;
    bool updateProgress(const QString& noteId, int progress, QString* errorMessage = nullptr) override;
    bool assignFolderBinding(
        const QString& noteId,
        const QString& folderPath,
        const QString& folderUuid,
        QString* errorMessage = nullptr) override;
    bool assignTag(
        const QString& noteId,
        const QString& tag,
        QString* errorMessage = nullptr) override;
    bool removeFolderAt(const QString& noteId, int index, QString* errorMessage = nullptr) override;
    bool removeTagAt(const QString& noteId, int index, QString* errorMessage = nullptr) override;

private:
    static QString resolveHeaderFilePath(const QString& noteDirectoryPath);
    Entry* findEntry(const QString& noteId);
    const Entry* findEntry(const QString& noteId) const;
    bool persistEntry(Entry& entry, QString* errorMessage = nullptr);

    QHash<QString, Entry> m_entries;
};
