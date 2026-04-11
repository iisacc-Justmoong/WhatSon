#include "ContentsAgendaBackend.hpp"

#include <QDate>
#include <QRegularExpression>
#include <QVariantMap>
#include <algorithm>
#include <optional>

namespace
{
    struct AgendaTaskContext final
    {
        int agendaCloseEnd = 0;
        int agendaCloseStart = 0;
        int agendaOpenEnd = 0;
        int agendaOpenStart = 0;
        bool done = false;
        int taskCloseEnd = 0;
        int taskCloseStart = 0;
        int taskContentStart = 0;
        int taskOpenEnd = 0;
        int taskOpenStart = 0;
        QString taskInnerSourceText;
    };

    QVariantMap notAppliedResult()
    {
        return {
            {QStringLiteral("applied"), false}
        };
    }

    int boundedTextIndex(const QString& text, const int index)
    {
        return std::clamp(index, 0, text.size());
    }

    QString normalizePlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar(0x2028), QLatin1Char('\n'));
        text.replace(QChar(0x2029), QLatin1Char('\n'));
        text.replace(QChar(0x00A0), QLatin1Char(' '));
        return text;
    }

    QString escapeSourceLiteral(QString text)
    {
        text = normalizePlainText(std::move(text));
        text.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        text.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        text.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        text.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        text.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return text;
    }

    QString decodeSourceEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString tagAttributeValue(const QString& rawAttributes, const QString& attributeName)
    {
        if (attributeName.trimmed().isEmpty())
        {
            return {};
        }

        const QRegularExpression attributePattern(
            QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR")
                .arg(QRegularExpression::escape(attributeName.trimmed())),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawAttributes);
        if (!match.hasMatch())
        {
            return {};
        }

        if (match.capturedStart(1) >= 0)
        {
            return decodeSourceEntities(match.captured(1));
        }
        if (match.capturedStart(2) >= 0)
        {
            return decodeSourceEntities(match.captured(2));
        }
        if (match.capturedStart(3) >= 0)
        {
            return decodeSourceEntities(match.captured(3));
        }
        return {};
    }

    bool parseBooleanAttributeValue(const QString& rawValue)
    {
        const QString normalizedValue = rawValue.trimmed().toCaseFolded();
        return normalizedValue == QStringLiteral("true")
            || normalizedValue == QStringLiteral("1")
            || normalizedValue == QStringLiteral("yes");
    }

    bool parseTaskDoneValue(const QString& taskOpenToken)
    {
        static const QRegularExpression donePattern(
            QStringLiteral(R"(\bdone\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch doneMatch = donePattern.match(taskOpenToken);
        if (!doneMatch.hasMatch())
        {
            return false;
        }

        QString rawValue;
        if (doneMatch.capturedStart(1) >= 0)
        {
            rawValue = doneMatch.captured(1);
        }
        else if (doneMatch.capturedStart(2) >= 0)
        {
            rawValue = doneMatch.captured(2);
        }
        else if (doneMatch.capturedStart(3) >= 0)
        {
            rawValue = doneMatch.captured(3);
        }
        return parseBooleanAttributeValue(rawValue);
    }

    std::optional<AgendaTaskContext> agendaTaskContextAtSourceOffset(const QString& sourceText, const int sourceOffset)
    {
        const QString normalizedSourceText = sourceText;
        const int boundedOffset = boundedTextIndex(normalizedSourceText, sourceOffset);
        const QString beforeOffsetText = normalizedSourceText.left(boundedOffset);

        const int taskOpenStart = beforeOffsetText.lastIndexOf(QStringLiteral("<task"));
        if (taskOpenStart < 0)
        {
            return std::nullopt;
        }
        const int taskOpenEnd = normalizedSourceText.indexOf(QLatin1Char('>'), taskOpenStart);
        if (taskOpenEnd < 0)
        {
            return std::nullopt;
        }
        const int taskContentStart = taskOpenEnd + 1;
        const int taskCloseStart = normalizedSourceText.indexOf(QStringLiteral("</task>"), taskContentStart);
        if (taskCloseStart < 0 || boundedOffset < taskContentStart || boundedOffset > taskCloseStart)
        {
            return std::nullopt;
        }
        const int taskCloseEnd = taskCloseStart + QStringLiteral("</task>").size();

        const int agendaOpenStart = beforeOffsetText.lastIndexOf(QStringLiteral("<agenda"));
        if (agendaOpenStart < 0)
        {
            return std::nullopt;
        }
        const int agendaOpenEnd = normalizedSourceText.indexOf(QLatin1Char('>'), agendaOpenStart);
        if (agendaOpenEnd < 0 || agendaOpenEnd > taskOpenStart)
        {
            return std::nullopt;
        }
        const int agendaCloseStart = normalizedSourceText.indexOf(QStringLiteral("</agenda>"), taskCloseEnd);
        if (agendaCloseStart < 0)
        {
            return std::nullopt;
        }
        const int agendaCloseEnd = agendaCloseStart + QStringLiteral("</agenda>").size();
        if (boundedOffset > agendaCloseStart)
        {
            return std::nullopt;
        }
        const QString taskOpenToken =
            normalizedSourceText.mid(taskOpenStart, taskOpenEnd - taskOpenStart + 1);

        AgendaTaskContext context;
        context.agendaCloseEnd = agendaCloseEnd;
        context.agendaCloseStart = agendaCloseStart;
        context.agendaOpenEnd = agendaOpenEnd + 1;
        context.agendaOpenStart = agendaOpenStart;
        context.done = parseTaskDoneValue(taskOpenToken);
        context.taskCloseEnd = taskCloseEnd;
        context.taskCloseStart = taskCloseStart;
        context.taskContentStart = taskContentStart;
        context.taskOpenEnd = taskOpenEnd + 1;
        context.taskOpenStart = taskOpenStart;
        context.taskInnerSourceText = normalizedSourceText.mid(taskContentStart, taskCloseStart - taskContentStart);
        return context;
    }

    bool agendaContainsNonEmptyTaskBody(const QString& agendaInnerSourceText)
    {
        static const QRegularExpression taskPattern(
            QStringLiteral(R"(<task\b[^>]*>([\s\S]*?)</task>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression tagPattern(QStringLiteral(R"(<[^>]*>)"));
        static const QRegularExpression entityPattern(QStringLiteral(R"(&[^;]+;)"));

        QRegularExpressionMatchIterator taskIterator = taskPattern.globalMatch(agendaInnerSourceText);
        while (taskIterator.hasNext())
        {
            const QRegularExpressionMatch taskMatch = taskIterator.next();
            if (!taskMatch.hasMatch())
            {
                continue;
            }

            QString normalizedTaskVisibleText = taskMatch.captured(1);
            normalizedTaskVisibleText.replace(tagPattern, QString());
            normalizedTaskVisibleText.replace(entityPattern, QStringLiteral(" "));
            if (!normalizedTaskVisibleText.trimmed().isEmpty())
            {
                return true;
            }
        }
        return false;
    }

    QString normalizedAgendaDateForDisplay(const QString& rawDate)
    {
        const QString decodedDate = decodeSourceEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        if (parsedDate.isValid())
        {
            return parsedDate.toString(QStringLiteral("yyyy-MM-dd"));
        }
        return QStringLiteral("yyyy-mm-dd");
    }
} // namespace

ContentsAgendaBackend::ContentsAgendaBackend(QObject* parent)
    : QObject(parent)
{
}

ContentsAgendaBackend::~ContentsAgendaBackend() = default;

QVariantList ContentsAgendaBackend::parseAgendas(const QString& sourceText) const
{
    QVariantList agendas;
    if (sourceText.isEmpty())
    {
        return agendas;
    }

    static const QRegularExpression agendaPattern(
        QStringLiteral(R"(<agenda\b([^>]*)>([\s\S]*?)</agenda>)"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression taskPattern(
        QStringLiteral(R"(<task\b([^>]*)>([\s\S]*?)</task>)"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator agendaIterator = agendaPattern.globalMatch(sourceText);
    while (agendaIterator.hasNext())
    {
        const QRegularExpressionMatch agendaMatch = agendaIterator.next();
        if (!agendaMatch.hasMatch())
        {
            continue;
        }

        QVariantMap agendaEntry;
        agendaEntry.insert(
            QStringLiteral("date"),
            normalizedAgendaDateForDisplay(tagAttributeValue(agendaMatch.captured(1), QStringLiteral("date"))));

        QVariantList tasks;
        const QString innerSource = agendaMatch.captured(2);
        const int innerSourceStart = agendaMatch.capturedStart(2);
        QRegularExpressionMatchIterator taskIterator = taskPattern.globalMatch(innerSource);
        while (taskIterator.hasNext())
        {
            const QRegularExpressionMatch taskMatch = taskIterator.next();
            if (!taskMatch.hasMatch())
            {
                continue;
            }

            const QString fullTaskToken = taskMatch.captured(0);
            const int openTokenEndOffset = fullTaskToken.indexOf(QLatin1Char('>'));
            const int openTokenLength = openTokenEndOffset >= 0 ? openTokenEndOffset + 1 : 0;
            const int taskOpenTagStart = innerSourceStart + std::max(0, taskMatch.capturedStart(0));
            const int taskOpenTagEnd = taskOpenTagStart + openTokenLength;

            QVariantMap taskEntry;
            taskEntry.insert(
                QStringLiteral("done"),
                parseBooleanAttributeValue(tagAttributeValue(taskMatch.captured(1), QStringLiteral("done"))));
            taskEntry.insert(
                QStringLiteral("openTagStart"),
                taskOpenTagStart);
            taskEntry.insert(
                QStringLiteral("openTagEnd"),
                taskOpenTagEnd);
            taskEntry.insert(
                QStringLiteral("text"),
                decodeSourceEntities(taskMatch.captured(2)));
            tasks.push_back(taskEntry);
        }

        agendaEntry.insert(QStringLiteral("tasks"), tasks);
        agendas.push_back(agendaEntry);
    }

    return agendas;
}

QString ContentsAgendaBackend::rewriteTaskDoneAttribute(
    const QString& sourceText,
    int taskOpenTagStart,
    int taskOpenTagEnd,
    const bool checked) const
{
    const int safeStart = std::clamp(taskOpenTagStart, 0, sourceText.size());
    const int safeEnd = std::clamp(taskOpenTagEnd, safeStart, sourceText.size());
    const QString openTaskTag = sourceText.mid(safeStart, safeEnd - safeStart);
    if (!openTaskTag.startsWith(QStringLiteral("<task"), Qt::CaseInsensitive))
    {
        return sourceText;
    }

    const QString nextDoneValue = checked ? QStringLiteral("true") : QStringLiteral("false");
    static const QRegularExpression doneAttributePattern(
        QStringLiteral(R"(\bdone\s*=\s*(?:"[^"]*"|'[^']*'|[^\s>]+))"),
        QRegularExpression::CaseInsensitiveOption);

    QString rewrittenOpenTag;
    if (doneAttributePattern.match(openTaskTag).hasMatch())
    {
        rewrittenOpenTag = openTaskTag;
        rewrittenOpenTag.replace(doneAttributePattern, QStringLiteral("done=\"%1\"").arg(nextDoneValue));
    }
    else
    {
        const int closeBracketOffset = openTaskTag.lastIndexOf(QLatin1Char('>'));
        if (closeBracketOffset < 0)
        {
            return sourceText;
        }
        rewrittenOpenTag = openTaskTag;
        rewrittenOpenTag.insert(closeBracketOffset, QStringLiteral(" done=\"%1\"").arg(nextDoneValue));
    }

    if (rewrittenOpenTag == openTaskTag)
    {
        return sourceText;
    }

    QString rewrittenSource = sourceText;
    rewrittenSource.replace(safeStart, safeEnd - safeStart, rewrittenOpenTag);
    return rewrittenSource;
}

QVariantMap ContentsAgendaBackend::buildAgendaInsertionPayload(
    const bool done,
    const QString& taskText) const
{
    const QString escapedTaskBodyText = escapeSourceLiteral(taskText);
    const QString doneValue = done ? QStringLiteral("true") : QStringLiteral("false");
    const QString agendaOpenTag = QStringLiteral("<agenda date=\"%1\"><task done=\"%2\">")
                                      .arg(todayIsoDate(), doneValue);
    const QString agendaCloseTag = QStringLiteral("</task></agenda>");

    QVariantMap insertionPayload;
    insertionPayload.insert(QStringLiteral("applied"), true);
    insertionPayload.insert(QStringLiteral("agendaOpenTag"), agendaOpenTag);
    insertionPayload.insert(QStringLiteral("agendaCloseTag"), agendaCloseTag);
    insertionPayload.insert(
        QStringLiteral("cursorSourceOffsetFromInsertionStart"),
        agendaOpenTag.size() + escapedTaskBodyText.size());
    insertionPayload.insert(
        QStringLiteral("insertionSourceText"),
        agendaOpenTag + escapedTaskBodyText + agendaCloseTag);
    return insertionPayload;
}

QVariantMap ContentsAgendaBackend::detectTodoShortcutReplacement(
    const QString& previousPlainText,
    const int replacementStart,
    const int replacementEnd,
    const QString& insertedText) const
{
    const QString previousText = normalizePlainText(previousPlainText);
    const int safeStart = boundedTextIndex(previousText, replacementStart);
    const int safeEnd = boundedTextIndex(previousText, std::max(safeStart, replacementEnd));
    const QString normalizedInsertedText = normalizePlainText(insertedText);
    if (normalizedInsertedText.contains(QLatin1Char('<')) || normalizedInsertedText.contains(QLatin1Char('>')))
    {
        return notAppliedResult();
    }

    const QString candidateText = previousText.left(safeStart)
        + normalizedInsertedText
        + previousText.mid(safeEnd);
    const int candidateCursor = safeStart + normalizedInsertedText.size();
    const int lineAnchor = std::max(0, candidateCursor - 1);
    const int candidateLineStart =
        lineAnchor > 0 ? candidateText.lastIndexOf(QLatin1Char('\n'), lineAnchor) + 1 : 0;
    const int candidateLineEndIndex = candidateText.indexOf(QLatin1Char('\n'), candidateLineStart);
    const int candidateLineEnd = candidateLineEndIndex >= 0 ? candidateLineEndIndex : candidateText.size();
    const QString candidateLineText = candidateText.mid(candidateLineStart, candidateLineEnd - candidateLineStart);

    static const QRegularExpression todoPattern(
        QStringLiteral(R"(^([ \t]*)\[(x|X)?\][ \t]*(.*)$)"));
    const QRegularExpressionMatch todoMatch = todoPattern.match(candidateLineText);
    if (!todoMatch.hasMatch())
    {
        return notAppliedResult();
    }

    const int previousLineAnchor = std::max(0, safeStart - 1);
    const int previousLineStart =
        previousLineAnchor > 0 ? previousText.lastIndexOf(QLatin1Char('\n'), previousLineAnchor) + 1 : 0;
    const int previousLineEndIndex = previousText.indexOf(QLatin1Char('\n'), previousLineStart);
    const int previousLineEnd = previousLineEndIndex >= 0 ? previousLineEndIndex : previousText.size();
    const bool done = !todoMatch.captured(2).isEmpty();
    const QVariantMap insertionPayload = buildAgendaInsertionPayload(done, todoMatch.captured(3));

    QVariantMap replacementPayload;
    replacementPayload.insert(QStringLiteral("applied"), true);
    replacementPayload.insert(
        QStringLiteral("cursorSourceOffsetFromReplacementStart"),
        insertionPayload.value(QStringLiteral("cursorSourceOffsetFromInsertionStart")).toInt());
    replacementPayload.insert(QStringLiteral("replacementStart"), previousLineStart);
    replacementPayload.insert(QStringLiteral("replacementEnd"), previousLineEnd);
    replacementPayload.insert(
        QStringLiteral("replacementSourceText"),
        insertionPayload.value(QStringLiteral("insertionSourceText")).toString());
    return replacementPayload;
}

QVariantMap ContentsAgendaBackend::detectAgendaTaskEnterReplacement(
    const QString& sourceText,
    const int sourceStart,
    const int sourceEnd,
    const QString& insertedText) const
{
    const QString normalizedInsertedText = normalizePlainText(insertedText);
    if (!normalizedInsertedText.endsWith(QLatin1Char('\n')))
    {
        return notAppliedResult();
    }
    if (normalizedInsertedText.count(QLatin1Char('\n')) != 1)
    {
        return notAppliedResult();
    }

    const std::optional<AgendaTaskContext> taskContext = agendaTaskContextAtSourceOffset(sourceText, sourceStart);
    if (!taskContext.has_value())
    {
        return notAppliedResult();
    }

    const QString insertedWithoutNewline = normalizedInsertedText.left(std::max(0, normalizedInsertedText.size() - 1));
    QString normalizedTaskVisibleText = taskContext->taskInnerSourceText;
    normalizedTaskVisibleText.replace(QRegularExpression(QStringLiteral("<[^>]*>")), QString());
    normalizedTaskVisibleText.replace(QRegularExpression(QStringLiteral("&[^;]+;")), QStringLiteral(" "));
    normalizedTaskVisibleText = normalizedTaskVisibleText.trimmed();
    const bool taskIsEffectivelyEmpty =
        normalizedTaskVisibleText.isEmpty() && insertedWithoutNewline.trimmed().isEmpty();

    const QString normalizedSourceText = sourceText;
    if (taskIsEffectivelyEmpty)
    {
        const QString agendaInnerSourceText = normalizedSourceText.mid(
            taskContext->agendaOpenEnd,
            taskContext->agendaCloseStart - taskContext->agendaOpenEnd);
        if (!agendaContainsNonEmptyTaskBody(agendaInnerSourceText))
        {
            return {
                {QStringLiteral("applied"), true},
                {QStringLiteral("cursorSourceOffsetFromReplacementStart"), 1},
                {QStringLiteral("replacementSourceStart"), taskContext->agendaOpenStart},
                {QStringLiteral("replacementSourceEnd"), taskContext->agendaCloseEnd},
                {QStringLiteral("replacementSourceText"), QStringLiteral("\n")}
            };
        }

        QString agendaBeforeCurrentTask =
            normalizedSourceText.mid(
                taskContext->agendaOpenEnd,
                taskContext->taskOpenStart - taskContext->agendaOpenEnd);
        QString agendaAfterCurrentTask =
            normalizedSourceText.mid(
                taskContext->taskCloseEnd,
                taskContext->agendaCloseStart - taskContext->taskCloseEnd);
        agendaBeforeCurrentTask.replace(QRegularExpression(QStringLiteral("\\s+")), QString());
        agendaAfterCurrentTask.replace(QRegularExpression(QStringLiteral("\\s+")), QString());

        QVariantMap replacementPayload;
        replacementPayload.insert(QStringLiteral("applied"), true);
        if (agendaBeforeCurrentTask.isEmpty() && agendaAfterCurrentTask.isEmpty())
        {
            replacementPayload.insert(QStringLiteral("cursorSourceOffsetFromReplacementStart"), 1);
            replacementPayload.insert(QStringLiteral("replacementSourceStart"), taskContext->agendaOpenStart);
            replacementPayload.insert(QStringLiteral("replacementSourceEnd"), taskContext->agendaCloseEnd);
            replacementPayload.insert(QStringLiteral("replacementSourceText"), QStringLiteral("\n"));
            return replacementPayload;
        }

        const QString exitAgendaText = QStringLiteral("</agenda>\n");
        replacementPayload.insert(
            QStringLiteral("cursorSourceOffsetFromReplacementStart"),
            exitAgendaText.size());
        replacementPayload.insert(QStringLiteral("replacementSourceStart"), taskContext->taskOpenStart);
        replacementPayload.insert(QStringLiteral("replacementSourceEnd"), taskContext->agendaCloseEnd);
        replacementPayload.insert(QStringLiteral("replacementSourceText"), exitAgendaText);
        return replacementPayload;
    }

    const int safeStart = boundedTextIndex(normalizedSourceText, sourceStart);
    const int safeEnd = boundedTextIndex(normalizedSourceText, std::max(safeStart, sourceEnd));
    const QString replacementSourceText =
        escapeSourceLiteral(insertedWithoutNewline) + QStringLiteral("</task><task done=\"false\">");

    return {
        {QStringLiteral("applied"), true},
        {QStringLiteral("cursorSourceOffsetFromReplacementStart"), replacementSourceText.size()},
        {QStringLiteral("replacementSourceStart"), safeStart},
        {QStringLiteral("replacementSourceEnd"), safeEnd},
        {QStringLiteral("replacementSourceText"), replacementSourceText}
    };
}

QString ContentsAgendaBackend::normalizeAgendaModifiedDate(const QString& sourceText) const
{
    if (sourceText.isEmpty())
    {
        return sourceText;
    }

    const QString todayDate = todayIsoDate();
    QString normalizedSource = sourceText;

    static const QRegularExpression dateDoubleQuotePattern(
        QStringLiteral(R"((<agenda\b[^>]*\bdate\s*=\s*")yyyy-mm-dd("))"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression dateSingleQuotePattern(
        QStringLiteral(R"((<agenda\b[^>]*\bdate\s*=\s*')yyyy-mm-dd('))"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression dateUnquotedPattern(
        QStringLiteral(R"((<agenda\b[^>]*\bdate\s*=\s*)yyyy-mm-dd(\b))"),
        QRegularExpression::CaseInsensitiveOption);

    normalizedSource.replace(
        dateDoubleQuotePattern,
        QStringLiteral("\\1%1\\2").arg(todayDate));
    normalizedSource.replace(
        dateSingleQuotePattern,
        QStringLiteral("\\1%1\\2").arg(todayDate));
    normalizedSource.replace(
        dateUnquotedPattern,
        QStringLiteral("\\1%1\\2").arg(todayDate));

    return normalizedSource;
}

QString ContentsAgendaBackend::todayIsoDate() const
{
    return QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
}
