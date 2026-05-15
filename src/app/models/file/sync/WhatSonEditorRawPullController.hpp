#pragma once

#include <QObject>
#include <QString>
#include <QtTypes>

#include <functional>

class WhatSonEditorRawPullController final : public QObject
{
    Q_OBJECT

public:
    using RawPullCallback = std::function<quint64(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& reason,
        QString* errorMessage)>;

    explicit WhatSonEditorRawPullController(QObject* parent = nullptr);

    void setRawPullCallback(RawPullCallback callback);

    Q_INVOKABLE quint64 requestNoteEntryPull(
        const QString& noteId,
        const QString& noteDirectoryPath);
    Q_INVOKABLE quint64 requestNoteOpenPull(
        const QString& noteId,
        const QString& noteDirectoryPath);

signals:
    void rawPullRequested(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& reason);
    void rawPullFinished(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& reason,
        quint64 sequence,
        bool success,
        const QString& errorMessage);

private:
    static QString normalizedNoteId(const QString& noteId);
    static QString normalizedPath(const QString& path);

    quint64 executePull(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& reason);

    RawPullCallback m_rawPullCallback;
};
