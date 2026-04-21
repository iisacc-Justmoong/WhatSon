#include "app/models/content/display/ContentsDisplaySessionCoordinator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

ContentsDisplaySessionCoordinator::ContentsDisplaySessionCoordinator(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("ctor"));
}

ContentsDisplaySessionCoordinator::~ContentsDisplaySessionCoordinator()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 sessionBound=%4")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_editorSessionBoundToSelectedNote));
}

bool ContentsDisplaySessionCoordinator::editorSessionBoundToSelectedNote() const noexcept { return m_editorSessionBoundToSelectedNote; }
void ContentsDisplaySessionCoordinator::setEditorSessionBoundToSelectedNote(const bool value)
{
    if (m_editorSessionBoundToSelectedNote == value)
        return;
    m_editorSessionBoundToSelectedNote = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setEditorSessionBoundToSelectedNote"),
        QStringLiteral("value=%1").arg(value));
    emit editorSessionBoundToSelectedNoteChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteId() const { return m_selectedNoteId; }
void ContentsDisplaySessionCoordinator::setSelectedNoteId(const QString& value)
{
    if (m_selectedNoteId == value)
        return;
    m_selectedNoteId = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setSelectedNoteId"),
        QStringLiteral("value=%1").arg(value));
    emit selectedNoteIdChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteBodyNoteId() const { return m_selectedNoteBodyNoteId; }
void ContentsDisplaySessionCoordinator::setSelectedNoteBodyNoteId(const QString& value)
{
    if (m_selectedNoteBodyNoteId == value)
        return;
    m_selectedNoteBodyNoteId = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setSelectedNoteBodyNoteId"),
        QStringLiteral("value=%1").arg(value));
    emit selectedNoteBodyNoteIdChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteBodyText() const { return m_selectedNoteBodyText; }
void ContentsDisplaySessionCoordinator::setSelectedNoteBodyText(const QString& value)
{
    if (m_selectedNoteBodyText == value)
        return;
    m_selectedNoteBodyText = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setSelectedNoteBodyText"),
        WhatSon::Debug::summarizeText(value));
    emit selectedNoteBodyTextChanged();
}

bool ContentsDisplaySessionCoordinator::selectedNoteBodyResolved() const noexcept
{
    return m_selectedNoteBodyResolved;
}

void ContentsDisplaySessionCoordinator::setSelectedNoteBodyResolved(const bool value)
{
    if (m_selectedNoteBodyResolved == value)
        return;
    m_selectedNoteBodyResolved = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setSelectedNoteBodyResolved"),
        QStringLiteral("value=%1").arg(value));
    emit selectedNoteBodyResolvedChanged();
}

QString ContentsDisplaySessionCoordinator::editorText() const { return m_editorText; }
void ContentsDisplaySessionCoordinator::setEditorText(const QString& value)
{
    if (m_editorText == value)
        return;
    m_editorText = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setEditorText"),
        WhatSon::Debug::summarizeText(value));
    emit editorTextChanged();
}

QString ContentsDisplaySessionCoordinator::structuredFlowSourceText() const { return m_structuredFlowSourceText; }
void ContentsDisplaySessionCoordinator::setStructuredFlowSourceText(const QString& value)
{
    if (m_structuredFlowSourceText == value)
        return;
    m_structuredFlowSourceText = value;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("displaySessionCoordinator"),
        QStringLiteral("setStructuredFlowSourceText"),
        WhatSon::Debug::summarizeText(value));
    emit structuredFlowSourceTextChanged();
}

QString ContentsDisplaySessionCoordinator::resolvedDocumentPresentationSourceText() const
{
    if (m_editorSessionBoundToSelectedNote)
        return m_editorText;
    if (m_selectedNoteBodyResolved
        && m_selectedNoteBodyNoteId == m_selectedNoteId)
        return m_selectedNoteBodyText;
    return {};
}

QString ContentsDisplaySessionCoordinator::currentMinimapSourceText(const bool structuredHostGeometryActive) const
{
    return structuredHostGeometryActive ? m_structuredFlowSourceText : m_editorText;
}

QVariantMap ContentsDisplaySessionCoordinator::normalizedDocumentSourceMutation(const QVariant& nextSourceText) const
{
    QVariantMap result;
    const QString normalizedNextSourceText = normalizeString(nextSourceText);
    result.insert(QStringLiteral("nextSourceText"), normalizedNextSourceText);
    result.insert(QStringLiteral("currentSourceText"), m_editorText);
    result.insert(QStringLiteral("changed"), normalizedNextSourceText != m_editorText);
    return result;
}

QString ContentsDisplaySessionCoordinator::normalizeString(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
        return {};
    return value.toString();
}
