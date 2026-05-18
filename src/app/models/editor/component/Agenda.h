#pragma once

#include <optional>
#include <QString>
#include <QStringList>
#include <QVector>

namespace WhatSon::EditorComponent
{
    struct AgendaStaticTag final
    {
        QString canonicalName;
        QString openingToken;
        QString closingToken;

        bool isValid() const noexcept;
    };

    struct AgendaTaskItem final
    {
        QString sourceText;
        QString contentHtml;
        bool done = false;
    };

    struct AgendaDescriptor final
    {
        QString sourceText;
        QString dateText;
        QString timeText;
        QVector<AgendaTaskItem> tasks;
        int editorViewportWidth = 0;
    };

    struct AgendaSourceRange final
    {
        int openingStart = -1;
        int openingEnd = -1;
        int contentStart = -1;
        int contentEnd = -1;
        int closingStart = -1;
        int closingEnd = -1;

        bool isValid() const noexcept;
    };

    struct AgendaTaskSourceRange final
    {
        int openingStart = -1;
        int openingEnd = -1;
        int contentStart = -1;
        int contentEnd = -1;
        int closingStart = -1;
        int closingEnd = -1;
        bool done = false;

        bool isValid() const noexcept;
    };

    struct AgendaBoundaryEdit final
    {
        QString bodySourceText;
        int sourceCursorPosition = 0;
        int targetTaskIndex = -1;
        bool cursorAfterAgenda = false;
        bool changed = false;
    };

    class Agenda final
    {
    public:
        static QString sourceMarker(const QString& sourceText);
        static QStringList staticTagNames();
        static AgendaStaticTag staticTagFor(const QString& tagName);
        static QString renderHtml(const AgendaDescriptor& descriptor);
        static QVector<AgendaSourceRange> sourceRanges(const QString& bodySourceText);
        static QVector<AgendaTaskSourceRange> taskSourceRanges(const QString& agendaSourceText);
        static std::optional<AgendaBoundaryEdit> backspaceAtFirstTaskContentStart(
            const QString& bodySourceText,
            int globalTaskIndex,
            int sourceCursorPosition);
        static std::optional<AgendaBoundaryEdit> enterInLastTask(
            const QString& bodySourceText,
            int globalTaskIndex,
            int sourceCursorPosition);
        static QString dateTextFromSource(const QString& agendaSourceText);
        static QString timeTextFromSource(const QString& agendaSourceText);
    };
} // namespace WhatSon::EditorComponent
