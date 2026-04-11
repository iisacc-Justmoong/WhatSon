#include "ContentsStructuredTagValidator.hpp"

#include "WhatSonStructuredTagLinter.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonLocalNoteDocument.hpp"
#include "file/note/WhatSonLocalNoteFileStore.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/statistic/WhatSonNoteFileStatSupport.hpp"

#include <QMetaObject>

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
    if (!m_correctionAuthorityEnabled)
    {
        return false;
    }

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

    if (m_lastCorrectionNoteId == normalizedNoteId
        && m_lastCorrectionSourceText == normalizedSourceText
        && m_lastCorrectedSourceText == normalizedCorrectedSourceText
        && m_lastCorrectionError.isEmpty())
    {
        return true;
    }

    const QString noteDirectoryPath = resolveNoteDirectoryPathForNote(normalizedNoteId);
    if (noteDirectoryPath.isEmpty())
    {
        const QString errorMessage = QStringLiteral("Failed to resolve note directory path for correction.");
        updateLastCorrectionVerification(verification);
        updateLastCorrectedSourceText(normalizedCorrectedSourceText);
        updateLastCorrectionError(errorMessage);
        emit correctionFailed(normalizedNoteId, normalizedSourceText, errorMessage, verification);
        return false;
    }

    WhatSonLocalNoteFileStore noteFileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = normalizedNoteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    if (!noteFileStore.readNote(readRequest, &document, &readError))
    {
        updateLastCorrectionVerification(verification);
        updateLastCorrectedSourceText(normalizedCorrectedSourceText);
        updateLastCorrectionError(readError);
        emit correctionFailed(normalizedNoteId, normalizedSourceText, readError, verification);
        return false;
    }

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = std::move(document);
    updateRequest.document.bodyPlainText = normalizedCorrectedSourceText;
    updateRequest.document.bodySourceText = normalizedCorrectedSourceText;
    updateRequest.persistHeader = false;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;
    updateRequest.incrementModifiedCount = false;
    updateRequest.refreshIncomingBacklinkStatistics = false;
    updateRequest.refreshAffectedBacklinkTargets = false;

    WhatSonLocalNoteDocument persistedDocument;
    QString writeError;
    if (!noteFileStore.updateNote(updateRequest, &persistedDocument, &writeError))
    {
        updateLastCorrectionVerification(verification);
        updateLastCorrectedSourceText(normalizedCorrectedSourceText);
        updateLastCorrectionError(writeError);
        emit correctionFailed(normalizedNoteId, normalizedSourceText, writeError, verification);
        return false;
    }

    const QString persistedDirectoryPath = persistedDocument.noteDirectoryPath.trimmed().isEmpty()
        ? noteDirectoryPath
        : persistedDocument.noteDirectoryPath.trimmed();
    const bool appliedToContentViewModel =
        applyPersistedBodyStateToContentViewModel(normalizedNoteId, persistedDocument);

    QString statsError;
    if (!persistedDirectoryPath.isEmpty()
        && !WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
            normalizedNoteId,
            persistedDirectoryPath,
            false,
            &statsError))
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("content.structuredTag.validator"),
            QStringLiteral("refreshTrackedStatistics.failed"),
            QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, statsError));
    }

    if (!appliedToContentViewModel)
    {
        if (!reloadNoteMetadataForNote(normalizedNoteId)
            && m_contentViewModel != nullptr
            && hasInvokableMethod(m_contentViewModel, kRequestViewModelHookSignature))
        {
            QMetaObject::invokeMethod(
                m_contentViewModel,
                "requestViewModelHook",
                Qt::QueuedConnection);
        }
    }
    else
    {
        reloadNoteMetadataForNote(normalizedNoteId);
    }

    m_lastCorrectionNoteId = normalizedNoteId;
    m_lastCorrectionSourceText = normalizedSourceText;
    updateLastCorrectionVerification(verification);
    updateLastCorrectedSourceText(normalizedCorrectedSourceText);
    updateLastCorrectionError(QString());
    emit correctionApplied(normalizedNoteId, normalizedCorrectedSourceText, verification);
    return true;
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
