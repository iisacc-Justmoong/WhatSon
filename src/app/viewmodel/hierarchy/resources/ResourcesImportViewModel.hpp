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
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit ResourcesImportViewModel(QObject* parent = nullptr);

    QString currentHubPath() const;
    void setCurrentHubPath(QString hubPath);

    bool busy() const noexcept;
    QString lastError() const;

    void setReloadResourcesCallback(std::function<bool(const QString&, QString*)> callback);

    Q_INVOKABLE bool canImportUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importUrls(const QVariantList& urls);
    Q_INVOKABLE QVariantList importUrlsForEditor(const QVariantList& urls);
    Q_INVOKABLE bool canImportDroppedUrls(const QVariantList& urls) const;
    Q_INVOKABLE bool importDroppedUrls(const QVariantList& urls);

public slots:
    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

signals:
    void currentHubPathChanged();
    void busyChanged();
    void lastErrorChanged();
    void importCompleted(int importedCount);
    void operationFailed(const QString& message);
    void viewModelHookRequested();

private:
    bool importUrlsInternal(const QVariantList& urls, QVariantList* importedEntries);
    void setBusy(bool busy);
    void setLastError(QString errorMessage);

    QString m_currentHubPath;
    bool m_busy = false;
    QString m_lastError;
    std::function<bool(const QString&, QString*)> m_reloadResourcesCallback;
};
