#pragma once

#include "file/note/WhatSonNoteHeaderStore.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

class IWhatSonNoteHeaderSessionStore : public QObject
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

    explicit IWhatSonNoteHeaderSessionStore(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IWhatSonNoteHeaderSessionStore() override = default;

    virtual bool ensureLoaded(
        const QString& noteId,
        const QString& noteDirectoryPath,
        QString* errorMessage = nullptr,
        bool forceReload = false) = 0;
    virtual bool hasEntry(const QString& noteId) const = 0;
    virtual Entry entry(const QString& noteId) const = 0;

    virtual QString noteDirectoryPath(const QString& noteId) const = 0;
    virtual QString headerFilePath(const QString& noteId) const = 0;
    virtual WhatSonNoteHeaderStore header(const QString& noteId) const = 0;

    virtual bool updateProject(const QString& noteId, const QString& project, QString* errorMessage = nullptr) = 0;
    virtual bool updateBookmarked(
        const QString& noteId,
        bool bookmarked,
        QStringList bookmarkColors,
        QString* errorMessage = nullptr) = 0;
    virtual bool updateProgress(const QString& noteId, int progress, QString* errorMessage = nullptr) = 0;
    virtual bool assignFolderBinding(
        const QString& noteId,
        const QString& folderPath,
        const QString& folderUuid,
        QString* errorMessage = nullptr) = 0;
    virtual bool removeFolderAt(const QString& noteId, int index, QString* errorMessage = nullptr) = 0;
    virtual bool removeTagAt(const QString& noteId, int index, QString* errorMessage = nullptr) = 0;

signals:
    void entryChanged(const QString& noteId);
};
