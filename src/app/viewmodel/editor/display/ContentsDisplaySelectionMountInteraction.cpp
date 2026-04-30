#include "app/viewmodel/editor/display/ContentsDisplaySelectionMountInteraction.hpp"

#include <QMetaObject>
#include <QTimer>

namespace
{
    QVariantMap safeMap(const QVariant& value)
    {
        return value.toMap();
    }
}

ContentsDisplaySelectionMountInteraction::ContentsDisplaySelectionMountInteraction(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplaySelectionMountInteraction::~ContentsDisplaySelectionMountInteraction() = default;

QObject* ContentsDisplaySelectionMountInteraction::contentsView() const noexcept { return m_contentsView.data(); }
void ContentsDisplaySelectionMountInteraction::setContentsView(QObject* value)
{
    if (m_contentsView == value)
        return;
    m_contentsView = value;
    emit contentsViewChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::editorSelectionController() const noexcept { return m_editorSelectionController.data(); }
void ContentsDisplaySelectionMountInteraction::setEditorSelectionController(QObject* value)
{
    if (m_editorSelectionController == value)
        return;
    m_editorSelectionController = value;
    emit editorSelectionControllerChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::editorSession() const noexcept { return m_editorSession.data(); }
void ContentsDisplaySelectionMountInteraction::setEditorSession(QObject* value)
{
    if (m_editorSession == value)
        return;
    m_editorSession = value;
    emit editorSessionChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::editorTypingController() const noexcept { return m_editorTypingController.data(); }
void ContentsDisplaySelectionMountInteraction::setEditorTypingController(QObject* value)
{
    if (m_editorTypingController == value)
        return;
    m_editorTypingController = value;
    emit editorTypingControllerChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::noteBodyMountCoordinator() const noexcept { return m_noteBodyMountCoordinator.data(); }
void ContentsDisplaySelectionMountInteraction::setNoteBodyMountCoordinator(QObject* value)
{
    if (m_noteBodyMountCoordinator == value)
        return;
    m_noteBodyMountCoordinator = value;
    emit noteBodyMountCoordinatorChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::selectionBridge() const noexcept { return m_selectionBridge.data(); }
void ContentsDisplaySelectionMountInteraction::setSelectionBridge(QObject* value)
{
    if (m_selectionBridge == value)
        return;
    m_selectionBridge = value;
    emit selectionBridgeChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::selectionSyncCoordinator() const noexcept { return m_selectionSyncCoordinator.data(); }
void ContentsDisplaySelectionMountInteraction::setSelectionSyncCoordinator(QObject* value)
{
    if (m_selectionSyncCoordinator == value)
        return;
    m_selectionSyncCoordinator = value;
    emit selectionSyncCoordinatorChanged();
}

QObject* ContentsDisplaySelectionMountInteraction::structuredDocumentFlow() const noexcept { return m_structuredDocumentFlow.data(); }
void ContentsDisplaySelectionMountInteraction::setStructuredDocumentFlow(QObject* value)
{
    if (m_structuredDocumentFlow == value)
        return;
    m_structuredDocumentFlow = value;
    emit structuredDocumentFlowChanged();
}

bool ContentsDisplaySelectionMountInteraction::shouldFlushBlurredEditorState(const QString& scheduledNoteId) const
{
    const QString normalizedScheduledNoteId = scheduledNoteId.trimmed();
    if (normalizedScheduledNoteId.isEmpty() || m_editorSession == nullptr || m_contentsView == nullptr)
        return false;
    if (editorBoundNoteId() != normalizedScheduledNoteId || selectedNoteId() != normalizedScheduledNoteId)
        return false;
    return property(m_editorSession, "localEditorAuthority").toBool()
        || property(m_editorSession, "pendingBodySave").toBool();
}

void ContentsDisplaySelectionMountInteraction::flushEditorStateAfterInputSettles(const QString& scheduledNoteId) const
{
    const QString normalizedScheduledNoteId = scheduledNoteId.trimmed();
    if (!shouldFlushBlurredEditorState(normalizedScheduledNoteId))
        return;
    if (invokeBool(m_contentsView, "nativeEditorCompositionActive"))
        return;
    if (!normalizedScheduledNoteId.isEmpty()
        && (editorBoundNoteId() != normalizedScheduledNoteId || selectedNoteId() != normalizedScheduledNoteId))
        return;
    invokeBool(m_editorTypingController, "handleEditorTextEdited");
    invokeBool(m_editorSession, "flushPendingEditorText");
}

void ContentsDisplaySelectionMountInteraction::focusEditorForSelectedNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || !property(m_contentsView, "hasSelectedNote").toBool())
        return;
    QPointer<QObject> view(m_contentsView);
    QPointer<QObject> structuredFlow(m_structuredDocumentFlow);
    QTimer::singleShot(0, this, [this, normalizedNoteId, view, structuredFlow]() {
        if (view == nullptr || structuredFlow == nullptr)
            return;
        if (normalizedTrimmedTextValue(property(view, "selectedNoteId")) != normalizedNoteId)
            return;
        focusStructuredDocumentAtNoteEnd(normalizedNoteId);
    });
}

void ContentsDisplaySelectionMountInteraction::focusEditorForPendingNote() const
{
    const QString pendingNoteId = invoke(m_selectionSyncCoordinator, "takePendingEditorFocusNoteId", {selectedNoteId()}).toString().trimmed();
    if (pendingNoteId.isEmpty())
        return;
    focusEditorForSelectedNoteId(pendingNoteId);
}

void ContentsDisplaySelectionMountInteraction::scheduleEditorEntrySnapshotReconcile() const
{
    invokeVoid(m_selectionSyncCoordinator, "scheduleSnapshotReconcile");
}

void ContentsDisplaySelectionMountInteraction::pollSelectedNoteSnapshot() const
{
    const QVariantMap pollPlan = safeMap(invoke(m_selectionSyncCoordinator, "snapshotPollPlan"));
    const QString noteId = normalizedTrimmedTextValue(pollPlan.value(QStringLiteral("noteId")));
    if (pollPlan.value(QStringLiteral("attemptReconcile")).toBool() && requestSelectionSnapshotReconcile(noteId))
        return;
    if (!pollPlan.value(QStringLiteral("allowSnapshotRefresh")).toBool() || m_selectionBridge == nullptr)
        return;
    if (invokeBool(m_selectionBridge, "refreshSelectedNoteSnapshot"))
        invokeVoid(m_contentsView, "scheduleGutterRefresh", {2});
}

bool ContentsDisplaySelectionMountInteraction::reconcileEditorEntrySnapshotOnce() const
{
    const QVariantMap reconcilePlan = safeMap(invoke(m_selectionSyncCoordinator, "snapshotReconcilePlan"));
    const QString noteId = normalizedTrimmedTextValue(reconcilePlan.value(QStringLiteral("noteId")));
    if (!reconcilePlan.value(QStringLiteral("attemptReconcile")).toBool())
        return false;
    return requestSelectionSnapshotReconcile(noteId);
}

void ContentsDisplaySelectionMountInteraction::scheduleSelectionModelSync(const QVariant& options) const
{
    const QVariantMap normalizedOptions = safeMap(options);
    invokeVoid(m_selectionSyncCoordinator, "scheduleSelectionSync", {normalizedOptions});
    invokeVoid(m_noteBodyMountCoordinator, "scheduleMount", {normalizedOptions});
}

bool ContentsDisplaySelectionMountInteraction::executeSelectionDeliveryPlan(const QVariant& plan, const QString& fallbackKey) const
{
    const QVariantMap normalizedPlan = safeMap(plan);
    if (normalizedPlan.value(QStringLiteral("resetSelectionCache")).toBool())
        invokeVoid(m_contentsView, "resetEditorSelectionCache");
    if (normalizedPlan.value(QStringLiteral("flushPendingEditorText")).toBool())
        invokeBool(m_editorSession, "flushPendingEditorText");

    bool selectionSynced = false;
    if ((normalizedPlan.value(QStringLiteral("attemptSelectionSync")).toBool()
         || normalizedPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool())
        && m_editorSession != nullptr)
    {
        selectionSynced = invokeBool(
            m_editorSession,
            "requestSyncEditorTextFromSelection",
            {normalizedTextValue(normalizedPlan.value(QStringLiteral("selectedNoteId"))),
             normalizedTextValue(normalizedPlan.value(QStringLiteral("selectedNoteBodyText"))),
             normalizedTextValue(normalizedPlan.value(QStringLiteral("selectedNoteBodyNoteId"))),
             selectedNoteDirectoryPath()});
    }

    if (normalizedPlan.value(QStringLiteral("scheduleSnapshotReconcile")).toBool())
        scheduleEditorEntrySnapshotReconcile();

    if (normalizedPlan.value(QStringLiteral("forceVisualRefresh")).toBool()
        || (!selectionSynced && normalizedPlan.value(fallbackKey).toBool()))
    {
        invokeVoid(m_contentsView, "scheduleMinimapSnapshotRefresh", {true});
        invokeVoid(m_contentsView, "scheduleDocumentPresentationRefresh", {true});
        invokeVoid(m_contentsView, "scheduleGutterRefresh", {4});
    }

    if (normalizedPlan.value(QStringLiteral("focusEditorForSelectedNote")).toBool())
        invokeVoid(
            m_selectionSyncCoordinator,
            "scheduleEditorFocusForNote",
            {normalizedTextValue(normalizedPlan.value(QStringLiteral("selectedNoteId")))});

    return selectionSynced;
}

void ContentsDisplaySelectionMountInteraction::scheduleEditorFocusForNote(const QString& noteId) const
{
    invokeVoid(m_selectionSyncCoordinator, "scheduleEditorFocusForNote", {noteId});
}

void ContentsDisplaySelectionMountInteraction::resetEditorSelectionCache() const
{
    invokeVoid(m_editorSelectionController, "resetEditorSelectionCache");
}

QVariant ContentsDisplaySelectionMountInteraction::invoke(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    if (target == nullptr || methodName == nullptr)
        return {};
    QVariant result;
    switch (arguments.size())
    {
    case 0:
        QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, result));
        break;
    case 1:
        QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arguments.at(0)));
        break;
    case 2:
        QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arguments.at(0)), Q_ARG(QVariant, arguments.at(1)));
        break;
    case 3:
        QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arguments.at(0)), Q_ARG(QVariant, arguments.at(1)), Q_ARG(QVariant, arguments.at(2)));
        break;
    case 4:
        QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arguments.at(0)), Q_ARG(QVariant, arguments.at(1)), Q_ARG(QVariant, arguments.at(2)), Q_ARG(QVariant, arguments.at(3)));
        break;
    default:
        break;
    }
    return result;
}

bool ContentsDisplaySelectionMountInteraction::invokeBool(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    return invoke(target, methodName, arguments).toBool();
}

void ContentsDisplaySelectionMountInteraction::invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    Q_UNUSED(invoke(target, methodName, arguments));
}

QVariant ContentsDisplaySelectionMountInteraction::property(QObject* target, const char* propertyName) const
{
    return target == nullptr || propertyName == nullptr ? QVariant() : target->property(propertyName);
}

QString ContentsDisplaySelectionMountInteraction::normalizedTextValue(const QVariant& value) const
{
    return value.isValid() && !value.isNull() ? value.toString() : QString();
}

QString ContentsDisplaySelectionMountInteraction::normalizedTrimmedTextValue(const QVariant& value) const
{
    return normalizedTextValue(value).trimmed();
}

QString ContentsDisplaySelectionMountInteraction::selectedNoteId() const
{
    return normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteId"));
}

QString ContentsDisplaySelectionMountInteraction::editorBoundNoteId() const
{
    return normalizedTrimmedTextValue(property(m_editorSession, "editorBoundNoteId"));
}

QString ContentsDisplaySelectionMountInteraction::selectedNoteDirectoryPath() const
{
    return normalizedTextValue(property(m_contentsView, "selectedNoteDirectoryPath"));
}

QString ContentsDisplaySelectionMountInteraction::sessionEditorText() const
{
    return normalizedTextValue(property(m_editorSession, "editorText"));
}

bool ContentsDisplaySelectionMountInteraction::requestSelectionSnapshotReconcile(const QString& noteId) const
{
    if (noteId.isEmpty() || m_selectionBridge == nullptr)
        return false;
    const bool reconcileAccepted = invokeBool(
        m_selectionBridge,
        "reconcileViewSessionAndRefreshSnapshotForNote",
        {noteId, sessionEditorText(), property(m_editorSession, "localEditorAuthority").toBool()});
    if (reconcileAccepted)
        invokeVoid(m_selectionSyncCoordinator, "markSnapshotReconcileStarted", {noteId});
    return reconcileAccepted;
}

void ContentsDisplaySelectionMountInteraction::focusStructuredDocumentAtNoteEnd(const QString& noteId) const
{
    Q_UNUSED(noteId);
    if (m_structuredDocumentFlow == nullptr)
        return;
    QVariantMap focusRequest;
    const int logicalCursorPosition = qMax(0, property(m_contentsView, "resolvedLogicalTextLength").toInt());
    focusRequest.insert(QStringLiteral("cursorPosition"), logicalCursorPosition);
    focusRequest.insert(QStringLiteral("logicalCursorPosition"), logicalCursorPosition);
    focusRequest.insert(QStringLiteral("sourceOffset"), normalizedTextValue(property(m_contentsView, "documentPresentationSourceText")).size());
    invokeVoid(m_structuredDocumentFlow, "requestFocus", {focusRequest});
}
