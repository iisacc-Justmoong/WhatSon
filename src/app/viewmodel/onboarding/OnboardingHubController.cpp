#include "OnboardingHubController.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <utility>

namespace
{
    constexpr auto kDefaultMobileHubFileName = "Untitled.wshub";

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
        const QFileInfo hubInfo(hubPath.trimmed());
        QString fileName = hubInfo.fileName().trimmed();
        if (fileName.endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            fileName.chop(QStringLiteral(".wshub").size());
        }
        return fileName.trimmed();
    }

    QString hubPathNameFromPath(const QString& hubPath)
    {
        return QFileInfo(hubPath.trimmed()).fileName().trimmed();
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
}

OnboardingHubController::OnboardingHubController(QObject* parent)
    : QObject(parent)
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

    const QString requestedPackagePath = localPathFromUrl(hubUrl);
    if (requestedPackagePath.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Selected hub path must not be empty."));
        emit operationFailed(m_lastError);
        return false;
    }

    setCurrentHubPath(requestedPackagePath);
    setBusy(true);

    QString createdHubPath;
    QString errorMessage;
    const bool created = m_createHubCallback(requestedPackagePath, &createdHubPath, &errorMessage);

    setBusy(false);

    if (!created)
    {
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to create the WhatSon Hub.")
                         : errorMessage.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

    const QString normalizedCreatedHubPath = normalizedAbsolutePath(createdHubPath);
    if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(normalizedCreatedHubPath, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("The hub was created but iOS did not grant access to it.")
                         : accessError.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }
    setCurrentHubPath(normalizedCreatedHubPath);
    if (hubUrl.isLocalFile())
    {
        setCurrentFolderPath(QFileInfo(normalizedCreatedHubPath).absolutePath());
    }
    emit hubCreated(normalizedCreatedHubPath);

    if (loadCreatedHub(normalizedCreatedHubPath, &errorMessage))
    {
        return true;
    }

    setLastError(errorMessage.trimmed().isEmpty()
                     ? QStringLiteral("The hub was created but could not be loaded.")
                     : QStringLiteral("The hub was created but could not be loaded: %1").arg(errorMessage.trimmed()));
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
    return createHubAtUrl(WhatSon::HubPath::urlFromPath(targetHubPath));
}

bool OnboardingHubController::prepareHubSelectionFromUrl(const QUrl& hubUrl)
{
    clearLastError();
    clearHubSelectionCandidates();
    if (m_busy)
    {
        setLastError(QStringLiteral("A hub operation is already in progress."));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!m_loadHubCallback)
    {
        setLastError(QStringLiteral("Hub loading callback is not configured."));
        emit operationFailed(m_lastError);
        return false;
    }

    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::startAccessForUrl(hubUrl, false, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to access the selected WhatSon Hub location.")
                         : accessError.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

    const QString selectedPath = localPathFromUrl(hubUrl);
    const QString normalizedSelectedPath = normalizedAbsolutePath(selectedPath);
    if (normalizedSelectedPath.isEmpty())
    {
        setLastError(QStringLiteral("Selected hub path must not be empty."));
        emit operationFailed(m_lastError);
        return false;
    }

    const QFileInfo selectedInfo(normalizedSelectedPath);
    if (!selectedInfo.exists())
    {
        setLastError(QStringLiteral("Selected hub path does not exist: %1").arg(normalizedSelectedPath));
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
        emit operationFailed(m_lastError);
        return false;
    }

    if (!selectedInfo.isDir())
    {
        setLastError(QStringLiteral(
            "Selected hub path is not a directory and is not inside a WhatSon Hub package: %1")
                         .arg(normalizedSelectedPath));
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
        return true;
    }

    setLastError(QStringLiteral("Selected folder does not contain a WhatSon Hub package."));
    emit operationFailed(m_lastError);
    return false;
}

bool OnboardingHubController::loadHubFromUrl(const QUrl& hubUrl)
{
    clearLastError();
    clearHubSelectionCandidates();
    if (m_busy)
    {
        setLastError(QStringLiteral("A hub operation is already in progress."));
        emit operationFailed(m_lastError);
        return false;
    }

    if (!m_loadHubCallback)
    {
        setLastError(QStringLiteral("Hub loading callback is not configured."));
        emit operationFailed(m_lastError);
        return false;
    }

    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::startAccessForUrl(hubUrl, false, &accessError))
    {
        setLastError(accessError.trimmed().isEmpty()
                         ? QStringLiteral("Failed to access the selected WhatSon Hub location.")
                         : accessError.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

    QString errorMessage;
    const QString selectedPath = localPathFromUrl(hubUrl);
    const QString resolvedHubPath = resolveExistingHubPath(selectedPath, &errorMessage);
    if (resolvedHubPath.isEmpty())
    {
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Selected folder does not resolve to a WhatSon Hub.")
                         : errorMessage.trimmed());
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
    emit operationFailed(m_lastError);
    return false;
}

bool OnboardingHubController::loadHubSelectionCandidate(int index)
{
    clearLastError();
    if (index < 0 || index >= m_hubSelectionCandidatePaths.size())
    {
        setLastError(QStringLiteral("Selected WhatSon Hub candidate is out of range."));
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
    emit operationFailed(m_lastError);
    return false;
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

bool OnboardingHubController::loadCreatedHub(const QString& hubPath, QString* errorMessage)
{
    if (!m_loadHubCallback)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub loading callback is not configured.");
        }
        return false;
    }

    setBusy(true);
    const bool loaded = m_loadHubCallback(hubPath, errorMessage);
    setBusy(false);

    if (loaded)
    {
        emit hubLoaded(hubPath);
    }

    return loaded;
}

bool OnboardingHubController::loadResolvedHubPath(const QString& resolvedHubPath, QString* errorMessage)
{
    QString accessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(resolvedHubPath, &accessError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = accessError.trimmed().isEmpty()
                                ? QStringLiteral("Failed to access the resolved WhatSon Hub package.")
                                : accessError.trimmed();
        }
        return false;
    }

    setCurrentHubPath(resolvedHubPath);
    setBusy(true);
    QString callbackError;
    const bool loaded = m_loadHubCallback(resolvedHubPath, &callbackError);
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

    if (!WhatSon::HubPath::isNonLocalUrl(resolvedHubPath))
    {
        setCurrentFolderPath(QFileInfo(resolvedHubPath).absolutePath());
    }
    clearHubSelectionCandidates();
    emit hubLoaded(resolvedHubPath);
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
