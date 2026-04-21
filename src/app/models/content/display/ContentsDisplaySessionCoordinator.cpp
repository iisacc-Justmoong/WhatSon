#include "app/models/content/display/ContentsDisplaySessionCoordinator.hpp"

ContentsDisplaySessionCoordinator::ContentsDisplaySessionCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplaySessionCoordinator::~ContentsDisplaySessionCoordinator() = default;

bool ContentsDisplaySessionCoordinator::editorSessionBoundToSelectedNote() const noexcept { return m_editorSessionBoundToSelectedNote; }
void ContentsDisplaySessionCoordinator::setEditorSessionBoundToSelectedNote(const bool value)
{
    if (m_editorSessionBoundToSelectedNote == value)
        return;
    m_editorSessionBoundToSelectedNote = value;
    emit editorSessionBoundToSelectedNoteChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteId() const { return m_selectedNoteId; }
void ContentsDisplaySessionCoordinator::setSelectedNoteId(const QString& value)
{
    if (m_selectedNoteId == value)
        return;
    m_selectedNoteId = value;
    emit selectedNoteIdChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteBodyNoteId() const { return m_selectedNoteBodyNoteId; }
void ContentsDisplaySessionCoordinator::setSelectedNoteBodyNoteId(const QString& value)
{
    if (m_selectedNoteBodyNoteId == value)
        return;
    m_selectedNoteBodyNoteId = value;
    emit selectedNoteBodyNoteIdChanged();
}

QString ContentsDisplaySessionCoordinator::selectedNoteBodyText() const { return m_selectedNoteBodyText; }
void ContentsDisplaySessionCoordinator::setSelectedNoteBodyText(const QString& value)
{
    if (m_selectedNoteBodyText == value)
        return;
    m_selectedNoteBodyText = value;
    emit selectedNoteBodyTextChanged();
}

QString ContentsDisplaySessionCoordinator::editorText() const { return m_editorText; }
void ContentsDisplaySessionCoordinator::setEditorText(const QString& value)
{
    if (m_editorText == value)
        return;
    m_editorText = value;
    emit editorTextChanged();
}

QString ContentsDisplaySessionCoordinator::structuredFlowSourceText() const { return m_structuredFlowSourceText; }
void ContentsDisplaySessionCoordinator::setStructuredFlowSourceText(const QString& value)
{
    if (m_structuredFlowSourceText == value)
        return;
    m_structuredFlowSourceText = value;
    emit structuredFlowSourceTextChanged();
}

QString ContentsDisplaySessionCoordinator::resolvedDocumentPresentationSourceText() const
{
    if (m_editorSessionBoundToSelectedNote)
        return m_editorText;
    if (m_selectedNoteBodyNoteId == m_selectedNoteId)
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
