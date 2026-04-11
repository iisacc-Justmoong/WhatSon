#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsAgendaBackend : public QObject
{
    Q_OBJECT

public:
    explicit ContentsAgendaBackend(QObject* parent = nullptr);
    ~ContentsAgendaBackend() override;

    Q_INVOKABLE QVariantList parseAgendas(const QString& sourceText) const;
    Q_INVOKABLE QString rewriteTaskDoneAttribute(
        const QString& sourceText,
        int taskOpenTagStart,
        int taskOpenTagEnd,
        bool checked) const;
    Q_INVOKABLE QVariantMap buildAgendaInsertionPayload(
        bool done = false,
        const QString& taskText = QString()) const;
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
};
