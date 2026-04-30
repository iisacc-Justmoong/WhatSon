#include "app/models/onboarding/OnboardingHubController.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/hub/WhatSonHubMountValidator.hpp"
#include "app/platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "app/platform/Apple/AppleSecurityScopedResourceAccess.hpp"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <exception>
#include <utility>

namespace
{
    constexpr auto kDefaultMobileHubFileName = "Untitled.wshub";
    constexpr auto kSessionStateIdle = "idle";
    constexpr auto kSessionStateResolvingSelection = "resolvingSelection";
    constexpr auto kSessionStateLoadingHub = "loadingHub";
    constexpr auto kSessionStateHubLoaded = "hubLoaded";
    constexpr auto kSessionStateRoutingWorkspace = "routingWorkspace";
    constexpr auto kSessionStateReady = "ready";
    constexpr auto kSessionStateFailed = "failed";

    QString normalizedAbsolutePath(const QString& path)
    {
        return WhatSon::HubPath::normalizeAbsolutePath(path);
    }

    QString defaultOnboardingFolderPath()
    {
        const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (!documentsPath.trimmed().isEmpty())
        {
            return QDir::cleanPath(documentsPath);
        }

        const QString homePath = QDir::homePath();
        if (!homePath.trimmed().isEmpty())
        {
            return QDir::cleanPath(homePath);
        }

        return QDir::currentPath();
    }

    QString hubNameFromPath(const QString& hubPath)
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizePath(hubPath);
        QString fileName;
        if (WhatSon::Android::Storage::isSupportedUri(normalizedHubPath))
        {
            fileName = WhatSon::Android::Storage::displayName(normalizedHubPath).trimmed();
        }
        else
        {
            fileName = QFileInfo(normalizedHubPath).fileName().trimmed();
        }

        if (fileName.endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            fileName.chop(QStringLiteral(".wshub").size());
        }
        return fileName.trimmed();
    }

    QString hubPathNameFromPath(const QString& hubPath)
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizePath(hubPath);
        if (WhatSon::Android::Storage::isSupportedUri(normalizedHubPath))
        {
            return WhatSon::Android::Storage::displayName(normalizedHubPath).trimmed();
        }

        return QFileInfo(normalizedHubPath).fileName().trimmed();
    }

    QString enclosingHubPackagePath(const QString& selectedPath)
    {
        const QString normalizedSelectedPath = normalizedAbsolutePath(selectedPath);
        if (normalizedSelectedPath.isEmpty())
        {
            return {};
        }

        const QFileInfo selectedInfo(normalizedSelectedPath);
        if (selectedInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            return normalizedSelectedPath;
        }

        if (WhatSon::HubPath::isNonLocalUrl(normalizedSelectedPath))
        {
            return {};
        }

        QString candidatePath = selectedInfo.isDir()
                                    ? selectedInfo.absoluteFilePath()
                                    : selectedInfo.absolutePath();

        while (!candidatePath.isEmpty())
        {
            const QFileInfo candidateInfo(candidatePath);
            if (candidateInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
            {
                return normalizedAbsolutePath(candidateInfo.absoluteFilePath());
            }

            const QString parentPath = candidateInfo.absolutePath();
            if (parentPath.isEmpty() || parentPath == candidatePath)
            {
                break;
            }

            candidatePath = parentPath;
        }

        return QString();
    }

    QString normalizedHubPackageFileName(const QString& preferredFileName)
    {
        QString fileName = preferredFileName.trimmed();
        if (fileName.isEmpty())
        {
            fileName = QString::fromLatin1(kDefaultMobileHubFileName);
        }

        if (!fileName.endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            fileName += QStringLiteral(".wshub");
        }

        return fileName;
    }

    QString uniqueHubPackagePathInDirectory(const QString& directoryPath, const QString& preferredFileName)
    {
        const QString normalizedDirectoryPath = normalizedAbsolutePath(directoryPath);
        if (normalizedDirectoryPath.isEmpty())
        {
            return QString();
        }

        const QString normalizedFileName = normalizedHubPackageFileName(preferredFileName);
        const QFileInfo preferredInfo(normalizedFileName);
        const QString preferredBaseName = preferredInfo.completeBaseName();
        const QString suffix = preferredInfo.suffix().trimmed();
        const QString normalizedSuffix = suffix.isEmpty() ? QString() : QStringLiteral(".") + suffix;
        QString candidateName = normalizedFileName;
        int candidateIndex = 2;
        while (QFileInfo::exists(WhatSon::HubPath::joinPath(normalizedDirectoryPath, candidateName)))
        {
            candidateName = QStringLiteral("%1-%2%3").arg(preferredBaseName).arg(candidateIndex).arg(normalizedSuffix);
            ++candidateIndex;
        }

        return WhatSon::HubPath::joinPath(normalizedDirectoryPath, candidateName);
    }

    bool invokeCreateHubCallbackSafely(
        const OnboardingHubController::CreateHubCallback& callback,
        const QString& requestedPath,
        QString* outPackagePath,
        QString* errorMessage)
    {
        try
        {
            return callback(requestedPath, outPackagePath, errorMessage);
        }
        catch (const std::exception& exception)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub creation raised an exception: %1")
                                    .arg(QString::fromUtf8(exception.what()));
            }
        }
        catch (...)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub creation raised an unknown exception.");
            }
        }

        return false;
    }

    bool invokeLoadHubCallbackSafely(
        const OnboardingHubController::LoadHubCallback& callback,
        const QString& hubPath,
        QString* errorMessage)
    {
        try
        {
            return callback(hubPath, errorMessage);
        }
        catch (const std::exception& exception)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub loading raised an exception: %1")
                                    .arg(QString::fromUtf8(exception.what()));
            }
        }
        catch (...)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub loading raised an unknown exception.");
            }
        }

        return false;
    }
}

OnboardingHubController::OnboardingHubController(QObject* parent)
    : IOnboardingHubController(parent)
{
    setCurrentFolderPath(defaultOnboardingFolderPath());
}

bool OnboardingHubController::busy() const noexcept
{
    return m_busy;
}

QString OnboardingHubController::currentHubName() const
{
    return m_currentHubName;
}

QString OnboardingHubController::currentHubPathName() const
{
    return m_currentHubPathName;
}

QStringList OnboardingHubController::hubSelectionCandidateNames() const
{
    QStringList candidateNames;
    candidateNames.reserve(m_hubSelectionCandidatePaths.size());
    for (const QString& candidatePath : m_hubSelectionCandidatePaths)
    {
        candidateNames.append(hubPathNameFromPath(candidatePath));
    }
    return candidateNames;
}

QString OnboardingHubController::lastError() const
{
    return m_lastError;
}

QUrl OnboardingHubController::currentFolderUrl() const
{
    return m_currentFolderUrl;
}

QString OnboardingHubController::sessionState() const
{
    return m_sessionState;
}

QByteArray OnboardingHubController::currentHubAccessBookmark() const
{
    return m_currentHubAccessBookmark;
}

void OnboardingHubController::setCreateHubCallback(CreateHubCallback callback)
{
    m_createHubCallback = std::move(callback);
}

void OnboardingHubController::setLoadHubCallback(LoadHubCallback callback)
{
    m_loadHubCallback = std::move(callback);
}

bool OnboardingHubController::createHubAtUrl(const QUrl& hubUrl)
{
    clearLastError();
    clearHubSelectionCandidates();
    if (m_busy)
    {
        setLastError(QStringLiteral("A hub operation is already in progress."));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!m_createHubCallback)
    {
        setLastError(QStringLiteral("Hub creation callback is not configured."));
        emit operationFailed(m_lastError);
        return false;
    }

    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::startAccessForUrl(hubUrl, true, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to access the selected WhatSon Hub location.")
                         : accessError.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

#if defined(Q_OS_IOS)
    const QByteArray accessBookmark = WhatSon::Apple::SecurityScopedResourceAccess::bookmarkDataForUrl(
        hubUrl,
        true,
        &accessError);
    if (accessBookmark.isEmpty())
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to persist access to the selected WhatSon Hub location.")
                         : accessError.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }
    setCurrentHubAccessBookmark(accessBookmark);
#endif

    const QString requestedPackagePath = localPathFromUrl(hubUrl);
    if (requestedPackagePath.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Selected hub path must not be empty."));
        emit operationFailed(m_lastError);
        return false;
    }

    setCurrentHubPath(requestedPackagePath);
    setSessionState(QString::fromLatin1(kSessionStateLoadingHub));
    setBusy(true);

    QString createdHubPath;
    QString errorMessage;
    const bool created = invokeCreateHubCallbackSafely(
        m_createHubCallback,
        requestedPackagePath,
        &createdHubPath,
        &errorMessage);

    setBusy(false);

    if (!created)
    {
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to create the WhatSon Hub.")
                         : errorMessage.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    const QString normalizedCreatedHubPath = normalizedAbsolutePath(createdHubPath);
    if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(normalizedCreatedHubPath, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("The hub was created but iOS did not grant access to it.")
                         : accessError.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }
    setCurrentHubPath(normalizedCreatedHubPath);
    if (hubUrl.isLocalFile())
    {
        setCurrentFolderPath(QFileInfo(normalizedCreatedHubPath).absolutePath());
    }
    emit hubCreated(normalizedCreatedHubPath);

    if (loadResolvedHubPath(normalizedCreatedHubPath, &errorMessage))
    {
        return true;
    }

    setLastError(errorMessage.trimmed().isEmpty()
                     ? QStringLiteral("The hub was created but could not be loaded.")
                     : QStringLiteral("The hub was created but could not be loaded: %1").arg(errorMessage.trimmed()));
    setSessionState(QString::fromLatin1(kSessionStateFailed));
    emit operationFailed(m_lastError);
    return false;
}

bool OnboardingHubController::createHubInDirectoryUrl(const QUrl& directoryUrl, const QString& preferredFileName)
{
    clearLastError();
    clearHubSelectionCandidates();

    const QString directoryPath = localPathFromUrl(directoryUrl);
    if (directoryPath.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Selected hub directory must not be empty."));
        emit operationFailed(m_lastError);
        return false;
    }

    const QString targetHubPath = uniqueHubPackagePathInDirectory(directoryPath, preferredFileName);
    if (targetHubPath.isEmpty())
    {
        setLastError(QStringLiteral("Failed to resolve the target WhatSon Hub directory."));
        emit operationFailed(m_lastError);
        return false;
    }

    setCurrentFolderPath(directoryPath);

    if (WhatSon::Android::Storage::isSupportedUri(directoryPath))
    {
        if (m_busy)
        {
            setLastError(QStringLiteral("A hub operation is already in progress."));
            emit operationFailed(m_lastError);
            return false;
        }

        if (!m_createHubCallback)
        {
            setLastError(QStringLiteral("Hub creation callback is not configured."));
            emit operationFailed(m_lastError);
            return false;
        }

        if (!m_loadHubCallback)
        {
            setLastError(QStringLiteral("Hub loading callback is not configured."));
            emit operationFailed(m_lastError);
            return false;
        }

        QTemporaryDir temporaryDirectory;
        if (!temporaryDirectory.isValid())
        {
            setLastError(QStringLiteral("Failed to prepare a temporary WhatSon Hub scaffold directory."));
            emit operationFailed(m_lastError);
            return false;
        }

        const QString temporaryHubPath = QDir(temporaryDirectory.path()).filePath(
            normalizedHubPackageFileName(preferredFileName));
        setSessionState(QString::fromLatin1(kSessionStateLoadingHub));
        setBusy(true);

        QString createdHubPath;
        QString errorMessage;
        bool created = invokeCreateHubCallbackSafely(
            m_createHubCallback,
            temporaryHubPath,
            &createdHubPath,
            &errorMessage);
        if (created)
        {
            const QString localScaffoldPath = normalizedAbsolutePath(
                createdHubPath.trimmed().isEmpty() ? temporaryHubPath : createdHubPath);
            QString mountedHubPath;
            created = WhatSon::Android::Storage::exportLocalHubToDirectory(
                localScaffoldPath,
                directoryPath,
                &mountedHubPath,
                &errorMessage);
            if (created)
            {
                createdHubPath = mountedHubPath;
            }
        }

        setBusy(false);

        if (!created)
        {
            setLastError(errorMessage.trimmed().isEmpty()
                             ? QStringLiteral("Failed to create the WhatSon Hub.")
                             : errorMessage.trimmed());
            setSessionState(QString::fromLatin1(kSessionStateFailed));
            emit operationFailed(m_lastError);
            return false;
        }

        const QString normalizedCreatedHubPath = normalizedAbsolutePath(createdHubPath);
        setCurrentHubPath(normalizedCreatedHubPath);
        emit hubCreated(normalizedCreatedHubPath);

        if (loadResolvedHubPath(normalizedCreatedHubPath, &errorMessage))
        {
            return true;
        }

        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("The hub was created but could not be loaded.")
                         : QStringLiteral("The hub was created but could not be loaded: %1").arg(errorMessage.trimmed()));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    return createHubAtUrl(WhatSon::HubPath::urlFromPath(targetHubPath));
}

bool OnboardingHubController::prepareHubSelectionFromUrl(const QUrl& hubUrl)
{
    clearLastError();
    clearHubSelectionCandidates();
    setSessionState(QString::fromLatin1(kSessionStateResolvingSelection));
    if (m_busy)
    {
        setLastError(QStringLiteral("A hub operation is already in progress."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!m_loadHubCallback)
    {
        setLastError(QStringLiteral("Hub loading callback is not configured."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::startAccessForUrl(hubUrl, false, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to access the selected WhatSon Hub location.")
                         : accessError.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

#if defined(Q_OS_IOS)
    const QByteArray accessBookmark = WhatSon::Apple::SecurityScopedResourceAccess::bookmarkDataForUrl(
        hubUrl,
        false,
        &accessError);
    if (accessBookmark.isEmpty())
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to persist access to the selected WhatSon Hub location.")
                         : accessError.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }
    setCurrentHubAccessBookmark(accessBookmark);
#endif

    const QString selectedPath = localPathFromUrl(hubUrl);
    const QString normalizedSelectedPath = normalizedAbsolutePath(selectedPath);
    if (normalizedSelectedPath.isEmpty())
    {
        setLastError(QStringLiteral("Selected hub path must not be empty."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (WhatSon::Android::Storage::isSupportedUri(normalizedSelectedPath))
    {
        QStringList packageCandidates;
        QString errorMessage;
        if (!WhatSon::Android::Storage::resolveHubSelection(
                normalizedSelectedPath,
                &packageCandidates,
                &errorMessage))
        {
            setLastError(errorMessage.trimmed().isEmpty()
                             ? QStringLiteral("Failed to resolve the selected WhatSon Hub.")
                             : errorMessage.trimmed());
            setSessionState(QString::fromLatin1(kSessionStateFailed));
            emit operationFailed(m_lastError);
            return false;
        }

        if (packageCandidates.size() == 1)
        {
            if (loadResolvedHubPath(packageCandidates.constFirst(), &errorMessage))
            {
                return true;
            }

            setLastError(errorMessage.trimmed().isEmpty()
                             ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                             : errorMessage.trimmed());
            setSessionState(QString::fromLatin1(kSessionStateFailed));
            emit operationFailed(m_lastError);
            return false;
        }

        if (packageCandidates.size() > 1)
        {
            setCurrentFolderPath(normalizedSelectedPath);
            setHubSelectionCandidatePaths(packageCandidates);
            setSessionState(QString::fromLatin1(kSessionStateIdle));
            return true;
        }

        setLastError(QStringLiteral("Selected Android folder does not contain a WhatSon Hub package directory."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (WhatSon::HubPath::isNonLocalUrl(normalizedSelectedPath))
    {
        QString errorMessage;
        const QString resolvedHubPath = resolveExistingHubPath(normalizedSelectedPath, &errorMessage);
        if (!resolvedHubPath.isEmpty() && loadResolvedHubPath(resolvedHubPath, &errorMessage))
        {
            return true;
        }

        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to resolve the selected WhatSon Hub.")
                         : errorMessage.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    const QFileInfo selectedInfo(normalizedSelectedPath);
    if (!selectedInfo.exists())
    {
        setLastError(QStringLiteral("Selected hub path does not exist: %1").arg(normalizedSelectedPath));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    const QString enclosingHubPath = enclosingHubPackagePath(normalizedSelectedPath);
    if (!enclosingHubPath.isEmpty())
    {
        QString errorMessage;
        if (loadResolvedHubPath(enclosingHubPath, &errorMessage))
        {
            return true;
        }

        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                         : errorMessage.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!selectedInfo.isDir())
    {
        setLastError(QStringLiteral(
            "Selected hub path is not a directory and is not inside a WhatSon Hub package: %1")
                         .arg(normalizedSelectedPath));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    const QStringList packageCandidates = hubPackageCandidatesInDirectory(normalizedSelectedPath);
    if (packageCandidates.size() == 1)
    {
        QString errorMessage;
        if (loadResolvedHubPath(packageCandidates.constFirst(), &errorMessage))
        {
            return true;
        }

        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                         : errorMessage.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

    if (packageCandidates.size() > 1)
    {
        setCurrentFolderPath(normalizedSelectedPath);
        setHubSelectionCandidatePaths(packageCandidates);
        setSessionState(QString::fromLatin1(kSessionStateIdle));
        return true;
    }

    setLastError(QStringLiteral("Selected folder does not contain a WhatSon Hub package."));
    setSessionState(QString::fromLatin1(kSessionStateFailed));
    emit operationFailed(m_lastError);
    return false;
}

bool OnboardingHubController::loadHubFromUrl(const QUrl& hubUrl)
{
    clearLastError();
    clearHubSelectionCandidates();
    setSessionState(QString::fromLatin1(kSessionStateResolvingSelection));
    if (m_busy)
    {
        setLastError(QStringLiteral("A hub operation is already in progress."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!m_loadHubCallback)
    {
        setLastError(QStringLiteral("Hub loading callback is not configured."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::startAccessForUrl(hubUrl, false, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to access the selected WhatSon Hub location.")
                         : accessError.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

#if defined(Q_OS_IOS)
    const QByteArray accessBookmark = WhatSon::Apple::SecurityScopedResourceAccess::bookmarkDataForUrl(
        hubUrl,
        false,
        &accessError);
    if (accessBookmark.isEmpty())
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to persist access to the selected WhatSon Hub location.")
                         : accessError.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }
    setCurrentHubAccessBookmark(accessBookmark);
#endif

    QString errorMessage;
    const QString selectedPath = localPathFromUrl(hubUrl);
    const QString resolvedHubPath = resolveExistingHubPath(selectedPath, &errorMessage);
    if (resolvedHubPath.isEmpty())
    {
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Selected folder does not resolve to a WhatSon Hub.")
                         : errorMessage.trimmed());
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    if (loadResolvedHubPath(resolvedHubPath, &errorMessage))
    {
        return true;
    }

    setLastError(errorMessage.trimmed().isEmpty()
                     ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                     : errorMessage.trimmed());
    setSessionState(QString::fromLatin1(kSessionStateFailed));
    emit operationFailed(m_lastError);
    return false;
}

bool OnboardingHubController::loadHubSelectionCandidate(int index)
{
    clearLastError();
    if (index < 0 || index >= m_hubSelectionCandidatePaths.size())
    {
        setLastError(QStringLiteral("Selected WhatSon Hub candidate is out of range."));
        setSessionState(QString::fromLatin1(kSessionStateFailed));
        emit operationFailed(m_lastError);
        return false;
    }

    QString errorMessage;
    if (loadResolvedHubPath(m_hubSelectionCandidatePaths.at(index), &errorMessage))
    {
        return true;
    }

    setLastError(errorMessage.trimmed().isEmpty()
                     ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                     : errorMessage.trimmed());
    setSessionState(QString::fromLatin1(kSessionStateFailed));
    emit operationFailed(m_lastError);
    return false;
}

void OnboardingHubController::beginWorkspaceTransition()
{
    setSessionState(QString::fromLatin1(kSessionStateRoutingWorkspace));
}

void OnboardingHubController::completeWorkspaceTransition()
{
    setSessionState(QString::fromLatin1(kSessionStateReady));
}

void OnboardingHubController::failWorkspaceTransition(const QString& message)
{
    if (!message.trimmed().isEmpty())
    {
        setLastError(message);
    }
    setSessionState(QString::fromLatin1(kSessionStateFailed));
}

void OnboardingHubController::clearLastError()
{
    setLastError(QString());
}

void OnboardingHubController::clearHubSelectionCandidates()
{
    setHubSelectionCandidatePaths(QStringList());
}

void OnboardingHubController::requestViewHook()
{
    emit viewHookRequested();
}

void OnboardingHubController::syncCurrentHubSelection(const QString& hubPath)
{
    const QString normalizedHubPath = normalizedAbsolutePath(hubPath);
    setCurrentHubPath(normalizedHubPath);
    if (!normalizedHubPath.isEmpty() && !WhatSon::HubPath::isNonLocalUrl(normalizedHubPath))
    {
        setCurrentFolderPath(QFileInfo(normalizedHubPath).absolutePath());
    }
}

QString OnboardingHubController::localPathFromUrl(const QUrl& hubUrl) const
{
    if (!hubUrl.isValid())
    {
        return QString();
    }

    const QString rawUrlText = hubUrl.toString(QUrl::FullyEncoded).trimmed();
    if (WhatSon::Android::Storage::isSupportedUri(rawUrlText))
    {
        return WhatSon::HubPath::normalizePath(rawUrlText);
    }

    if (rawUrlText.startsWith(QStringLiteral("content:"), Qt::CaseInsensitive))
    {
        return rawUrlText;
    }

    if (hubUrl.isLocalFile())
    {
        return WhatSon::HubPath::normalizeAbsolutePath(hubUrl.toLocalFile());
    }

    return WhatSon::HubPath::pathFromUrl(hubUrl);
}

QString OnboardingHubController::resolveExistingHubPath(
    const QString& selectedPath,
    QString* errorMessage) const
{
    const QString normalizedSelectedPath = normalizedAbsolutePath(selectedPath);
    if (normalizedSelectedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Selected hub path must not be empty.");
        }
        return QString();
    }

    if (WhatSon::Android::Storage::isSupportedUri(normalizedSelectedPath))
    {
        QStringList packageCandidates;
        if (!WhatSon::Android::Storage::resolveHubSelection(normalizedSelectedPath, &packageCandidates, errorMessage))
        {
            return {};
        }

        if (packageCandidates.size() == 1)
        {
            return WhatSon::HubPath::normalizePath(packageCandidates.constFirst());
        }

        if (errorMessage != nullptr)
        {
            if (packageCandidates.isEmpty())
            {
                *errorMessage = QStringLiteral("Selected Android folder does not contain a WhatSon Hub package directory.");
            }
            else
            {
                *errorMessage = QStringLiteral(
                    "Selected Android folder contains multiple WhatSon Hub packages. Choose a single .wshub package directory.");
            }
        }
        return {};
    }

    if (WhatSon::HubPath::isNonLocalUrl(normalizedSelectedPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(
                "The selected WhatSon Hub provider does not expose a mountable local directory path.");
        }
        return QString();
    }

    const QFileInfo selectedInfo(normalizedSelectedPath);
    if (!selectedInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Selected hub path does not exist: %1").arg(normalizedSelectedPath);
        }
        return QString();
    }

    // Mobile document/folder pickers can resolve to a path inside a package bundle rather than the
    // `.wshub` root directory itself. Always promote nested selections back to the enclosing hub.
    const QString enclosingHubPath = enclosingHubPackagePath(normalizedSelectedPath);
    if (!enclosingHubPath.isEmpty())
    {
        return enclosingHubPath;
    }

    if (!selectedInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(
                "Selected hub path is not a directory and is not inside a WhatSon Hub package: %1")
                .arg(normalizedSelectedPath);
        }
        return QString();
    }

    const QStringList packageCandidates = hubPackageCandidatesInDirectory(normalizedSelectedPath);

    if (packageCandidates.size() == 1)
    {
        return packageCandidates.constFirst();
    }

    if (errorMessage != nullptr)
    {
        if (packageCandidates.isEmpty())
        {
            *errorMessage = QStringLiteral("Selected folder does not contain a WhatSon Hub package.");
        }
        else
        {
            *errorMessage = QStringLiteral(
                "Selected folder contains multiple WhatSon Hub packages. Choose a single .wshub package directory.");
        }
    }

    return QString();
}

QStringList OnboardingHubController::hubPackageCandidatesInDirectory(const QString& directoryPath) const
{
    const QString normalizedDirectoryPath = normalizedAbsolutePath(directoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return QStringList();
    }

    const QDir selectedDirectory(normalizedDirectoryPath);
    const QFileInfoList packageCandidates = selectedDirectory.entryInfoList(
        QStringList{QStringLiteral("*.wshub")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);

    QStringList candidatePaths;
    candidatePaths.reserve(packageCandidates.size());
    for (const QFileInfo& candidateInfo : packageCandidates)
    {
        candidatePaths.append(WhatSon::HubPath::normalizePath(candidateInfo.absoluteFilePath()));
    }
    return candidatePaths;
}

bool OnboardingHubController::loadResolvedHubPath(const QString& resolvedHubPath, QString* errorMessage)
{
    const WhatSonHubMountValidator hubMountValidator;
    const WhatSonHubMountValidation mountValidation =
        hubMountValidator.resolveMountedHub(resolvedHubPath, m_currentHubAccessBookmark);
    if (!mountValidation.mounted)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = mountValidation.failureMessage.trimmed().isEmpty()
                                ? QStringLiteral("Failed to resolve the selected WhatSon Hub package.")
                                : mountValidation.failureMessage.trimmed();
        }
        return false;
    }
    const QString mountableHubPath = mountValidation.hubPath;

    setCurrentHubPath(mountableHubPath);
    setSessionState(QString::fromLatin1(kSessionStateLoadingHub));
    setBusy(true);
    QString callbackError;
    const bool loaded = invokeLoadHubCallbackSafely(m_loadHubCallback, mountableHubPath, &callbackError);
    setBusy(false);

    if (!loaded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = callbackError.trimmed().isEmpty()
                                ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                                : callbackError.trimmed();
        }
        return false;
    }

    if (!WhatSon::HubPath::isNonLocalUrl(mountableHubPath))
    {
        setCurrentFolderPath(QFileInfo(mountableHubPath).absolutePath());
    }
    clearHubSelectionCandidates();
    setSessionState(QString::fromLatin1(kSessionStateHubLoaded));
    emit hubLoaded(mountableHubPath);
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void OnboardingHubController::setBusy(bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void OnboardingHubController::setCurrentHubPath(const QString& hubPath)
{
    const QString nextHubName = hubNameFromPath(hubPath);
    const QString nextHubPathName = hubPathNameFromPath(hubPath);
    const bool hubNameChanged = m_currentHubName != nextHubName;
    const bool hubPathNameChanged = m_currentHubPathName != nextHubPathName;
    if (!hubNameChanged && !hubPathNameChanged)
    {
        return;
    }

    if (hubNameChanged)
    {
        m_currentHubName = nextHubName;
        emit currentHubNameChanged();
    }
    if (hubPathNameChanged)
    {
        m_currentHubPathName = nextHubPathName;
        emit currentHubPathNameChanged();
    }
}

void OnboardingHubController::setCurrentFolderPath(const QString& folderPath)
{
    const QString normalizedFolderPath = folderPath.trimmed().isEmpty()
                                             ? defaultOnboardingFolderPath()
                                             : normalizedAbsolutePath(folderPath);
    const QUrl nextFolderUrl = WhatSon::HubPath::urlFromPath(normalizedFolderPath);
    if (m_currentFolderUrl == nextFolderUrl)
    {
        return;
    }

    m_currentFolderUrl = nextFolderUrl;
    emit currentFolderUrlChanged();
}

void OnboardingHubController::setCurrentHubAccessBookmark(const QByteArray& accessBookmark)
{
    if (m_currentHubAccessBookmark == accessBookmark)
    {
        return;
    }

    m_currentHubAccessBookmark = accessBookmark;
}

void OnboardingHubController::setHubSelectionCandidatePaths(const QStringList& hubCandidatePaths)
{
    const QStringList normalizedCandidatePaths = [&hubCandidatePaths]() {
        QStringList normalizedPaths;
        normalizedPaths.reserve(hubCandidatePaths.size());
        for (const QString& hubCandidatePath : hubCandidatePaths)
        {
            const QString normalizedPath = normalizedAbsolutePath(hubCandidatePath);
            if (!normalizedPath.isEmpty())
            {
                normalizedPaths.append(normalizedPath);
            }
        }
        normalizedPaths.removeDuplicates();
        return normalizedPaths;
    }();

    if (m_hubSelectionCandidatePaths == normalizedCandidatePaths)
    {
        return;
    }

    m_hubSelectionCandidatePaths = normalizedCandidatePaths;
    emit hubSelectionCandidatesChanged();
}

void OnboardingHubController::setLastError(const QString& errorMessage)
{
    const QString normalizedErrorMessage = errorMessage.trimmed();
    if (m_lastError == normalizedErrorMessage)
    {
        return;
    }

    m_lastError = normalizedErrorMessage;
    emit lastErrorChanged();
}

void OnboardingHubController::setSessionState(const QString& sessionState)
{
    const QString normalizedSessionState = sessionState.trimmed().isEmpty()
                                              ? QString::fromLatin1(kSessionStateIdle)
                                              : sessionState.trimmed();
    if (m_sessionState == normalizedSessionState)
    {
        return;
    }

    m_sessionState = normalizedSessionState;
    emit sessionStateChanged();
}
