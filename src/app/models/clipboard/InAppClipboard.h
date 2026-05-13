#pragma once

#include "app/models/clipboard/ClipboardResourceImport.h"

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <functional>

class QClipboard;
class QMimeData;

class InAppClipboard final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentHubPath READ currentHubPath WRITE setCurrentHubPath NOTIFY currentHubPathChanged)
    Q_PROPERTY(bool hasResource READ hasResource NOTIFY resourceChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString resourceFileName READ resourceFileName NOTIFY resourceChanged)
    Q_PROPERTY(QString resourceFormat READ resourceFormat NOTIFY resourceChanged)
    Q_PROPERTY(QString resourceType READ resourceType NOTIFY resourceChanged)
    Q_PROPERTY(QString resourceBucket READ resourceBucket NOTIFY resourceChanged)
    Q_PROPERTY(QString resourceMimeType READ resourceMimeType NOTIFY resourceChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum ImportConflictPolicy
    {
        ConflictPolicyAbort = 0,
        ConflictPolicyOverwrite = 1,
        ConflictPolicyKeepBoth = 2
    };
    Q_ENUM(ImportConflictPolicy)

    explicit InAppClipboard(QObject* parent = nullptr);
    ~InAppClipboard() override;

    QString currentHubPath() const;
    void setCurrentHubPath(QString hubPath);

    bool hasResource() const noexcept;
    bool busy() const noexcept;
    QString resourceFileName() const;
    QString resourceFormat() const;
    QString resourceType() const;
    QString resourceBucket() const;
    QString resourceMimeType() const;
    QString lastError() const;
    QVariantMap resourceEntry() const;

    void setReloadResourcesCallback(std::function<bool(const QString&, QString*)> callback);

    const WhatSon::Clipboard::ClipboardResourceImport& resourceImport() const noexcept;
    WhatSon::Clipboard::ClipboardResourceImport takeResourceImport();

    Q_INVOKABLE bool canImportUrls(const QVariantList& urls) const;
    Q_INVOKABLE QVariantMap inspectImportConflictForUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importUrls(const QVariantList& urls);
    Q_INVOKABLE bool importUrlsWithConflictPolicy(const QVariantList& urls, int conflictPolicy);
    Q_INVOKABLE QVariantList importUrlsForEditor(const QVariantList& urls);
    Q_INVOKABLE QVariantList importUrlsForEditorWithConflictPolicy(const QVariantList& urls, int conflictPolicy);
    Q_INVOKABLE bool refreshClipboardResourceAvailabilitySnapshot();
    Q_INVOKABLE QVariantList importClipboardResourceForEditor();
    Q_INVOKABLE QVariantList importClipboardResourceForEditorWithConflictPolicy(int conflictPolicy);
    Q_INVOKABLE bool canImportDroppedUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importDroppedUrls(const QVariantList& urls);
    Q_INVOKABLE bool reloadImportedResources();
    Q_INVOKABLE bool captureSystemClipboardResource();
    bool captureResourceFromClipboard(const QClipboard* clipboard);
    bool captureResourceFromMimeData(const QMimeData* mimeData);
    Q_INVOKABLE bool setResourceFileType(const QString& fileName, const QString& mimeType = QString());
    Q_INVOKABLE bool setResourceLocalFile(const QString& localFilePath, const QString& mimeType = QString());
    Q_INVOKABLE bool setResourceBytes(
        const QByteArray& bytes,
        const QString& fileName,
        const QString& mimeType);
    Q_INVOKABLE bool setResourceText(
        const QString& text,
        const QString& fileName = QStringLiteral("clipboard-resource.txt"),
        const QString& mimeType = QStringLiteral("text/plain"));
    bool setImageResource(const QImage& image, const QString& mimeType = QStringLiteral("image/png"));
    bool setResourceImport(WhatSon::Clipboard::ClipboardResourceImport resourceImport);
    Q_INVOKABLE void clear();

public slots:
    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

signals:
    void currentHubPathChanged();
    void busyChanged();
    void lastErrorChanged();
    void importCompleted(int importedCount);
    void operationFailed(const QString& message);
    void controllerHookRequested();
    void resourceChanged();

private:
    bool importUrlsInternal(
        const QVariantList& urls,
        QVariantList* importedEntries,
        bool reloadRuntime,
        int conflictPolicy);
    bool importClipboardResourceInternal(
        QVariantList* importedEntries,
        bool reloadRuntime,
        int conflictPolicy);
    void setBusy(bool busy);
    void setLastError(QString errorMessage);

    QString m_currentHubPath;
    WhatSon::Clipboard::ClipboardResourceImport m_resourceImport;
    bool m_busy = false;
    QString m_lastError;
    std::function<bool(const QString&, QString*)> m_reloadResourcesCallback;
};
