#include "app/models/editor/component/Agenda.h"

#include <QDateTime>
#include <QRegularExpression>

#include <algorithm>
#include <optional>

namespace
{
    constexpr int kFramePadding = 8;
    constexpr int kHeaderTaskGap = 8;
    constexpr int kTaskGap = 4;
    constexpr int kTaskInset = 8;
    constexpr int kCheckboxSize = 17;
    constexpr double kCheckboxRadius = 3.5;
    constexpr int kCheckboxTextGap = 6;
    constexpr auto kAgendaRenderVersion = "figma-279-7854-lv-checkbox-slot-v3";

    QString normalizedTagName(const QString& tagName)
    {
        QString normalized;
        const QString trimmed = tagName.trimmed();
        normalized.reserve(trimmed.size());
        for (const QChar ch : trimmed)
        {
            if (ch.isLetterOrNumber())
            {
                normalized.push_back(ch.toCaseFolded());
            }
        }
        return normalized;
    }

    QString agendaOpeningToken()
    {
        const QDateTime now = QDateTime::currentDateTime();
        return QStringLiteral("<agenda date=\"%1\" time=\"%2\"><task done=false>")
            .arg(
                now.date().toString(QStringLiteral("yyyy-MM-dd")),
                now.time().toString(QStringLiteral("HH-mm")));
    }

    QString rawAttributeValue(const QString& tagToken, const QString& attributeName)
    {
        const QRegularExpression attributePattern(
            QStringLiteral(R"rx(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))rx")
                .arg(QRegularExpression::escape(attributeName)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(tagToken);
        if (!match.hasMatch())
        {
            return {};
        }
        for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
        {
            const QString captured = match.captured(captureIndex);
            if (!captured.isNull())
            {
                return captured.trimmed();
            }
        }
        return {};
    }

    bool taskDoneFromOpeningToken(const QString& openingToken)
    {
        const QString doneValue = rawAttributeValue(openingToken, QStringLiteral("done"))
                                      .trimmed()
                                      .toCaseFolded();
        return doneValue == QStringLiteral("true")
            || doneValue == QStringLiteral("1")
            || doneValue == QStringLiteral("yes");
    }

    QString normalizedContentHtml(const QString& contentHtml)
    {
        return contentHtml.trimmed().isEmpty() ? QStringLiteral("&nbsp;") : contentHtml;
    }

    QString openingTokenFromSource(const QString& sourceText, const QString& fallback)
    {
        const int tagStart = sourceText.indexOf(QLatin1Char('<'));
        if (tagStart < 0)
        {
            return fallback;
        }
        const int tagEnd = sourceText.indexOf(QLatin1Char('>'), tagStart + 1);
        if (tagEnd <= tagStart)
        {
            return fallback;
        }
        return sourceText.mid(tagStart, tagEnd - tagStart + 1).trimmed();
    }

    int clampedPosition(const int position, const int textSize)
    {
        return std::clamp(position, 0, textSize);
    }

    void expandRemovalToSourceLine(
        const QString& bodySourceText,
        int* removeStart,
        int* removeEnd)
    {
        if (removeStart == nullptr || removeEnd == nullptr)
        {
            return;
        }

        const bool hasPreviousLineBreak =
            *removeStart > 0 && bodySourceText.at(*removeStart - 1) == QLatin1Char('\n');
        const bool hasNextLineBreak =
            *removeEnd < bodySourceText.size() && bodySourceText.at(*removeEnd) == QLatin1Char('\n');
        if (hasPreviousLineBreak && hasNextLineBreak)
        {
            ++(*removeEnd);
            return;
        }
        if (hasPreviousLineBreak)
        {
            --(*removeStart);
            return;
        }
        if (hasNextLineBreak)
        {
            ++(*removeEnd);
        }
    }
} // namespace

namespace WhatSon::EditorComponent
{
    bool AgendaSourceRange::isValid() const noexcept
    {
        return openingStart >= 0
            && openingEnd >= openingStart
            && contentStart == openingEnd
            && contentEnd >= contentStart
            && closingStart == contentEnd
            && closingEnd >= closingStart;
    }

    bool AgendaTaskSourceRange::isValid() const noexcept
    {
        return openingStart >= 0
            && openingEnd >= openingStart
            && contentStart == openingEnd
            && contentEnd >= contentStart
            && closingStart == contentEnd
            && closingEnd >= closingStart;
    }

    bool AgendaStaticTag::isValid() const noexcept
    {
        return !canonicalName.isEmpty()
            && !openingToken.isEmpty()
            && !closingToken.isEmpty();
    }

    QString Agenda::sourceMarker(const QString& sourceText)
    {
        return QString::fromLatin1(sourceText.toUtf8().toHex());
    }

    QStringList Agenda::staticTagNames()
    {
        return {
            QStringLiteral("agenda"),
            QStringLiteral("task")
        };
    }

    AgendaStaticTag Agenda::staticTagFor(const QString& tagName)
    {
        const QString normalized = normalizedTagName(tagName);
        if (normalized == QStringLiteral("agenda"))
        {
            return {
                QStringLiteral("agenda"),
                agendaOpeningToken(),
                QStringLiteral("</task></agenda>")
            };
        }
        if (normalized == QStringLiteral("task"))
        {
            return {
                QStringLiteral("task"),
                QStringLiteral("<task done=false>"),
                QStringLiteral("</task>")
            };
        }
        return {};
    }

    QString Agenda::renderHtml(const AgendaDescriptor& descriptor)
    {
        const QString sourceText = descriptor.sourceText.trimmed();
        const QString dateText = descriptor.dateText.trimmed().isEmpty()
                                     ? dateTextFromSource(sourceText)
                                     : descriptor.dateText.trimmed();
        const QString timeText = descriptor.timeText.trimmed().isEmpty()
                                     ? timeTextFromSource(sourceText)
                                     : descriptor.timeText.trimmed();
        const QString displayDateText = dateText.isEmpty() ? QStringLiteral("yyyy-mm-dd") : dateText;

        QString html;
        html.reserve(sourceText.size() + 1800 + descriptor.tasks.size() * 480);
        html += QStringLiteral("<!--whatson-agenda-source:");
        html += sourceMarker(sourceText);
        html += QStringLiteral("-->");
        html += QStringLiteral(
            "<table class=\"whatson-agenda\" data-figma-node-id=\"279:7854\" "
            "data-agenda-render=\"%1\" data-frame-width-mode=\"fill\" "
            "data-frame-height-mode=\"hug-contents\" data-frame-padding=\"%2\" "
            "data-agenda-header-gap=\"%3\" data-agenda-task-gap=\"%4\" "
            "data-agenda-task-inset=\"%5\" data-agenda-checkbox-size=\"%6\" "
            "data-agenda-checkbox-radius=\"%7\" data-agenda-checkbox-text-gap=\"%8\" "
            "data-agenda-task-component=\"LV.CheckBox\" data-agenda-date=\"%9\" "
            "data-agenda-time=\"%10\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" "
            "bgcolor=\"#262728\" "
            "style=\"width:100%;max-width:100%;height:auto;min-height:0;box-sizing:border-box;"
            "background-color:#262728;border:1px solid #343536;border-radius:12px;"
            "padding:8px;border-collapse:separate;border-spacing:0;"
            "font-family:Pretendard;color:rgba(255,255,255,0.8);"
            "white-space:normal;word-break:break-word;\">"
            "<tr><td bgcolor=\"#262728\" "
            "style=\"background-color:#262728;padding:8px;"
            "font-family:Pretendard;color:rgba(255,255,255,0.8);\">")
            .arg(
                QString::fromLatin1(kAgendaRenderVersion),
                QString::number(kFramePadding),
                QString::number(kHeaderTaskGap),
                QString::number(kTaskGap),
                QString::number(kTaskInset),
                QString::number(kCheckboxSize),
                QString::number(kCheckboxRadius),
                QString::number(kCheckboxTextGap),
                displayDateText,
                timeText);
        html += QStringLiteral(
            "<span style=\"font-size:1px;line-height:1px;color:#262728;\">"
            "__WHATSON_AGENDA_SERIALIZED_SOURCE_%1__"
            "</span>")
            .arg(sourceMarker(sourceText));
        html += QStringLiteral(
            "<table class=\"whatson-agenda-header\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" "
            "style=\"width:100%;height:auto;border-collapse:collapse;border-spacing:0;"
            "font-family:Pretendard;font-size:11px;font-weight:400;"
            "line-height:11px;color:rgba(255,255,255,0.5);white-space:nowrap;\">"
            "<tr>"
            "<td class=\"whatson-agenda-title\" width=\"50%\" "
            "style=\"font-family:Pretendard;font-size:11px;font-weight:400;"
            "line-height:11px;color:rgba(255,255,255,0.5);padding:0;white-space:nowrap;\">Agenda</td>"
            "<td class=\"whatson-agenda-date\" align=\"right\" width=\"50%\" "
            "style=\"font-family:Pretendard;font-size:11px;font-weight:400;text-align:right;"
            "line-height:11px;color:rgba(255,255,255,0.5);padding:0;white-space:nowrap;\">");
        html += displayDateText;
        html += QStringLiteral("</td></tr></table>");

        html += QStringLiteral(
            "<table class=\"whatson-agenda-tasks\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" "
            "style=\"width:100%;height:auto;margin-top:8px;border-collapse:collapse;border-spacing:0;"
            "font-family:Pretendard;color:rgba(255,255,255,0.8);\">"
            "<tr><td colspan=\"2\" height=\"8\" "
            "style=\"height:8px;font-size:1px;line-height:8px;padding:0;\">&nbsp;</td></tr>");

        for (int index = 0; index < descriptor.tasks.size(); ++index)
        {
            const AgendaTaskItem& task = descriptor.tasks.at(index);
            const QString taskContentHtml = normalizedContentHtml(task.contentHtml);
            const QString doneText = task.done ? QStringLiteral("true") : QStringLiteral("false");
            if (index > 0)
            {
                html += QStringLiteral(
                    "<tr><td colspan=\"2\" height=\"4\" "
                    "style=\"height:4px;font-size:1px;line-height:4px;padding:0;\">&nbsp;</td></tr>");
            }
            html += QStringLiteral(
                "<tr class=\"whatson-agenda-task\" data-agenda-task-done=\"%1\" "
                "style=\"height:17px;font-family:Pretendard;font-size:12px;font-weight:500;"
                "line-height:12px;color:rgba(255,255,255,0.8);white-space:nowrap;\">"
                "<td width=\"31\" valign=\"middle\" "
                "style=\"width:31px;height:17px;padding:0 0 0 8px;vertical-align:middle;\">"
                "<span class=\"whatson-agenda-checkbox-slot\" "
                "data-agenda-frame-chrome=\"true\" data-agenda-checkbox-control=\"LV.CheckBox\" "
                "style=\"display:inline-block;width:17px;height:17px;font-size:1px;"
                "line-height:17px;color:transparent;\">&nbsp;</span>"
                "</td>"
                "<td class=\"whatson-agenda-task-label\" valign=\"middle\" "
                "style=\"height:17px;padding:0;vertical-align:middle;"
                "font-family:Pretendard;font-size:12px;font-weight:500;"
                "line-height:12px;color:rgba(255,255,255,0.8);white-space:nowrap;\">"
                "<!--whatson-agenda-task:done=%1-->"
                "<span data-agenda-task-content=\"true\" "
                "style=\"font-family:Pretendard;font-size:12px;font-weight:500;"
                "line-height:12px;color:rgba(255,255,255,0.8);white-space:nowrap;\">"
                "<!--whatson-agenda-task-content-->")
                .arg(doneText);
            html += taskContentHtml;
            html += QStringLiteral(
                "<!--/whatson-agenda-task-content--></span>"
                "<!--/whatson-agenda-task-->"
                "</td></tr>");
        }

        html += QStringLiteral("</table></td></tr></table><!--/whatson-agenda-source-->");
        return html;
    }

    QVector<AgendaSourceRange> Agenda::sourceRanges(const QString& bodySourceText)
    {
        struct OpeningAgendaTag final
        {
            int start = -1;
            int end = -1;
        };

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*(/?)\s*agenda\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);

        QVector<OpeningAgendaTag> openingStack;
        QVector<AgendaSourceRange> ranges;
        QRegularExpressionMatchIterator matchIterator = tagPattern.globalMatch(bodySourceText);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            const QString token = match.captured(0).trimmed();
            const bool closingTag = !match.captured(1).isEmpty();
            const bool selfClosingTag = token.endsWith(QStringLiteral("/>"));
            if (!closingTag && !selfClosingTag)
            {
                openingStack.push_back({
                    static_cast<int>(match.capturedStart(0)),
                    static_cast<int>(match.capturedEnd(0))
                });
                continue;
            }
            if (!closingTag || openingStack.isEmpty())
            {
                continue;
            }

            const OpeningAgendaTag opening = openingStack.takeLast();
            ranges.push_back({
                opening.start,
                opening.end,
                opening.end,
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedEnd(0))
            });
        }

        std::sort(
            ranges.begin(),
            ranges.end(),
            [](const AgendaSourceRange& left, const AgendaSourceRange& right)
            {
                return left.openingStart < right.openingStart;
            });
        return ranges;
    }

    QVector<AgendaTaskSourceRange> Agenda::taskSourceRanges(const QString& agendaSourceText)
    {
        struct OpeningTaskTag final
        {
            int start = -1;
            int end = -1;
            bool done = false;
        };

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*(/?)\s*task\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);

        QVector<OpeningTaskTag> openingStack;
        QVector<AgendaTaskSourceRange> ranges;
        QRegularExpressionMatchIterator matchIterator = tagPattern.globalMatch(agendaSourceText);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            const QString token = match.captured(0).trimmed();
            const bool closingTag = !match.captured(1).isEmpty();
            const bool selfClosingTag = token.endsWith(QStringLiteral("/>"));
            if (!closingTag && !selfClosingTag)
            {
                openingStack.push_back({
                    static_cast<int>(match.capturedStart(0)),
                    static_cast<int>(match.capturedEnd(0)),
                    taskDoneFromOpeningToken(token)
                });
                continue;
            }
            if (!closingTag || openingStack.isEmpty())
            {
                continue;
            }

            const OpeningTaskTag opening = openingStack.takeLast();
            ranges.push_back({
                opening.start,
                opening.end,
                opening.end,
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedEnd(0)),
                opening.done
            });
        }

        std::sort(
            ranges.begin(),
            ranges.end(),
            [](const AgendaTaskSourceRange& left, const AgendaTaskSourceRange& right)
            {
                return left.openingStart < right.openingStart;
            });
        return ranges;
    }

    std::optional<AgendaBoundaryEdit> Agenda::backspaceAtFirstTaskContentStart(
        const QString& bodySourceText,
        const int globalTaskIndex,
        const int sourceCursorPosition)
    {
        if (globalTaskIndex < 0)
        {
            return std::nullopt;
        }

        const QVector<AgendaSourceRange> agendaRanges = sourceRanges(bodySourceText);
        int taskIndex = 0;
        for (const AgendaSourceRange& agendaRange : agendaRanges)
        {
            if (!agendaRange.isValid())
            {
                continue;
            }

            const QString agendaSource =
                bodySourceText.mid(agendaRange.openingStart, agendaRange.closingEnd - agendaRange.openingStart);
            const QVector<AgendaTaskSourceRange> taskRanges = taskSourceRanges(agendaSource);
            if (taskRanges.isEmpty())
            {
                continue;
            }

            const AgendaTaskSourceRange firstTask = taskRanges.constFirst();
            const int firstTaskGlobalIndex = taskIndex;
            taskIndex += taskRanges.size();

            Q_UNUSED(sourceCursorPosition);
            if (globalTaskIndex != firstTaskGlobalIndex)
            {
                continue;
            }

            int removeStart = agendaRange.openingStart;
            int removeEnd = agendaRange.closingEnd;
            expandRemovalToSourceLine(bodySourceText, &removeStart, &removeEnd);

            AgendaBoundaryEdit edit;
            edit.bodySourceText = bodySourceText.left(removeStart) + bodySourceText.mid(removeEnd);
            edit.sourceCursorPosition = clampedPosition(removeStart, edit.bodySourceText.size());
            edit.changed = edit.bodySourceText != bodySourceText;
            return edit;
        }
        return std::nullopt;
    }

    std::optional<AgendaBoundaryEdit> Agenda::enterInLastTask(
        const QString& bodySourceText,
        const int globalTaskIndex,
        const int sourceCursorPosition)
    {
        if (globalTaskIndex < 0)
        {
            return std::nullopt;
        }

        const QVector<AgendaSourceRange> agendaRanges = sourceRanges(bodySourceText);
        int taskIndex = 0;
        for (const AgendaSourceRange& agendaRange : agendaRanges)
        {
            if (!agendaRange.isValid())
            {
                continue;
            }

            const QString agendaSource =
                bodySourceText.mid(agendaRange.openingStart, agendaRange.closingEnd - agendaRange.openingStart);
            const QVector<AgendaTaskSourceRange> taskRanges = taskSourceRanges(agendaSource);
            if (taskRanges.isEmpty())
            {
                continue;
            }

            const int firstTaskIndex = taskIndex;
            const int lastTaskIndex = firstTaskIndex + taskRanges.size() - 1;
            taskIndex += taskRanges.size();
            if (globalTaskIndex != lastTaskIndex)
            {
                continue;
            }

            const AgendaTaskSourceRange lastTask = taskRanges.constLast();
            const QString lastTaskContent =
                agendaSource.mid(lastTask.contentStart, lastTask.contentEnd - lastTask.contentStart);
            const bool lastTaskEmpty = lastTaskContent.trimmed().isEmpty();
            Q_UNUSED(sourceCursorPosition);
            AgendaBoundaryEdit edit;
            if (lastTaskEmpty)
            {
                const int removeStart = agendaRange.openingStart + lastTask.openingStart;
                const int removeEnd = agendaRange.openingStart + lastTask.closingEnd;
                edit.bodySourceText = bodySourceText.left(removeStart) + bodySourceText.mid(removeEnd);

                const int agendaClosingEndAfterRemoval = agendaRange.closingEnd - (removeEnd - removeStart);
                int nextCursorPosition = clampedPosition(agendaClosingEndAfterRemoval, edit.bodySourceText.size());
                if (nextCursorPosition < edit.bodySourceText.size()
                    && edit.bodySourceText.at(nextCursorPosition) == QLatin1Char('\n'))
                {
                    ++nextCursorPosition;
                }
                else
                {
                    edit.bodySourceText.insert(nextCursorPosition, QLatin1Char('\n'));
                    ++nextCursorPosition;
                }

                edit.sourceCursorPosition = clampedPosition(nextCursorPosition, edit.bodySourceText.size());
                edit.cursorAfterAgenda = true;
                edit.changed = edit.bodySourceText != bodySourceText;
                return edit;
            }

            const QString insertedTask = QStringLiteral("<task done=false></task>");
            const int insertPosition = agendaRange.closingStart;
            edit.bodySourceText = bodySourceText.left(insertPosition)
                + insertedTask
                + bodySourceText.mid(insertPosition);
            edit.sourceCursorPosition = insertPosition + QStringLiteral("<task done=false>").size();
            edit.targetTaskIndex = lastTaskIndex + 1;
            edit.changed = true;
            return edit;
        }
        return std::nullopt;
    }

    QString Agenda::dateTextFromSource(const QString& agendaSourceText)
    {
        return rawAttributeValue(
            openingTokenFromSource(agendaSourceText, QStringLiteral("<agenda>")),
            QStringLiteral("date"));
    }

    QString Agenda::timeTextFromSource(const QString& agendaSourceText)
    {
        return rawAttributeValue(
            openingTokenFromSource(agendaSourceText, QStringLiteral("<agenda>")),
            QStringLiteral("time"));
    }
} // namespace WhatSon::EditorComponent
