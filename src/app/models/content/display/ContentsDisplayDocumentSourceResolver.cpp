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

QString ContentsDisplayDocumentSourceResolver::structuredFlowSourceText() const { return m_structuredFlowSourceText; }
void ContentsDisplayDocumentSourceResolver::setStructuredFlowSourceText(const QString& value)
{
    if (m_structuredFlowSourceText == value)
        return;
    m_structuredFlowSourceText = value;
    emit structuredFlowSourceTextChanged();
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
    const bool sessionBound = editorSessionBoundToSelectedNote();
    const bool bodyIdMatches = !m_selectedNoteId.isEmpty() && m_selectedNoteBodyNoteId == m_selectedNoteId;
    const bool bodyHasText = !m_selectedNoteBodyText.isEmpty();
    const bool bodyAvailable = bodyHasText && (m_selectedNoteBodyResolved || bodyIdMatches || m_selectedNoteBodyNoteId.isEmpty());
    const bool editorAvailable = !m_editorText.isEmpty() && (sessionBound || m_pendingBodySave || m_editorBoundNoteId.isEmpty() || m_selectedNoteId.isEmpty());
    const bool preferEditor = editorAvailable && (!bodyAvailable || sessionBound || m_pendingBodySave);

    plan.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);
    plan.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    plan.insert(QStringLiteral("editorBoundNoteId"), m_editorBoundNoteId);
    plan.insert(QStringLiteral("editorSessionBoundToSelectedNote"), sessionBound);
    plan.insert(QStringLiteral("bodyMatchesSelection"), bodyIdMatches);
    plan.insert(QStringLiteral("bodyAvailable"), bodyAvailable);
    plan.insert(QStringLiteral("editorAvailable"), editorAvailable);
    plan.insert(QStringLiteral("preferEditorSessionSource"), preferEditor);
    plan.insert(QStringLiteral("resolvedSourceText"), preferEditor ? m_editorText : (bodyAvailable ? m_selectedNoteBodyText : QString()));
    plan.insert(QStringLiteral("resolvedSourceReady"), preferEditor || bodyAvailable);
    return plan;
}

QString ContentsDisplayDocumentSourceResolver::resolvedDocumentPresentationSourceText() const
{
    if (editorSessionBoundToSelectedNote())
        return m_editorText;
    if (m_selectedNoteBodyResolved && m_selectedNoteBodyNoteId == m_selectedNoteId)
        return m_selectedNoteBodyText;
    return {};
}

QString ContentsDisplayDocumentSourceResolver::currentMinimapSourceText(const bool structuredHostGeometryActive) const
{
    return structuredHostGeometryActive ? m_structuredFlowSourceText : m_editorText;
}

QVariantMap ContentsDisplayDocumentSourceResolver::normalizedDocumentSourceMutation(const QVariant& nextSourceText) const
{
    QVariantMap result;
    const QString normalizedNextSourceText = nextSourceText.isValid() && !nextSourceText.isNull()
        ? nextSourceText.toString()
        : QString();
    result.insert(QStringLiteral("nextSourceText"), normalizedNextSourceText);
    result.insert(QStringLiteral("currentSourceText"), m_editorText);
    result.insert(QStringLiteral("changed"), normalizedNextSourceText != m_editorText);
    return result;
}

QString ContentsDisplayDocumentSourceResolver::normalizeNoteId(const QString& value)
{
    return value.trimmed();
}

bool ContentsDisplayDocumentSourceResolver::editorSessionBoundToSelectedNote() const noexcept
{
    return !m_editorBoundNoteId.isEmpty() && m_editorBoundNoteId == m_selectedNoteId;
}
