#include "app/models/editor/component/Agenda.h"

namespace
{
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
} // namespace

namespace WhatSon::EditorComponent
{
    bool AgendaStaticTag::isValid() const noexcept
    {
        return !canonicalName.isEmpty()
            && !openingToken.isEmpty()
            && !closingToken.isEmpty();
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
                QStringLiteral("<agenda><task>"),
                QStringLiteral("</task></agenda>")
            };
        }
        if (normalized == QStringLiteral("task"))
        {
            return {
                QStringLiteral("task"),
                QStringLiteral("<task>"),
                QStringLiteral("</task>")
            };
        }
        return {};
    }
} // namespace WhatSon::EditorComponent
