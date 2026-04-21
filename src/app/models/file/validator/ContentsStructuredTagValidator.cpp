#include "app/models/file/validator/ContentsStructuredTagValidator.hpp"

#include "app/models/file/validator/WhatSonStructuredTagLinter.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonLocalNoteDocument.hpp"
#include "app/models/file/note/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/statistic/WhatSonNoteFileStatSupport.hpp"

#include <QMetaObject>
#include <QThreadPool>

namespace
{
    constexpr auto kApplyPersistedBodyStateForNoteSignature =
        "applyPersistedBodyStateForNote(QString,QString,QString,QString)";
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";
    constexpr auto kReloadNoteMetadataForNoteIdSignature = "reloadNoteMetadataForNoteId(QString)";
    constexpr auto kRequestViewModelHookSignature = "requestViewModelHook()";
}

ContentsStructuredTagValidator::ContentsStructuredTagValidator(QObject* parent)
    : QObject(parent)
{
}

ContentsStructuredTagValidator::~ContentsStructuredTagValidator() = default;

QObject* ContentsStructuredTagValidator::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

void ContentsStructuredTagValidator::setContentViewModel(QObject* model)
{
    if (m_contentViewModel == model)
    {
        return;
    }

    m_contentViewModel = model;
    emit contentViewModelChanged();
}

QString ContentsStructuredTagValidator::noteId() const
{
    return m_noteId;
}

void ContentsStructuredTagValidator::setNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (m_noteId == normalizedNoteId)
    {
        return;
    }

    m_noteId = normalizedNoteId;
    emit noteIdChanged();
}

bool ContentsStructuredTagValidator::correctionAuthorityEnabled() const noexcept
{
    return m_correctionAuthorityEnabled;
}

void ContentsStructuredTagValidator::setCorrectionAuthorityEnabled(const bool enabled)
{
    if (m_correctionAuthorityEnabled == enabled)
    {
        return;
    }

    m_correctionAuthorityEnabled = enabled;
    emit correctionAuthorityEnabledChanged();
}

QVariantMap ContentsStructuredTagValidator::lastCorrectionVerification() const
{
    return m_lastCorrectionVerification;
}

QString ContentsStructuredTagValidator::lastCorrectedSourceText() const
{
    return m_lastCorrectedSourceText;
}

QString ContentsStructuredTagValidator::lastCorrectionError() const
{
    return m_lastCorrectionError;
}

bool ContentsStructuredTagValidator::requestStructuredCorrection(
    const QString& sourceText,
    const QString& correctedSourceText,
    const QVariantMap& verification)
{
    return requestStructuredCorrectionForNote(
        m_noteId,
        sourceText,
        correctedSourceText,
        verification);
}

bool ContentsStructuredTagValidator::requestStructuredCorrectionForNote(
    const QString& noteId,
    const QString& sourceText,
    const QString& correctedSourceText,
    const QVariantMap& verification)
{
    const QString normalizedNoteId = noteId.trimmed().isEmpty()
        ? m_noteId.trimmed()
        : noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QString normalizedSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(sourceText);
    const WhatSonStructuredTagLinter tagLinter;
    QString normalizedCorrectedSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(correctedSourceText);
    if (normalizedCorrectedSourceText.isEmpty())
    {
        normalizedCorrectedSourceText = tagLinter.normalizeStructuredSourceText(normalizedSourceText);
    }

    if (normalizedSourceText.isEmpty()
        || normalizedCorrectedSourceText.isEmpty()
        || normalizedSourceText == normalizedCorrectedSourceText)
    {
        return false;
    }

    const QString errorMessage =
        QStringLiteral("Automatic RAW correction is disabled. Note body source may only change through editor-driven mutations.");
    m_lastCorrectionNoteId = normalizedNoteId;
    m_lastCorrectionSourceText = normalizedSourceText;
    updateLastCorrectionVerification(verification);
    updateLastCorrectedSourceText(normalizedCorrectedSourceText);
    updateLastCorrectionError(errorMessage);
    emit correctionFailed(normalizedNoteId, normalizedSourceText, errorMessage, verification);
    return false;
}

bool ContentsStructuredTagValidator::enqueueCorrectionRequest(Request request)
{
    if (request.noteId.isEmpty()
        || request.sourceText.isEmpty()
        || request.correctedSourceText.isEmpty())
    {
        return false;
    }

    if (m_requestInFlight
        && m_activeRequest.noteId == request.noteId
        && m_activeRequest.sourceText == request.sourceText
        && m_activeRequest.correctedSourceText == request.correctedSourceText)
    {
        return true;
    }

    for (int index = m_pendingRequests.size() - 1; index >= 0; --index)
    {
        const Request& pendingRequest = m_pendingRequests.at(index);
        if (pendingRequest.noteId == request.noteId
            && pendingRequest.sourceText == request.sourceText
            && pendingRequest.correctedSourceText == request.correctedSourceText)
        {
            return true;
        }
    }

    if (m_requestInFlight)
    {
        m_pendingRequests.push_back(std::move(request));
        return true;
    }

    m_activeRequest = std::move(request);
    m_requestInFlight = true;
    dispatchNextCorrectionRequest();
    return true;
}

void ContentsStructuredTagValidator::dispatchNextCorrectionRequest()
{
    if (!m_requestInFlight)
    {
        if (m_pendingRequests.isEmpty())
        {
            return;
        }
        m_activeRequest = m_pendingRequests.takeFirst();
        m_requestInFlight = true;
    }

    const Request request = m_activeRequest;
    QPointer<ContentsStructuredTagValidator> validatorGuard(this);
    QThreadPool::globalInstance()->start([validatorGuard, request]()
    {
        const Result result = ContentsStructuredTagValidator::performCorrectionRequest(request);
        if (validatorGuard != nullptr)
        {
            QMetaObject::invokeMethod(
                validatorGuard,
                [validatorGuard, result]()
                {
                    if (validatorGuard != nullptr)
                    {
                        validatorGuard->handleCorrectionRequestFinished(result);
                    }
                },
                Qt::QueuedConnection);
        }
    });
}

ContentsStructuredTagValidator::Result
ContentsStructuredTagValidator::performCorrectionRequest(const Request& request)
{
    Result result;
    result.sequence = request.sequence;
    result.noteId = request.noteId;
    result.noteDirectoryPath = request.noteDirectoryPath;
    result.sourceText = request.sourceText;
    result.correctedSourceText = request.correctedSourceText;
    result.verification = request.verification;

    WhatSonLocalNoteFileStore noteFileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = request.noteId;
    readRequest.noteDirectoryPath = request.noteDirectoryPath;

    if (!noteFileStore.readNote(readRequest, &document, &result.errorMessage))
    {
        return result;
    }

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = std::move(document);
    updateRequest.document.bodyPlainText = request.correctedSourceText;
    updateRequest.document.bodySourceText = request.correctedSourceText;
    updateRequest.persistHeader = false;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;
    updateRequest.incrementModifiedCount = false;
    updateRequest.refreshIncomingBacklinkStatistics = false;
    updateRequest.refreshAffectedBacklinkTargets = false;

    if (!noteFileStore.updateNote(updateRequest, &result.persistedDocument, &result.errorMessage))
    {
        return result;
    }

    const QString persistedDirectoryPath = result.persistedDocument.noteDirectoryPath.trimmed().isEmpty()
        ? request.noteDirectoryPath
        : result.persistedDocument.noteDirectoryPath.trimmed();
    result.noteDirectoryPath = persistedDirectoryPath;

    if (!persistedDirectoryPath.isEmpty()
        && !WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
            request.noteId,
            persistedDirectoryPath,
            false,
            &result.statisticsError))
    {
        WhatSon::Debug::trace(
            QStringLiteral("content.structuredTag.validator"),
            QStringLiteral("refreshTrackedStatistics.failed"),
            QStringLiteral("noteId=%1 error=%2").arg(request.noteId, result.statisticsError));
    }

    result.success = true;
    return result;
}

void ContentsStructuredTagValidator::handleCorrectionRequestFinished(const Result& result)
{
    if (!result.success)
    {
        updateLastCorrectionVerification(result.verification);
        updateLastCorrectedSourceText(result.correctedSourceText);
        updateLastCorrectionError(result.errorMessage);
        emit correctionFailed(result.noteId, result.sourceText, result.errorMessage, result.verification);
    }
    else
    {
        const bool appliedToContentViewModel =
            applyPersistedBodyStateToContentViewModel(result.noteId, result.persistedDocument);

        if (!result.statisticsError.isEmpty())
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.structuredTag.validator"),
                QStringLiteral("refreshTrackedStatistics.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(result.noteId, result.statisticsError));
        }

        if (!appliedToContentViewModel)
        {
            if (!reloadNoteMetadataForNote(result.noteId)
                && m_contentViewModel != nullptr
                && hasInvokableMethod(m_contentViewModel, kRequestViewModelHookSignature))
            {
                QMetaObject::invokeMethod(
                    m_contentViewModel,
                    "requestViewModelHook",
                    Qt::QueuedConnection);
            }
        }

        m_lastCorrectionNoteId = result.noteId;
        m_lastCorrectionSourceText = result.sourceText;
        updateLastCorrectionVerification(result.verification);
        updateLastCorrectedSourceText(result.correctedSourceText);
        updateLastCorrectionError(QString());
        emit correctionApplied(result.noteId, result.correctedSourceText, result.verification);
    }

    if (!m_pendingRequests.isEmpty())
    {
        m_activeRequest = m_pendingRequests.takeFirst();
        m_requestInFlight = true;
        dispatchNextCorrectionRequest();
        return;
    }

    m_activeRequest = Request();
    m_requestInFlight = false;
}

bool ContentsStructuredTagValidator::hasInvokableMethod(
    const QObject* object,
    const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString ContentsStructuredTagValidator::resolveNoteDirectoryPathForNote(const QString& noteId) const
{
    if (m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature))
    {
        return {};
    }

    QString noteDirectoryPath;
    if (!QMetaObject::invokeMethod(
            m_contentViewModel,
            "noteDirectoryPathForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, noteDirectoryPath),
            Q_ARG(QString, noteId.trimmed())))
    {
        return {};
    }

    return noteDirectoryPath.trimmed();
}

bool ContentsStructuredTagValidator::applyPersistedBodyStateToContentViewModel(
    const QString& noteId,
    const WhatSonLocalNoteDocument& document) const
{
    if (m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kApplyPersistedBodyStateForNoteSignature))
    {
        return false;
    }

    bool applied = false;
    return QMetaObject::invokeMethod(
               m_contentViewModel,
               "applyPersistedBodyStateForNote",
               Qt::DirectConnection,
               Q_RETURN_ARG(bool, applied),
               Q_ARG(QString, noteId.trimmed()),
               Q_ARG(QString, document.bodyPlainText),
               Q_ARG(QString, document.bodySourceText),
               Q_ARG(QString, document.headerStore.lastModifiedAt()))
        && applied;
}

bool ContentsStructuredTagValidator::reloadNoteMetadataForNote(const QString& noteId) const
{
    if (noteId.trimmed().isEmpty()
        || m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
    {
        return false;
    }

    bool reloaded = false;
    return QMetaObject::invokeMethod(
               m_contentViewModel,
               "reloadNoteMetadataForNoteId",
               Qt::DirectConnection,
               Q_RETURN_ARG(bool, reloaded),
               Q_ARG(QString, noteId.trimmed()))
        && reloaded;
}

void ContentsStructuredTagValidator::updateLastCorrectionVerification(const QVariantMap& verification)
{
    if (m_lastCorrectionVerification == verification)
    {
        return;
    }

    m_lastCorrectionVerification = verification;
    emit lastCorrectionVerificationChanged();
}

void ContentsStructuredTagValidator::updateLastCorrectedSourceText(const QString& correctedSourceText)
{
    if (m_lastCorrectedSourceText == correctedSourceText)
    {
        return;
    }

    m_lastCorrectedSourceText = correctedSourceText;
    emit lastCorrectedSourceTextChanged();
}

void ContentsStructuredTagValidator::updateLastCorrectionError(const QString& errorMessage)
{
    if (m_lastCorrectionError == errorMessage)
    {
        return;
    }

    m_lastCorrectionError = errorMessage;
    emit lastCorrectionErrorChanged();
}
