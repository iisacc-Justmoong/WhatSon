#include "app/models/editor/display/ContentsDisplayDocumentSourceResolver.hpp"

#include <QDir>

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
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_selectedNoteId = normalized;
    emit selectedNoteIdChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::selectedNoteDirectoryPath() const
{
    return m_selectedNoteDirectoryPath;
}

void ContentsDisplayDocumentSourceResolver::setSelectedNoteDirectoryPath(const QString& value)
{
    const QString normalized = normalizeNoteDirectoryPath(value);
    if (m_selectedNoteDirectoryPath == normalized)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_selectedNoteDirectoryPath = normalized;
    emit selectedNoteDirectoryPathChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::selectedNoteBodyNoteId() const { return m_selectedNoteBodyNoteId; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyNoteId(const QString& value)
{
    const QString normalized = normalizeNoteId(value);
    if (m_selectedNoteBodyNoteId == normalized)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_selectedNoteBodyNoteId = normalized;
    emit selectedNoteBodyNoteIdChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::selectedNoteBodyText() const { return m_selectedNoteBodyText; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyText(const QString& value)
{
    if (m_selectedNoteBodyText == value)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_selectedNoteBodyText = value;
    emit selectedNoteBodyTextChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

bool ContentsDisplayDocumentSourceResolver::selectedNoteBodyResolved() const noexcept { return m_selectedNoteBodyResolved; }
void ContentsDisplayDocumentSourceResolver::setSelectedNoteBodyResolved(const bool value)
{
    if (m_selectedNoteBodyResolved == value)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_selectedNoteBodyResolved = value;
    emit selectedNoteBodyResolvedChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::editorBoundNoteId() const { return m_editorBoundNoteId; }
void ContentsDisplayDocumentSourceResolver::setEditorBoundNoteId(const QString& value)
{
    const QString normalized = normalizeNoteId(value);
    if (m_editorBoundNoteId == normalized)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_editorBoundNoteId = normalized;
    emit editorBoundNoteIdChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::editorBoundNoteDirectoryPath() const
{
    return m_editorBoundNoteDirectoryPath;
}

void ContentsDisplayDocumentSourceResolver::setEditorBoundNoteDirectoryPath(const QString& value)
{
    const QString normalized = normalizeNoteDirectoryPath(value);
    if (m_editorBoundNoteDirectoryPath == normalized)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_editorBoundNoteDirectoryPath = normalized;
    emit editorBoundNoteDirectoryPathChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QString ContentsDisplayDocumentSourceResolver::editorText() const { return m_editorText; }
void ContentsDisplayDocumentSourceResolver::setEditorText(const QString& value)
{
    if (m_editorText == value)
        return;
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_editorText = value;
    emit editorTextChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
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
    const QVariantMap previousPlan = documentSourcePlan();
    const QString previousPresentationSourceText = documentPresentationSourceText();
    m_pendingBodySave = value;
    emit pendingBodySaveChanged();
    emitDerivedOutputsIfChanged(previousPlan, previousPresentationSourceText);
}

QVariantMap ContentsDisplayDocumentSourceResolver::documentSourcePlan() const
{
    QVariantMap plan;
    const bool sessionBound = editorSessionBoundToSelectedNote();
    const bool bodyIdMatches = !m_selectedNoteId.isEmpty() && m_selectedNoteBodyNoteId == m_selectedNoteId;
    const bool bodyOwnedBySelection =
        !m_selectedNoteId.isEmpty()
        && (bodyIdMatches || m_selectedNoteBodyNoteId.isEmpty());
    const bool bodyHasText = !m_selectedNoteBodyText.isEmpty();
    const bool bodyAvailable = bodyOwnedBySelection
        && (bodyHasText || m_selectedNoteBodyResolved);
    const bool bodyReadyForPresentation =
        m_selectedNoteBodyResolved
        && bodyIdMatches;
    const bool editorHasText = !m_editorText.isEmpty();
    const bool editorCanRepresentEmptyDocument =
        m_pendingBodySave && sessionBound;
    const bool editorAvailable =
        sessionBound
        && (editorHasText || editorCanRepresentEmptyDocument);
    const bool preferEditor = editorAvailable
        && (m_pendingBodySave || !bodyAvailable);
    const bool presentationSourceReady = preferEditor || bodyReadyForPresentation;

    plan.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);
    plan.insert(QStringLiteral("selectedNoteDirectoryPath"), m_selectedNoteDirectoryPath);
    plan.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    plan.insert(QStringLiteral("editorBoundNoteId"), m_editorBoundNoteId);
    plan.insert(QStringLiteral("editorBoundNoteDirectoryPath"), m_editorBoundNoteDirectoryPath);
    plan.insert(QStringLiteral("editorSessionBoundToSelectedNote"), sessionBound);
    plan.insert(QStringLiteral("bodyMatchesSelection"), bodyIdMatches);
    plan.insert(QStringLiteral("bodyAvailable"), bodyAvailable);
    plan.insert(QStringLiteral("bodyReadyForPresentation"), bodyReadyForPresentation);
    plan.insert(QStringLiteral("editorAvailable"), editorAvailable);
    plan.insert(QStringLiteral("preferEditorSessionSource"), preferEditor);
    plan.insert(
        QStringLiteral("resolvedSourceText"),
        preferEditor ? m_editorText : (bodyReadyForPresentation ? m_selectedNoteBodyText : QString()));
    plan.insert(QStringLiteral("resolvedSourceReady"), presentationSourceReady);
    return plan;
}

QString ContentsDisplayDocumentSourceResolver::documentPresentationSourceText() const
{
    const QVariantMap plan = documentSourcePlan();
    if (plan.value(QStringLiteral("resolvedSourceReady")).toBool())
        return plan.value(QStringLiteral("resolvedSourceText")).toString();

    return {};
}

QVariantMap ContentsDisplayDocumentSourceResolver::resolveDocumentSourcePlan() const
{
    return documentSourcePlan();
}

QString ContentsDisplayDocumentSourceResolver::resolvedDocumentPresentationSourceText() const
{
    return documentPresentationSourceText();
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

QString ContentsDisplayDocumentSourceResolver::normalizeNoteDirectoryPath(const QString& value)
{
    const QString normalized = QDir::cleanPath(value.trimmed());
    return normalized == QStringLiteral(".") ? QString() : normalized;
}

bool ContentsDisplayDocumentSourceResolver::editorSessionBoundToSelectedNote() const noexcept
{
    if (m_editorBoundNoteId.isEmpty() || m_editorBoundNoteId != m_selectedNoteId)
    {
        return false;
    }

    if (m_selectedNoteDirectoryPath.isEmpty())
    {
        return true;
    }

    return m_editorBoundNoteDirectoryPath == m_selectedNoteDirectoryPath;
}

void ContentsDisplayDocumentSourceResolver::emitDerivedOutputsIfChanged(
    const QVariantMap& previousPlan,
    const QString& previousPresentationSourceText)
{
    const QVariantMap nextPlan = documentSourcePlan();
    if (nextPlan != previousPlan)
        emit documentSourcePlanChanged();

    const QString nextPresentationSourceText = documentPresentationSourceText();
    if (nextPresentationSourceText != previousPresentationSourceText)
        emit documentPresentationSourceTextChanged();
}
