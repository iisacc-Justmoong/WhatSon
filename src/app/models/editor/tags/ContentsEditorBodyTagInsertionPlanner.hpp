#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>

class ContentsAgendaBackend;
class ContentsCalloutBackend;

class ContentsEditorBodyTagInsertionPlanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* agendaBackend READ agendaBackend WRITE setAgendaBackend NOTIFY agendaBackendChanged)
    Q_PROPERTY(QObject* calloutBackend READ calloutBackend WRITE setCalloutBackend NOTIFY calloutBackendChanged)

public:
    explicit ContentsEditorBodyTagInsertionPlanner(QObject* parent = nullptr);
    ~ContentsEditorBodyTagInsertionPlanner() override;

    QObject* agendaBackend() const noexcept;
    void setAgendaBackend(QObject* agendaBackend);

    QObject* calloutBackend() const noexcept;
    void setCalloutBackend(QObject* calloutBackend);

    Q_INVOKABLE QVariantMap structuredShortcutInsertionSpec(const QString& shortcutKind) const;
    Q_INVOKABLE int resolveStructuredTagInsertionOffset(
        const QString& sourceText,
        int requestedInsertionOffset) const;
    Q_INVOKABLE QVariantMap buildRawSourceInsertionPayload(
        const QString& sourceText,
        int requestedInsertionOffset,
        const QString& rawSourceText,
        int cursorSourceOffsetFromInsertionStart) const;
    Q_INVOKABLE QVariantMap buildStructuredShortcutInsertionPayload(
        const QString& sourceText,
        int requestedInsertionOffset,
        const QString& shortcutKind) const;
    Q_INVOKABLE QVariantMap buildCalloutRangeWrappingPayload(
        const QString& sourceText,
        int selectionSourceStart,
        int selectionSourceEnd) const;

signals:
    void agendaBackendChanged();
    void calloutBackendChanged();

private:
    static QString normalizeSourceText(const QString& sourceText);
    static QVariantMap notAppliedPayload(const QString& reason);
    static QString spliceSourceText(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& replacementSourceText);

    QVariantMap agendaInsertionSpec() const;
    QVariantMap calloutInsertionSpec() const;
    QVariantMap breakInsertionSpec() const;

    QPointer<ContentsAgendaBackend> m_agendaBackend;
    QPointer<ContentsCalloutBackend> m_calloutBackend;
};
