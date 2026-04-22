#include "app/models/content/display/ContentsDisplayDocumentSourceResolver.hpp"

ContentsDisplayDocumentSourceResolver::ContentsDisplayDocumentSourceResolver(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayDocumentSourceResolver::~ContentsDisplayDocumentSourceResolver() = default;

QString ContentsDisplayDocumentSourceResolver::selectedNoteId() const { return m_selectedNoteId; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteId(const QString& value)
{
    const QString normalized = normalizeNoteId(value);
    if (m_selectedNoteId == normalized)
        return;
    m_selectedNoteId = normalized;
    emit selectedNoteIdChanged();
}

QString ContentsDisplayDocumentSourceResolver::selectedNoteBodyNoteId() const { return m_selectedNoteBodyNoteId; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyNoteId(const QString& value)
{
    const QString normalized = normalizeNoteId(value);
    if (m_selectedNoteBodyNoteId == normalized)
        return;
    m_selectedNoteBodyNoteId = normalized;
    emit selectedNoteBodyNoteIdChanged();
}

QString ContentsDisplayDocumentSourceResolver::selectedNoteBodyText() const { return m_selectedNoteBodyText; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyText(const QString& value)
{
    if (m_selectedNoteBodyText == value)
        return;
    m_selectedNoteBodyText = value;
    emit selectedNoteBodyTextChanged();
}

bool ContentsDisplayDocumentSourceResolver::selectedNoteBodyResolved() const noexcept { return m_selectedNoteBodyResolved; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyResolved(const bool value)
{
    if (m_selectedNoteBodyResolved == value)
        return;
    m_selectedNoteBodyResolved = value;
    emit selectedNoteBodyResolvedChanged();
}

QString ContentsDisplayDocumentSourceResolver::editorBoundNoteId() const { return m_editorBoundNoteId; }
void ContentsDisplayDocumentSourceResolver::setEditorBoundNoteId(const QString& value)
{
    const QString normalized = normalizeNoteId(value);
    if (m_editorBoundNoteId == normalized)
        return;
    m_editorBoundNoteId = normalized;
    emit editorBoundNoteIdChanged();
}

QString ContentsDisplayDocumentSourceResolver::editorText() const { return m_editorText; }
void ContentsDisplayDocumentSourceResolver::setEditorText(const QString& value)
{
    if (m_editorText == value)
        return;
    m_editorText = value;
    emit editorTextChanged();
}

bool ContentsDisplayDocumentSourceResolver::pendingBodySave() const noexcept { return m_pendingBodySave; }
void ContentsDisplayDocumentSourceResolver::setPendingBodySave(const bool value)
{
    if (m_pendingBodySave == value)
        return;
    m_pendingBodySave = value;
    emit pendingBodySaveChanged();
}

QVariantMap ContentsDisplayDocumentSourceResolver::resolveDocumentSourcePlan() const
{
    QVariantMap plan;
    const bool sessionBound = !m_editorBoundNoteId.isEmpty() && m_editorBoundNoteId == m_selectedNoteId;
    const bool bodyMatches = m_selectedNoteBodyResolved
        && !m_selectedNoteId.isEmpty()
        && m_selectedNoteBodyNoteId == m_selectedNoteId;
    const bool preferEditor = sessionBound && (!m_editorText.isEmpty() || m_pendingBodySave || !bodyMatches);

    plan.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);
    plan.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    plan.insert(QStringLiteral("editorBoundNoteId"), m_editorBoundNoteId);
    plan.insert(QStringLiteral("editorSessionBoundToSelectedNote"), sessionBound);
    plan.insert(QStringLiteral("bodyMatchesSelection"), bodyMatches);
    plan.insert(QStringLiteral("preferEditorSessionSource"), preferEditor);
    plan.insert(QStringLiteral("bodyAvailable"), bodyMatches && !m_selectedNoteBodyText.isEmpty());
    plan.insert(QStringLiteral("resolvedSourceText"), preferEditor ? m_editorText : (bodyMatches ? m_selectedNoteBodyText : QString()));
    plan.insert(QStringLiteral("resolvedSourceReady"), preferEditor || bodyMatches);
    return plan;
}

QString ContentsDisplayDocumentSourceResolver::normalizeNoteId(const QString& value)
{
    return value.trimmed();
}
