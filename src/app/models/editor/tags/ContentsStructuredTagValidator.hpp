#pragma once

#include "app/models/file/note/WhatSonLocalNoteDocument.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QVector>
#include <QtTypes>

class ContentsStructuredTagValidator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentViewModel READ contentViewModel WRITE setContentViewModel NOTIFY contentViewModelChanged)
    Q_PROPERTY(QString noteId READ noteId WRITE setNoteId NOTIFY noteIdChanged)
    Q_PROPERTY(bool correctionAuthorityEnabled READ correctionAuthorityEnabled WRITE setCorrectionAuthorityEnabled NOTIFY correctionAuthorityEnabledChanged)
    Q_PROPERTY(QVariantMap lastCorrectionVerification READ lastCorrectionVerification NOTIFY lastCorrectionVerificationChanged)
    Q_PROPERTY(QString lastCorrectedSourceText READ lastCorrectedSourceText NOTIFY lastCorrectedSourceTextChanged)
    Q_PROPERTY(QString lastCorrectionError READ lastCorrectionError NOTIFY lastCorrectionErrorChanged)

public:
    explicit ContentsStructuredTagValidator(QObject* parent = nullptr);
    ~ContentsStructuredTagValidator() override;

    QObject* contentViewModel() const noexcept;
    void setContentViewModel(QObject* model);

    QString noteId() const;
    void setNoteId(const QString& noteId);

    bool correctionAuthorityEnabled() const noexcept;
    void setCorrectionAuthorityEnabled(bool enabled);

    QVariantMap lastCorrectionVerification() const;
    QString lastCorrectedSourceText() const;
    QString lastCorrectionError() const;

public slots:
    bool requestStructuredCorrectionForNote(
        const QString& noteId,
        const QString& sourceText,
        const QString& correctedSourceText,
        const QVariantMap& verification);
    bool requestStructuredCorrection(
        const QString& sourceText,
        const QString& correctedSourceText,
        const QVariantMap& verification);

signals:
    void contentViewModelChanged();
    void noteIdChanged();
    void correctionAuthorityEnabledChanged();
    void lastCorrectionVerificationChanged();
    void lastCorrectedSourceTextChanged();
    void lastCorrectionErrorChanged();
    void correctionApplied(
        const QString& noteId,
        const QString& correctedSourceText,
        const QVariantMap& verification);
    void correctionFailed(
        const QString& noteId,
        const QString& sourceText,
        const QString& errorMessage,
        const QVariantMap& verification);

private:
    struct Request final
    {
        quint64 sequence = 0;
        QString noteId;
        QString noteDirectoryPath;
        QString sourceText;
        QString correctedSourceText;
        QVariantMap verification;
    };

    struct Result final
    {
        quint64 sequence = 0;
        QString noteId;
        QString noteDirectoryPath;
        QString sourceText;
        QString correctedSourceText;
        QVariantMap verification;
        WhatSonLocalNoteDocument persistedDocument;
        QString errorMessage;
        QString statisticsError;
        bool success = false;
    };

    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);

    bool enqueueCorrectionRequest(Request request);
    void dispatchNextCorrectionRequest();
    static Result performCorrectionRequest(const Request& request);
    void handleCorrectionRequestFinished(const Result& result);
    QString resolveNoteDirectoryPathForNote(const QString& noteId) const;
    bool applyPersistedBodyStateToContentViewModel(
        const QString& noteId,
        const WhatSonLocalNoteDocument& document) const;
    bool reloadNoteMetadataForNote(const QString& noteId) const;
    void updateLastCorrectionVerification(const QVariantMap& verification);
    void updateLastCorrectedSourceText(const QString& correctedSourceText);
    void updateLastCorrectionError(const QString& errorMessage);

    QPointer<QObject> m_contentViewModel;
    QString m_noteId;
    bool m_correctionAuthorityEnabled = false;
    QVariantMap m_lastCorrectionVerification;
    QString m_lastCorrectedSourceText;
    QString m_lastCorrectionError;
    QString m_lastCorrectionNoteId;
    QString m_lastCorrectionSourceText;
    bool m_requestInFlight = false;
    quint64 m_nextRequestSequence = 1;
    Request m_activeRequest;
    QVector<Request> m_pendingRequests;
};
