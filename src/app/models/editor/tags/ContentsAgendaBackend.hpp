#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsAgendaBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap lastParseVerification READ lastParseVerification NOTIFY lastParseVerificationChanged)

public:
    explicit ContentsAgendaBackend(QObject* parent = nullptr);
    ~ContentsAgendaBackend() override;

    QVariantMap lastParseVerification() const;

    Q_INVOKABLE QVariantList parseAgendas(const QString& sourceText);
    Q_INVOKABLE QString rewriteTaskDoneAttribute(
        const QString& sourceText,
        int taskOpenTagStart,
        int taskOpenTagEnd,
        bool checked) const;
    Q_INVOKABLE QVariantMap detectTodoShortcutReplacement(
        const QString& previousPlainText,
        int replacementStart,
        int replacementEnd,
        const QString& insertedText) const;
    Q_INVOKABLE QVariantMap detectAgendaTaskEnterReplacement(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& insertedText) const;
    Q_INVOKABLE QString normalizeAgendaModifiedDate(const QString& sourceText) const;
    Q_INVOKABLE QString todayIsoDate() const;

signals:
    void lastParseVerificationChanged();
    void parseVerificationReported(const QVariantMap& verification);

private:
    void updateLastParseVerification(const QVariantMap& verification);

    QVariantMap m_lastParseVerification;
};
