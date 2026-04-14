#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include <functional>

class ResourcesImportViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentHubPath READ currentHubPath WRITE setCurrentHubPath NOTIFY currentHubPathChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool clipboardImageAvailable READ clipboardImageAvailable NOTIFY clipboardImageAvailableChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit ResourcesImportViewModel(QObject* parent = nullptr);

    QString currentHubPath() const;
    void setCurrentHubPath(QString hubPath);

    bool busy() const noexcept;
    bool clipboardImageAvailable() const noexcept;
    QString lastError() const;

    void setReloadResourcesCallback(std::function<bool(const QString&, QString*)> callback);

    Q_INVOKABLE bool canImportUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importUrls(const QVariantList& urls);
    Q_INVOKABLE QVariantList importUrlsForEditor(const QVariantList& urls);
    Q_INVOKABLE bool importClipboardImage();
    Q_INVOKABLE QVariantList importClipboardImageForEditor();
    Q_INVOKABLE bool canImportDroppedUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importDroppedUrls(const QVariantList& urls);
    Q_INVOKABLE bool reloadImportedResources();

public slots:
    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

signals:
    void currentHubPathChanged();
    void busyChanged();
    void clipboardImageAvailableChanged();
    void lastErrorChanged();
    void importCompleted(int importedCount);
    void operationFailed(const QString& message);
    void viewModelHookRequested();

private:
    bool importUrlsInternal(const QVariantList& urls, QVariantList* importedEntries, bool reloadRuntime);
    bool importClipboardImageInternal(QVariantList* importedEntries, bool reloadRuntime);
    void setBusy(bool busy);
    void setLastError(QString errorMessage);
    void refreshClipboardImageAvailability();

    QString m_currentHubPath;
    bool m_busy = false;
    bool m_clipboardImageAvailable = false;
    QString m_lastError;
    std::function<bool(const QString&, QString*)> m_reloadResourcesCallback;
};
