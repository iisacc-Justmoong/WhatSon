#include "OnboardingHubController.hpp"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <utility>

namespace
{
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

    const QString requestedPackagePath = localPathFromUrl(hubUrl);
    if (requestedPackagePath.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Selected hub path must be a local filesystem path."));
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

    const QString normalizedCreatedHubPath = QDir::cleanPath(QFileInfo(createdHubPath).absoluteFilePath());
    setCurrentHubPath(normalizedCreatedHubPath);
    setCurrentFolderPath(QFileInfo(normalizedCreatedHubPath).absolutePath());
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

bool OnboardingHubController::loadHubFromUrl(const QUrl& hubUrl)
{
    clearLastError();
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

    setCurrentHubPath(resolvedHubPath);
    setBusy(true);
    const bool loaded = m_loadHubCallback(resolvedHubPath, &errorMessage);
    setBusy(false);

    if (!loaded)
    {
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                         : errorMessage.trimmed());
        emit operationFailed(m_lastError);
        return false;
    }

    setCurrentFolderPath(QFileInfo(resolvedHubPath).absolutePath());
    emit hubLoaded(resolvedHubPath);
    return true;
}

void OnboardingHubController::clearLastError()
{
    setLastError(QString());
}

void OnboardingHubController::requestViewHook()
{
    emit viewHookRequested();
}

void OnboardingHubController::syncCurrentHubSelection(const QString& hubPath)
{
    const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                          ? QString()
                                          : QDir::cleanPath(QFileInfo(hubPath).absoluteFilePath());
    setCurrentHubPath(normalizedHubPath);
    if (!normalizedHubPath.isEmpty())
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
        return QDir::cleanPath(hubUrl.toLocalFile());
    }

    const QString fallbackPath = hubUrl.toString(QUrl::PreferLocalFile).trimmed();
    if (fallbackPath.isEmpty())
    {
        return QString();
    }

    return QDir::cleanPath(fallbackPath);
}

QString OnboardingHubController::resolveExistingHubPath(
    const QString& selectedPath,
    QString* errorMessage) const
{
    const QString normalizedSelectedPath = selectedPath.trimmed().isEmpty()
                                               ? QString()
                                               : QDir::cleanPath(QFileInfo(selectedPath).absoluteFilePath());
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

    if (!selectedInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Selected hub path is not a directory: %1").arg(normalizedSelectedPath);
        }
        return QString();
    }

    if (selectedInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
    {
        return normalizedSelectedPath;
    }

    const QDir selectedDirectory(normalizedSelectedPath);
    const QFileInfoList packageCandidates = selectedDirectory.entryInfoList(
        QStringList{QStringLiteral("*.wshub")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);

    if (packageCandidates.size() == 1)
    {
        return QDir::cleanPath(packageCandidates.constFirst().absoluteFilePath());
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
    if (m_currentHubName == nextHubName)
    {
        return;
    }

    m_currentHubName = nextHubName;
    emit currentHubNameChanged();
}

void OnboardingHubController::setCurrentFolderPath(const QString& folderPath)
{
    const QString normalizedFolderPath = folderPath.trimmed().isEmpty()
                                             ? defaultOnboardingFolderPath()
                                             : QDir::cleanPath(QFileInfo(folderPath).absoluteFilePath());
    const QUrl nextFolderUrl = QUrl::fromLocalFile(normalizedFolderPath);
    if (m_currentFolderUrl == nextFolderUrl)
    {
        return;
    }

    m_currentFolderUrl = nextFolderUrl;
    emit currentFolderUrlChanged();
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
