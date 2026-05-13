#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include <functional>

class ResourcesImportController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentHubPath READ currentHubPath WRITE setCurrentHubPath NOTIFY currentHubPathChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum ImportConflictPolicy
    {
        ConflictPolicyAbort = 0,
        ConflictPolicyOverwrite = 1,
        ConflictPolicyKeepBoth = 2
    };
    Q_ENUM(ImportConflictPolicy)

    explicit ResourcesImportController(QObject* parent = nullptr);

    QString currentHubPath() const;
    void setCurrentHubPath(QString hubPath);

    bool busy() const noexcept;
    QString lastError() const;

    void setReloadResourcesCallback(std::function<bool(const QString&, QString*)> callback);

    Q_INVOKABLE bool canImportUrls(const QVariantList& urls) const;
    Q_INVOKABLE QVariantMap inspectImportConflictForUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importUrls(const QVariantList& urls);
    Q_INVOKABLE bool importUrlsWithConflictPolicy(const QVariantList& urls, int conflictPolicy);
    Q_INVOKABLE QVariantList importUrlsForEditor(const QVariantList& urls);
    Q_INVOKABLE QVariantList importUrlsForEditorWithConflictPolicy(const QVariantList& urls, int conflictPolicy);
    Q_INVOKABLE bool canImportDroppedUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importDroppedUrls(const QVariantList& urls);
    Q_INVOKABLE bool reloadImportedResources();

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

private:
    bool importUrlsInternal(
        const QVariantList& urls,
        QVariantList* importedEntries,
        bool reloadRuntime,
        int conflictPolicy);
    void setBusy(bool busy);
    void setLastError(QString errorMessage);

    QString m_currentHubPath;
    bool m_busy = false;
    QString m_lastError;
    std::function<bool(const QString&, QString*)> m_reloadResourcesCallback;
};
