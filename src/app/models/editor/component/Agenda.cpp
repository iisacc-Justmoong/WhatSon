#include "app/models/editor/component/Agenda.h"

#include <QBuffer>
#include <QDateTime>
#include <QImage>
#include <QIODevice>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>

#include <algorithm>

namespace
{
    constexpr int kFramePadding = 8;
    constexpr int kHeaderTaskGap = 8;
    constexpr int kTaskGap = 4;
    constexpr int kTaskInset = 8;
    constexpr int kCheckboxSize = 17;
    constexpr double kCheckboxRadius = 3.5;
    constexpr int kCheckboxTextGap = 6;
    constexpr auto kAgendaRenderVersion = "figma-279-7854-frame-v1";

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

    QString checkboxPngBase64(const bool done)
    {
        QImage image(kCheckboxSize, kCheckboxSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QColor bodyColor(255, 255, 255, 204);
        const QColor checkedFillColor(10, 132, 255);
        const QColor fillColor = done ? checkedFillColor : bodyColor;
        const QRectF boxRect(0.5, 0.5, kCheckboxSize - 1.0, kCheckboxSize - 1.0);

        painter.setPen(QPen(bodyColor, 0.5));
        painter.setBrush(fillColor);
        painter.drawRoundedRect(boxRect, kCheckboxRadius, kCheckboxRadius);

        if (!done)
        {
            painter.setPen(QPen(QColor(0, 0, 0, 26), 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(QRectF(1.0, 1.0, kCheckboxSize - 2.0, kCheckboxSize - 2.0), kCheckboxRadius, kCheckboxRadius);
        }
        else
        {
            QPen checkPen(bodyColor, 2.0);
            checkPen.setCapStyle(Qt::RoundCap);
            checkPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(checkPen);
            painter.setBrush(Qt::NoBrush);
            QPainterPath checkPath;
            checkPath.moveTo(kCheckboxSize * 0.25, kCheckboxSize * 0.52);
            checkPath.lineTo(kCheckboxSize * 0.43, kCheckboxSize * 0.70);
            checkPath.lineTo(kCheckboxSize * 0.69, kCheckboxSize * 0.34);
            painter.drawPath(checkPath);
        }

        painter.end();

        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return QString::fromLatin1(bytes.toBase64());
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
            "<div class=\"whatson-agenda\" data-figma-node-id=\"279:7854\" "
            "data-agenda-render=\"%1\" data-frame-width-mode=\"fill\" "
            "data-frame-height-mode=\"hug-contents\" data-frame-padding=\"%2\" "
            "data-agenda-header-gap=\"%3\" data-agenda-task-gap=\"%4\" "
            "data-agenda-task-inset=\"%5\" data-agenda-checkbox-size=\"%6\" "
            "data-agenda-checkbox-radius=\"%7\" data-agenda-checkbox-text-gap=\"%8\" "
            "data-agenda-task-component=\"LV.CheckBox\" data-agenda-date=\"%9\" "
            "data-agenda-time=\"%10\" width=\"100%\" "
            "style=\"width:100%;max-width:100%;height:auto;min-height:0;box-sizing:border-box;"
            "background-color:#262728;border:1px solid #343536;border-radius:12px;"
            "padding:8px;font-family:Pretendard;color:rgba(255,255,255,0.8);"
            "white-space:normal;word-break:break-word;\">")
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
            "<div class=\"whatson-agenda-header\" "
            "style=\"font-family:Pretendard;font-size:11px;font-weight:400;"
            "line-height:11px;color:rgba(255,255,255,0.5);white-space:nowrap;\">"
            "<span>Agenda</span>"
            "<span style=\"float:right;\">");
        html += displayDateText;
        html += QStringLiteral("</span></div>");

        html += QStringLiteral(
            "<div class=\"whatson-agenda-tasks\" "
            "style=\"margin-top:8px;margin-left:8px;margin-right:8px;\">");

        for (int index = 0; index < descriptor.tasks.size(); ++index)
        {
            const AgendaTaskItem& task = descriptor.tasks.at(index);
            const QString taskContentHtml = normalizedContentHtml(task.contentHtml);
            const QString doneText = task.done ? QStringLiteral("true") : QStringLiteral("false");
            if (index > 0)
            {
                html += QStringLiteral("<div style=\"height:4px;font-size:1px;line-height:4px;\">&nbsp;</div>");
            }
            html += QStringLiteral(
                "<div class=\"whatson-agenda-task\" data-agenda-task-done=\"%1\" "
                "style=\"font-family:Pretendard;font-size:12px;font-weight:500;"
                "line-height:12px;color:rgba(255,255,255,0.8);white-space:nowrap;\">"
                "<!--whatson-agenda-task:done=%1-->"
                "<img class=\"whatson-agenda-checkbox\" data-agenda-frame-chrome=\"true\" "
                "src=\"data:image/png;base64,%2\" width=\"17\" height=\"17\" align=\"left\" "
                "style=\"border:0;float:left;margin-right:6px;\"/>"
                "<span data-agenda-task-content=\"true\" "
                "style=\"font-family:Pretendard;font-size:12px;font-weight:500;"
                "line-height:12px;color:rgba(255,255,255,0.8);white-space:nowrap;\">"
                "<!--whatson-agenda-task-content-->")
                .arg(doneText, checkboxPngBase64(task.done));
            html += taskContentHtml;
            html += QStringLiteral(
                "<!--/whatson-agenda-task-content--></span>"
                "<!--/whatson-agenda-task-->"
                "</div>");
        }

        html += QStringLiteral("</div></div><!--/whatson-agenda-source-->");
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
