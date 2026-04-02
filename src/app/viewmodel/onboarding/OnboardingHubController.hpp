#pragma once

#include "IOnboardingHubController.hpp"

#include <QByteArray>
#include <QStringList>
#include <QUrl>

#include <functional>

class OnboardingHubController final : public IOnboardingHubController
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString currentHubName READ currentHubName NOTIFY currentHubNameChanged)
    Q_PROPERTY(QString currentHubPathName READ currentHubPathName NOTIFY currentHubPathNameChanged)
    Q_PROPERTY(QStringList hubSelectionCandidateNames READ hubSelectionCandidateNames NOTIFY hubSelectionCandidatesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QUrl currentFolderUrl READ currentFolderUrl NOTIFY currentFolderUrlChanged)
    Q_PROPERTY(QString sessionState READ sessionState NOTIFY sessionStateChanged)

public:
    using CreateHubCallback = std::function<bool(const QString &, QString *, QString *)>;
    using LoadHubCallback = std::function<bool(const QString &, QString *)>;

    explicit OnboardingHubController(QObject* parent = nullptr);

    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] QString currentHubName() const;
    [[nodiscard]] QString currentHubPathName() const;
    [[nodiscard]] QStringList hubSelectionCandidateNames() const;
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] QUrl currentFolderUrl() const;
    [[nodiscard]] QString sessionState() const;
    [[nodiscard]] QByteArray currentHubAccessBookmark() const;

    void setCreateHubCallback(CreateHubCallback callback);
    void setLoadHubCallback(LoadHubCallback callback);

    Q_INVOKABLE bool createHubAtUrl(const QUrl& hubUrl);
    Q_INVOKABLE bool createHubInDirectoryUrl(const QUrl& directoryUrl, const QString& preferredFileName);
    Q_INVOKABLE bool prepareHubSelectionFromUrl(const QUrl& hubUrl);
    Q_INVOKABLE bool loadHubFromUrl(const QUrl& hubUrl);
    Q_INVOKABLE bool loadHubSelectionCandidate(int index);
    Q_INVOKABLE void beginWorkspaceTransition() override;
    Q_INVOKABLE void completeWorkspaceTransition() override;
    Q_INVOKABLE void failWorkspaceTransition(const QString& message) override;

public
    slots  :



    void clearLastError();
    void clearHubSelectionCandidates();
    void requestViewHook();
    void syncCurrentHubSelection(const QString& hubPath);

    signals  :



    void busyChanged();
    void currentHubNameChanged();
    void currentHubPathNameChanged();
    void hubSelectionCandidatesChanged();
    void lastErrorChanged();
    void currentFolderUrlChanged();
    void sessionStateChanged();
    void hubCreated(const QString& hubPath);
    void hubLoaded(const QString& hubPath);
    void operationFailed(const QString& message);
    void viewHookRequested();

private:
    [[nodiscard]] QString localPathFromUrl(const QUrl& hubUrl) const;
    [[nodiscard]] QStringList hubPackageCandidatesInDirectory(const QString& directoryPath) const;
    [[nodiscard]] QString resolveExistingHubPath(const QString& selectedPath, QString* errorMessage) const;
    [[nodiscard]] bool validateMountableHubPath(const QString& hubPath, QString* errorMessage) const;
    bool loadCreatedHub(const QString& hubPath, QString* errorMessage);
    bool loadResolvedHubPath(const QString& resolvedHubPath, QString* errorMessage);
    void setBusy(bool busy);
    void setCurrentHubPath(const QString& hubPath);
    void setCurrentFolderPath(const QString& folderPath);
    void setCurrentHubAccessBookmark(const QByteArray& accessBookmark);
    void setHubSelectionCandidatePaths(const QStringList& hubCandidatePaths);
    void setLastError(const QString& errorMessage);
    void setSessionState(const QString& sessionState);

    CreateHubCallback m_createHubCallback;
    LoadHubCallback m_loadHubCallback;
    bool m_busy = false;
    QString m_currentHubName;
    QString m_currentHubPathName;
    QStringList m_hubSelectionCandidatePaths;
    QString m_lastError;
    QUrl m_currentFolderUrl;
    QByteArray m_currentHubAccessBookmark;
    QString m_sessionState = QStringLiteral("idle");
};
