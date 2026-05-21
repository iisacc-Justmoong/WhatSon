#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>

class WhatSonEditorRawPushController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int idleIntervalMs READ idleIntervalMs WRITE setIdleIntervalMs NOTIFY idleIntervalMsChanged)

public:
    using RawPushCallback = std::function<bool(
        const QString& editorFilePath,
        const QString& editorDocumentText,
        bool hasEditorDocumentText,
        const QString& reason,
        QString* errorMessage)>;

    explicit WhatSonEditorRawPushController(QObject* parent = nullptr);

    int idleIntervalMs() const noexcept;
    void setRawPushCallback(RawPushCallback callback);

    Q_INVOKABLE void requestIdlePush(
        const QString& editorFilePath,
        const QString& editorDocumentText);
    Q_INVOKABLE void requestModifiedCountPush(
        const QString& editorFilePath,
        int modifiedCount,
        const QString& editorDocumentText);
    Q_INVOKABLE bool pushBeforeNoteDeparture(const QString& editorFilePath);
    Q_INVOKABLE bool flushPendingPush();
    Q_INVOKABLE bool discardPendingPushForFile(const QString& editorFilePath);

public slots:
    void setIdleIntervalMs(int idleIntervalMs);

signals:
    void idleIntervalMsChanged();
    void rawPushRequested(
        const QString& editorFilePath,
        const QString& reason);
    void rawPushFinished(
        const QString& editorFilePath,
        const QString& reason,
        bool success,
        const QString& errorMessage);

private:
    struct PendingPush final
    {
        QString editorFilePath;
        QString editorDocumentText;
        QString reason;
        bool hasEditorDocumentText = false;
    };

    static QString normalizedPath(const QString& path);

    void schedulePush(PendingPush push);
    bool executePush(const PendingPush& push);
    void resetModifiedCountTrackingIfNeeded(const QString& editorFilePath);

    RawPushCallback m_rawPushCallback;
    QTimer m_idleTimer;
    PendingPush m_pendingPush;
    QString m_modifiedCountFilePath;
    int m_idleIntervalMs = 0;
    int m_lastModifiedCount = -1;
    bool m_hasPendingPush = false;
};
