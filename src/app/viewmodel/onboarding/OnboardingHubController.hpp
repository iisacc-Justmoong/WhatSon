#pragma once

#include <QObject>
#include <QUrl>

#include <functional>

class OnboardingHubController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString currentHubName READ currentHubName NOTIFY currentHubNameChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QUrl currentFolderUrl READ currentFolderUrl NOTIFY currentFolderUrlChanged)

public:
    using CreateHubCallback = std::function<bool(const QString &, QString *, QString *)>;
    using LoadHubCallback = std::function<bool(const QString &, QString *)>;

    explicit OnboardingHubController(QObject* parent = nullptr);

    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] QString currentHubName() const;
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] QUrl currentFolderUrl() const;

    void setCreateHubCallback(CreateHubCallback callback);
    void setLoadHubCallback(LoadHubCallback callback);

    Q_INVOKABLE bool createHubAtUrl(const QUrl& hubUrl);
    Q_INVOKABLE bool loadHubFromUrl(const QUrl& hubUrl);

public
    slots  :


    void clearLastError();
    void requestViewHook();
    void syncCurrentHubSelection(const QString& hubPath);

    signals  :


    void busyChanged();
    void currentHubNameChanged();
    void lastErrorChanged();
    void currentFolderUrlChanged();
    void hubCreated(const QString& hubPath);
    void hubLoaded(const QString& hubPath);
    void operationFailed(const QString& message);
    void viewHookRequested();

private:
    [[nodiscard]] QString localPathFromUrl(const QUrl& hubUrl) const;
    [[nodiscard]] QString resolveExistingHubPath(const QString& selectedPath, QString* errorMessage) const;
    bool loadCreatedHub(const QString& hubPath, QString* errorMessage);
    void setBusy(bool busy);
    void setCurrentHubPath(const QString& hubPath);
    void setCurrentFolderPath(const QString& folderPath);
    void setLastError(const QString& errorMessage);

    CreateHubCallback m_createHubCallback;
    LoadHubCallback m_loadHubCallback;
    bool m_busy = false;
    QString m_currentHubName;
    QString m_lastError;
    QUrl m_currentFolderUrl;
};
