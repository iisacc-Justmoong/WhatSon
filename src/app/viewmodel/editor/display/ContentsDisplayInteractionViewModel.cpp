#include "app/viewmodel/editor/display/ContentsDisplayInteractionViewModel.hpp"

#include <QMetaObject>
#include <QTimer>
#include <Qt>

namespace
{
    constexpr int keyB = 0x42;
    constexpr int keyC = 0x43;
    constexpr int keyE = 0x45;
    constexpr int keyH = 0x48;
    constexpr int keyI = 0x49;
    constexpr int keyInsert = 0x01000006;
    constexpr int keyT = 0x54;
    constexpr int keyU = 0x55;
    constexpr int keyV = 0x56;
    constexpr int keyX = 0x58;

    QVariantMap safeMap(const QVariant& value)
    {
        return value.toMap();
    }
}

ContentsDisplayInteractionViewModel::ContentsDisplayInteractionViewModel(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayInteractionViewModel::~ContentsDisplayInteractionViewModel() = default;

QObject* ContentsDisplayInteractionViewModel::contentsView() const noexcept { return m_contentsView.data(); }
void ContentsDisplayInteractionViewModel::setContentsView(QObject* value)
{
    if (m_contentsView == value)
        return;
    m_contentsView = value;
    emit contentsViewChanged();
}

QObject* ContentsDisplayInteractionViewModel::contentsAgendaBackend() const noexcept { return m_contentsAgendaBackend.data(); }
void ContentsDisplayInteractionViewModel::setContentsAgendaBackend(QObject* value)
{
    if (m_contentsAgendaBackend == value)
        return;
    m_contentsAgendaBackend = value;
    emit contentsAgendaBackendChanged();
}

QObject* ContentsDisplayInteractionViewModel::contextMenuCoordinator() const noexcept { return m_contextMenuCoordinator.data(); }
void ContentsDisplayInteractionViewModel::setContextMenuCoordinator(QObject* value)
{
    if (m_contextMenuCoordinator == value)
        return;
    m_contextMenuCoordinator = value;
    emit contextMenuCoordinatorChanged();
}

QObject* ContentsDisplayInteractionViewModel::editOperationCoordinator() const noexcept { return m_editOperationCoordinator.data(); }
void ContentsDisplayInteractionViewModel::setEditOperationCoordinator(QObject* value)
{
    if (m_editOperationCoordinator == value)
        return;
    m_editOperationCoordinator = value;
    emit editOperationCoordinatorChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorInputPolicyAdapter() const noexcept { return m_editorInputPolicyAdapter.data(); }
void ContentsDisplayInteractionViewModel::setEditorInputPolicyAdapter(QObject* value)
{
    if (m_editorInputPolicyAdapter == value)
        return;
    m_editorInputPolicyAdapter = value;
    emit editorInputPolicyAdapterChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorProjection() const noexcept { return m_editorProjection.data(); }
void ContentsDisplayInteractionViewModel::setEditorProjection(QObject* value)
{
    if (m_editorProjection == value)
        return;
    m_editorProjection = value;
    emit editorProjectionChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorSelectionController() const noexcept { return m_editorSelectionController.data(); }
void ContentsDisplayInteractionViewModel::setEditorSelectionController(QObject* value)
{
    if (m_editorSelectionController == value)
        return;
    m_editorSelectionController = value;
    emit editorSelectionControllerChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorSession() const noexcept { return m_editorSession.data(); }
void ContentsDisplayInteractionViewModel::setEditorSession(QObject* value)
{
    if (m_editorSession == value)
        return;
    m_editorSession = value;
    emit editorSessionChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorTypingController() const noexcept { return m_editorTypingController.data(); }
void ContentsDisplayInteractionViewModel::setEditorTypingController(QObject* value)
{
    if (m_editorTypingController == value)
        return;
    m_editorTypingController = value;
    emit editorTypingControllerChanged();
}

QObject* ContentsDisplayInteractionViewModel::editorViewport() const noexcept { return m_editorViewport.data(); }
void ContentsDisplayInteractionViewModel::setEditorViewport(QObject* value)
{
    if (m_editorViewport == value)
        return;
    m_editorViewport = value;
    emit editorViewportChanged();
}

QObject* ContentsDisplayInteractionViewModel::eventPump() const noexcept { return m_eventPump.data(); }
void ContentsDisplayInteractionViewModel::setEventPump(QObject* value)
{
    if (m_eventPump == value)
        return;
    m_eventPump = value;
    emit eventPumpChanged();
}

QObject* ContentsDisplayInteractionViewModel::minimapLayer() const noexcept { return m_minimapLayer.data(); }
void ContentsDisplayInteractionViewModel::setMinimapLayer(QObject* value)
{
    if (m_minimapLayer == value)
        return;
    m_minimapLayer = value;
    emit minimapLayerChanged();
}

QObject* ContentsDisplayInteractionViewModel::mutationViewModel() const noexcept { return m_mutationViewModel.data(); }
void ContentsDisplayInteractionViewModel::setMutationViewModel(QObject* value)
{
    if (m_mutationViewModel == value)
        return;
    m_mutationViewModel = value;
    emit mutationViewModelChanged();
}

QObject* ContentsDisplayInteractionViewModel::panelViewModel() const noexcept { return m_panelViewModel.data(); }
void ContentsDisplayInteractionViewModel::setPanelViewModel(QObject* value)
{
    if (m_panelViewModel == value)
        return;
    m_panelViewModel = value;
    emit panelViewModelChanged();
}

QObject* ContentsDisplayInteractionViewModel::presentationViewModel() const noexcept { return m_presentationViewModel.data(); }
void ContentsDisplayInteractionViewModel::setPresentationViewModel(QObject* value)
{
    if (m_presentationViewModel == value)
        return;
    m_presentationViewModel = value;
    emit presentationViewModelChanged();
}

QObject* ContentsDisplayInteractionViewModel::presentationRefreshController() const noexcept { return m_presentationRefreshController.data(); }
void ContentsDisplayInteractionViewModel::setPresentationRefreshController(QObject* value)
{
    if (m_presentationRefreshController == value)
        return;
    m_presentationRefreshController = value;
    emit presentationRefreshControllerChanged();
}

QObject* ContentsDisplayInteractionViewModel::resourceImportController() const noexcept { return m_resourceImportController.data(); }
void ContentsDisplayInteractionViewModel::setResourceImportController(QObject* value)
{
    if (m_resourceImportController == value)
        return;
    m_resourceImportController = value;
    emit resourceImportControllerChanged();
}

QObject* ContentsDisplayInteractionViewModel::selectionMountViewModel() const noexcept { return m_selectionMountViewModel.data(); }
void ContentsDisplayInteractionViewModel::setSelectionMountViewModel(QObject* value)
{
    if (m_selectionMountViewModel == value)
        return;
    m_selectionMountViewModel = value;
    emit selectionMountViewModelChanged();
}

QObject* ContentsDisplayInteractionViewModel::structuredDocumentFlow() const noexcept { return m_structuredDocumentFlow.data(); }
void ContentsDisplayInteractionViewModel::setStructuredDocumentFlow(QObject* value)
{
    if (m_structuredDocumentFlow == value)
        return;
    m_structuredDocumentFlow = value;
    emit structuredDocumentFlowChanged();
}

void ContentsDisplayInteractionViewModel::logEditorCreationState(const QString& reason) const
{
    invokeVoid(m_presentationViewModel, "logEditorCreationState", {reason});
}

bool ContentsDisplayInteractionViewModel::shouldFlushBlurredEditorState(const QString& scheduledNoteId) const
{
    return invokeBool(m_selectionMountViewModel, "shouldFlushBlurredEditorState", {scheduledNoteId});
}

bool ContentsDisplayInteractionViewModel::nativeEditorCompositionActive() const
{
    return invokeBool(m_structuredDocumentFlow, "nativeCompositionActive");
}

bool ContentsDisplayInteractionViewModel::nativeTextInputSessionOwnsKeyboard() const
{
    return property(m_editorInputPolicyAdapter, "nativeTextInputSessionActive").toBool();
}

void ContentsDisplayInteractionViewModel::flushEditorStateAfterInputSettles(const QString& scheduledNoteId) const
{
    invokeVoid(m_selectionMountViewModel, "flushEditorStateAfterInputSettles", {scheduledNoteId});
}

void ContentsDisplayInteractionViewModel::focusEditorForSelectedNoteId(const QString& noteId) const
{
    invokeVoid(m_selectionMountViewModel, "focusEditorForSelectedNoteId", {noteId});
}

void ContentsDisplayInteractionViewModel::focusEditorForPendingNote() const
{
    invokeVoid(m_selectionMountViewModel, "focusEditorForPendingNote");
}

bool ContentsDisplayInteractionViewModel::eventRequestsPasteShortcut(const QJSValue& event) const
{
    const int modifiers = eventModifiers(event);
    const bool metaPressed = (modifiers & Qt::MetaModifier) != 0;
    const bool controlPressed = (modifiers & Qt::ControlModifier) != 0;
    const bool altPressed = (modifiers & Qt::AltModifier) != 0;
    const bool shiftPressed = (modifiers & Qt::ShiftModifier) != 0;
    const int key = eventKey(event);
    const QString normalizedText = eventText(event).toUpper();
    if (!altPressed && !shiftPressed && (metaPressed || controlPressed)
        && (key == keyV || normalizedText == QStringLiteral("V")))
        return true;
    if (!metaPressed && !controlPressed && !altPressed && shiftPressed && key == keyInsert)
        return true;
    return false;
}

QString ContentsDisplayInteractionViewModel::inlineFormatShortcutTag(const QJSValue& event) const
{
    const int modifiers = eventModifiers(event);
    const bool metaPressed = (modifiers & Qt::MetaModifier) != 0;
    const bool controlPressed = (modifiers & Qt::ControlModifier) != 0;
    const bool altPressed = (modifiers & Qt::AltModifier) != 0;
    const bool shiftPressed = (modifiers & Qt::ShiftModifier) != 0;
    if (altPressed || (!metaPressed && !controlPressed))
        return {};
    const int key = eventKey(event);
    const QString normalizedText = eventText(event).toUpper();
    if (!shiftPressed)
    {
        if (key == keyB || normalizedText == QStringLiteral("B"))
            return QStringLiteral("bold");
        if (key == keyI || normalizedText == QStringLiteral("I"))
            return QStringLiteral("italic");
        if (key == keyU || normalizedText == QStringLiteral("U"))
            return QStringLiteral("underline");
        return {};
    }
    if (key == keyX || normalizedText == QStringLiteral("X"))
        return QStringLiteral("strikethrough");
    if (key == keyE || normalizedText == QStringLiteral("E"))
        return QStringLiteral("highlight");
    return {};
}

bool ContentsDisplayInteractionViewModel::handleInlineFormatTagShortcut(const QJSValue& event) const
{
    const QString tagName = inlineFormatShortcutTag(event);
    if (tagName.isEmpty())
        return false;
    const bool handled = queueInlineFormatWrap(tagName);
    if (handled)
        setEventAccepted(event);
    return handled;
}

bool ContentsDisplayInteractionViewModel::clipboardImageAvailableForPaste() const
{
    const QVariant resourcesImportViewModel = property(m_contentsView, "resourcesImportViewModel");
    auto* importVm = qobject_cast<QObject*>(resourcesImportViewModel.value<QObject*>());
    if (importVm == nullptr)
        return false;
    const int methodIndex = importVm->metaObject()->indexOfMethod("refreshClipboardImageAvailabilitySnapshot()");
    if (methodIndex >= 0)
        return invokeBool(importVm, "refreshClipboardImageAvailabilitySnapshot");
    return importVm->property("clipboardImageAvailable").toBool();
}

bool ContentsDisplayInteractionViewModel::handleClipboardImagePasteShortcut(const QJSValue& event) const
{
    if (!eventRequestsPasteShortcut(event))
        return false;
    const QVariant resourcesImportViewModel = property(m_contentsView, "resourcesImportViewModel");
    auto* importVm = qobject_cast<QObject*>(resourcesImportViewModel.value<QObject*>());
    if (importVm == nullptr || importVm->property("busy").toBool() || !clipboardImageAvailableForPaste())
        return false;
    const bool pasted = invokeBool(m_resourceImportController, "pasteClipboardImageAsResource");
    if (pasted)
        setEventAccepted(event);
    return pasted;
}

bool ContentsDisplayInteractionViewModel::handleTagManagementShortcutKeyPress(const QJSValue& event) const
{
    if (handleClipboardImagePasteShortcut(event))
        return true;
    if (handleInlineFormatTagShortcut(event))
        return true;
    const QString tagKind = bodyTagShortcutKind(event);
    bool handled = false;
    if (tagKind == QStringLiteral("agenda"))
        handled = queueAgendaShortcutInsertion();
    else if (tagKind == QStringLiteral("callout"))
        handled = queueCalloutShortcutInsertion();
    else if (tagKind == QStringLiteral("break"))
        handled = queueBreakShortcutInsertion();
    if (handled)
        setEventAccepted(event);
    return handled;
}

bool ContentsDisplayInteractionViewModel::handleSelectionContextMenuEvent(const QString& eventName) const
{
    if (handleStructuredSelectionContextMenuEvent(eventName))
        return true;
    invokeVoid(m_editorSelectionController, "handleSelectionContextMenuEvent", {eventName});
    return true;
}

void ContentsDisplayInteractionViewModel::commitDocumentPresentationRefresh() const
{
    const bool needsHtmlProjection = property(m_contentsView, "documentPresentationProjectionEnabled").toBool();
    if (!needsHtmlProjection)
    {
        if (!property(m_contentsView, "renderedEditorHtml").toString().isEmpty())
            setProperty(m_contentsView, "renderedEditorHtml", QString());
        invokeVoid(m_contentsView, "scheduleMinimapSnapshotRefresh", {false});
        if (m_minimapLayer != nullptr && property(m_contentsView, "minimapRefreshEnabled").toBool())
            invokeVoid(m_minimapLayer, "requestRepaint");
        invokeVoid(m_editorTypingController, "synchronizeLiveEditingStateFromPresentation");
        return;
    }

    const QString nextRenderedText = property(m_editorProjection, "editorSurfaceHtml").toString();
    const bool renderInlineResources = property(m_contentsView, "resourceBlocksRenderedInlineByHtmlProjection").toBool();
    const QString editorRenderedText = renderInlineResources
        ? invoke(m_resourceImportController, "renderEditorSurfaceHtmlWithInlineResources", {nextRenderedText}).toString()
        : nextRenderedText;
    if (property(m_contentsView, "renderedEditorHtml").toString() != editorRenderedText)
        setProperty(m_contentsView, "renderedEditorHtml", editorRenderedText);
    invokeVoid(m_contentsView, "scheduleMinimapSnapshotRefresh", {false});
    if (m_minimapLayer != nullptr && property(m_contentsView, "minimapRefreshEnabled").toBool())
        invokeVoid(m_minimapLayer, "requestRepaint");
    invokeVoid(m_editorTypingController, "synchronizeLiveEditingStateFromPresentation");
}

bool ContentsDisplayInteractionViewModel::documentPresentationRenderDirty() const
{
    const bool needsHtmlProjection = property(m_contentsView, "documentPresentationProjectionEnabled").toBool();
    if (!needsHtmlProjection)
        return !property(m_contentsView, "renderedEditorHtml").toString().isEmpty();
    const QString rendererRenderedText = property(m_editorProjection, "editorSurfaceHtml").toString();
    const bool renderInlineResources = property(m_contentsView, "resourceBlocksRenderedInlineByHtmlProjection").toBool();
    const QString expectedRenderedText = renderInlineResources
        ? invoke(m_resourceImportController, "renderEditorSurfaceHtmlWithInlineResources", {rendererRenderedText}).toString()
        : rendererRenderedText;
    return property(m_contentsView, "renderedEditorHtml").toString() != expectedRenderedText;
}

void ContentsDisplayInteractionViewModel::refreshInlineResourcePresentation() const
{
    commitDocumentPresentationRefresh();
}

void ContentsDisplayInteractionViewModel::requestViewHook(const QString& reason) const
{
    const QString hookReason = reason.isNull() ? QStringLiteral("manual") : reason;
    if (m_panelViewModel != nullptr)
        invokeVoid(m_panelViewModel, "requestViewModelHook", {hookReason});
    invokeVoid(m_contentsView, "viewHookRequested");
}

QString ContentsDisplayInteractionViewModel::encodeXmlAttributeValue(const QVariant& value) const
{
    QString encodedValue = value.toString();
    encodedValue.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
    encodedValue.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
    encodedValue.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
    encodedValue.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
    encodedValue.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
    return encodedValue;
}

void ContentsDisplayInteractionViewModel::resetStructuredSelectionContextMenuSnapshot() const
{
    setProperty(m_contentsView, "structuredContextMenuBlockIndex", -1);
    setProperty(m_contentsView, "structuredContextMenuSelectionSnapshot", QVariantMap());
    setProperty(m_contextMenuCoordinator, "structuredContextMenuBlockIndex", -1);
    setProperty(m_contextMenuCoordinator, "structuredContextMenuSelectionSnapshot", QVariantMap());
}

bool ContentsDisplayInteractionViewModel::primeStructuredSelectionContextMenuSnapshot() const
{
    if (!property(m_contentsView, "showStructuredDocumentFlow").toBool() || m_structuredDocumentFlow == nullptr)
    {
        resetStructuredSelectionContextMenuSnapshot();
        return false;
    }
    const QVariant targetState = invoke(m_structuredDocumentFlow, "inlineFormatTargetState");
    const QVariant plan = invoke(
        m_contextMenuCoordinator,
        "primeStructuredSelectionSnapshotPlan",
        {targetState.isValid() ? targetState : QVariant(QVariantMap())});
    const QVariantMap planMap = safeMap(plan);
    if (!planMap.value(QStringLiteral("accepted")).toBool())
    {
        resetStructuredSelectionContextMenuSnapshot();
        return false;
    }
    const int blockIndex = planMap.value(QStringLiteral("blockIndex")).toInt();
    const QVariantMap selectionSnapshot = planMap.value(QStringLiteral("selectionSnapshot")).toMap();
    setProperty(m_contentsView, "structuredContextMenuBlockIndex", blockIndex);
    setProperty(m_contentsView, "structuredContextMenuSelectionSnapshot", selectionSnapshot);
    setProperty(m_contextMenuCoordinator, "structuredContextMenuBlockIndex", blockIndex);
    setProperty(m_contextMenuCoordinator, "structuredContextMenuSelectionSnapshot", selectionSnapshot);
    return true;
}

bool ContentsDisplayInteractionViewModel::handleStructuredSelectionContextMenuEvent(const QString& eventName) const
{
    const QString inlineStyleTag = invoke(m_contextMenuCoordinator, "inlineStyleTagForEvent", {eventName}).toString();
    const bool canApply = m_structuredDocumentFlow != nullptr
        && m_structuredDocumentFlow->metaObject()->indexOfMethod(
               "applyInlineFormatToBlockSelection(QVariant,QVariant,QVariantMap)") >= 0;
    const QVariant plan = invoke(
        m_contextMenuCoordinator,
        "handleStructuredSelectionEventPlan",
        {inlineStyleTag, invokeBool(m_contextMenuCoordinator, "structuredSelectionValid"), canApply});
    const QVariantMap planMap = safeMap(plan);
    if (!planMap.value(QStringLiteral("applyStructuredInlineFormat")).toBool())
    {
        if (planMap.value(QStringLiteral("requireStructuredSelectionPrime")).toBool()
            && primeStructuredSelectionContextMenuSnapshot())
            return handleStructuredSelectionContextMenuEvent(eventName);
        return false;
    }
    const bool handled = invokeBool(
        m_structuredDocumentFlow,
        "applyInlineFormatToBlockSelection",
        {planMap.value(QStringLiteral("blockIndex")).toInt(),
         planMap.value(QStringLiteral("inlineStyleTag")).toString(),
         planMap.value(QStringLiteral("selectionSnapshot")).toMap()});
    resetStructuredSelectionContextMenuSnapshot();
    return handled;
}

bool ContentsDisplayInteractionViewModel::openEditorSelectionContextMenu(const qreal localX, const qreal localY) const
{
    auto* inputCommandSurface = qobject_cast<QObject*>(property(m_contentsView, "inputCommandSurface").value<QObject*>());
    auto* selectionContextMenu = inputCommandSurface == nullptr
        ? nullptr
        : qobject_cast<QObject*>(inputCommandSurface->property("selectionContextMenu").value<QObject*>());
    const QVariant plan = invoke(
        m_contextMenuCoordinator,
        "openSelectionContextMenuPlan",
        {invokeBool(m_contextMenuCoordinator, "structuredSelectionValid"), selectionContextMenu != nullptr, localX, localY});
    const QVariantMap planMap = safeMap(plan);
    if (planMap.value(QStringLiteral("delegateToEditorSelectionController")).toBool())
        return invokeBool(m_editorSelectionController, "openEditorSelectionContextMenu", {localX, localY});
    if (planMap.value(QStringLiteral("requireStructuredSelectionPrime")).toBool()
        && !primeStructuredSelectionContextMenuSnapshot())
        return false;
    if (selectionContextMenu == nullptr)
        return false;
    if (planMap.value(QStringLiteral("closeBeforeOpen")).toBool())
        invokeVoid(selectionContextMenu, "close");
    invokeVoid(selectionContextMenu, "openFor", {QVariant::fromValue(m_editorViewport.data()), planMap.value(QStringLiteral("openX")), planMap.value(QStringLiteral("openY"))});
    return true;
}

bool ContentsDisplayInteractionViewModel::editorContextMenuPointerTriggerAccepted(const QString& triggerKind) const
{
    const QString normalizedTrigger = triggerKind.trimmed().toLower();
    if (normalizedTrigger == QStringLiteral("rightclick")
        || normalizedTrigger == QStringLiteral("right-click")
        || normalizedTrigger == QStringLiteral("contextmenu")
        || normalizedTrigger == QStringLiteral("context-menu"))
        return true;
    if (normalizedTrigger == QStringLiteral("longpress")
        || normalizedTrigger == QStringLiteral("long-press")
        || normalizedTrigger == QStringLiteral("pressandhold")
        || normalizedTrigger == QStringLiteral("press-and-hold"))
        return property(m_contentsView, "contextMenuLongPressEnabled").toBool();
    return false;
}

bool ContentsDisplayInteractionViewModel::requestEditorSelectionContextMenuFromPointer(
    const qreal localX,
    const qreal localY,
    const QString& triggerKind) const
{
    if (!editorContextMenuPointerTriggerAccepted(triggerKind) || !ensureEditorSelectionContextMenuSnapshot())
        return false;
    return openEditorSelectionContextMenu(localX, localY);
}

bool ContentsDisplayInteractionViewModel::editorSelectionContextMenuSnapshotValid() const
{
    if (property(m_contentsView, "showStructuredDocumentFlow").toBool())
        return invokeBool(m_contextMenuCoordinator, "structuredSelectionValid");
    const QVariant selectionRange = invoke(m_editorSelectionController, "contextMenuEditorSelectionRange");
    const QVariantMap map = safeMap(selectionRange);
    return map.value(QStringLiteral("end")).toInt() > map.value(QStringLiteral("start")).toInt();
}

bool ContentsDisplayInteractionViewModel::ensureEditorSelectionContextMenuSnapshot() const
{
    return editorSelectionContextMenuSnapshotValid() || primeEditorSelectionContextMenuSnapshot();
}

bool ContentsDisplayInteractionViewModel::primeEditorSelectionContextMenuSnapshot() const
{
    if (property(m_contentsView, "showStructuredDocumentFlow").toBool())
        return primeStructuredSelectionContextMenuSnapshot();
    return invokeBool(m_editorSelectionController, "primeContextMenuSelectionSnapshot");
}

void ContentsDisplayInteractionViewModel::scheduleEditorEntrySnapshotReconcile() const
{
    invokeVoid(m_selectionMountViewModel, "scheduleEditorEntrySnapshotReconcile");
}

void ContentsDisplayInteractionViewModel::pollSelectedNoteSnapshot() const
{
    invokeVoid(m_selectionMountViewModel, "pollSelectedNoteSnapshot");
}

bool ContentsDisplayInteractionViewModel::reconcileEditorEntrySnapshotOnce() const
{
    return invokeBool(m_selectionMountViewModel, "reconcileEditorEntrySnapshotOnce");
}

bool ContentsDisplayInteractionViewModel::persistEditorTextImmediately(const QString& nextText) const
{
    return persistRawEditorTextImmediately(nextText);
}

bool ContentsDisplayInteractionViewModel::queueStructuredInlineFormatWrap(const QString& tagName) const
{
    if (!property(m_contentsView, "showStructuredDocumentFlow").toBool() || m_structuredDocumentFlow == nullptr)
        return false;
    if (m_structuredDocumentFlow->metaObject()->indexOfMethod("inlineFormatTargetState()") < 0
        || m_structuredDocumentFlow->metaObject()->indexOfMethod(
               "applyInlineFormatToBlockSelection(QVariant,QVariant,QVariantMap)") < 0)
    {
        if (m_structuredDocumentFlow->metaObject()->indexOfMethod("applyInlineFormatToActiveSelection(QVariant)") >= 0)
            return invokeBool(m_structuredDocumentFlow, "applyInlineFormatToActiveSelection", {tagName});
        return false;
    }
    const QVariantMap targetState = safeMap(invoke(m_structuredDocumentFlow, "inlineFormatTargetState"));
    if (!targetState.value(QStringLiteral("valid")).toBool())
        return false;
    const QString normalizedNoteId = normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteId"));
    if (normalizedNoteId.isEmpty() || normalizedNoteId != normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteId")))
        return false;
    QVariantMap selectionSnapshot = safeMap(targetState.value(QStringLiteral("selectionSnapshot")));
    QVariantMap normalizedSelectionSnapshot;
    normalizedSelectionSnapshot.insert(QStringLiteral("cursorPosition"), selectionSnapshot.value(QStringLiteral("cursorPosition")).toInt());
    normalizedSelectionSnapshot.insert(QStringLiteral("selectedText"), normalizedTextValue(selectionSnapshot.value(QStringLiteral("selectedText"))));
    normalizedSelectionSnapshot.insert(QStringLiteral("selectionEnd"), selectionSnapshot.value(QStringLiteral("selectionEnd")).toInt());
    normalizedSelectionSnapshot.insert(QStringLiteral("selectionStart"), selectionSnapshot.value(QStringLiteral("selectionStart")).toInt());
    return invokeBool(
        m_structuredDocumentFlow,
        "applyInlineFormatToBlockSelection",
        {qMax(0, targetState.value(QStringLiteral("blockIndex")).toInt()), tagName, normalizedSelectionSnapshot});
}

bool ContentsDisplayInteractionViewModel::queueInlineFormatWrap(const QString& tagName) const
{
    if (queueStructuredInlineFormatWrap(tagName))
        return true;
    return invokeBool(m_editorSelectionController, "queueInlineFormatWrap", {tagName});
}

bool ContentsDisplayInteractionViewModel::queueAgendaShortcutInsertion() const
{
    if (property(m_contentsView, "showStructuredDocumentFlow").toBool()
        && m_structuredDocumentFlow != nullptr
        && m_structuredDocumentFlow->metaObject()->indexOfMethod("insertStructuredShortcutAtActivePosition(QVariant)") >= 0
        && invokeBool(m_structuredDocumentFlow, "insertStructuredShortcutAtActivePosition", {QStringLiteral("agenda")}))
        return true;
    return invokeBool(m_editorTypingController, "queueAgendaShortcutInsertion");
}

bool ContentsDisplayInteractionViewModel::queueCalloutShortcutInsertion() const
{
    if (property(m_contentsView, "showStructuredDocumentFlow").toBool()
        && m_structuredDocumentFlow != nullptr
        && m_structuredDocumentFlow->metaObject()->indexOfMethod("insertStructuredShortcutAtActivePosition(QVariant)") >= 0
        && invokeBool(m_structuredDocumentFlow, "insertStructuredShortcutAtActivePosition", {QStringLiteral("callout")}))
        return true;
    return invokeBool(m_editorTypingController, "queueCalloutShortcutInsertion");
}

bool ContentsDisplayInteractionViewModel::queueBreakShortcutInsertion() const
{
    if (property(m_contentsView, "showStructuredDocumentFlow").toBool()
        && m_structuredDocumentFlow != nullptr
        && m_structuredDocumentFlow->metaObject()->indexOfMethod("insertStructuredShortcutAtActivePosition(QVariant)") >= 0
        && invokeBool(m_structuredDocumentFlow, "insertStructuredShortcutAtActivePosition", {QStringLiteral("break")}))
        return true;
    return invokeBool(m_editorTypingController, "queueBreakShortcutInsertion");
}

void ContentsDisplayInteractionViewModel::focusStructuredBlockSourceOffset(const int sourceOffset) const
{
    const QVariantMap focusPlan = safeMap(
        invoke(
            m_editOperationCoordinator,
            "focusStructuredSourceOffsetPlan",
            {property(m_contentsView, "showStructuredDocumentFlow"),
             m_structuredDocumentFlow != nullptr
                 && m_structuredDocumentFlow->metaObject()->indexOfMethod("requestFocus(QVariant)") >= 0,
             sourceOffset}));
    if (!focusPlan.value(QStringLiteral("handled")).toBool())
        return;
    QVariantMap request;
    request.insert(QStringLiteral("sourceOffset"), focusPlan.value(QStringLiteral("targetOffset")).toInt());
    invokeVoid(m_structuredDocumentFlow, "requestFocus", {request});
}

bool ContentsDisplayInteractionViewModel::applyDocumentSourceMutation(const QString& nextSourceText, const QVariant& focusRequest) const
{
    const QString currentSourceText = currentDocumentSourceText();
    const QString normalizedNextSourceText = normalizedTextValue(nextSourceText);
    if (normalizedNextSourceText == currentSourceText)
        return false;
    if (!ensureEditorSessionBoundForSourceMutation(currentSourceText))
        return false;
    if (invokeBool(m_resourceImportController, "resourceTagLossDetected", {currentSourceText, normalizedNextSourceText}))
    {
        invokeVoid(m_resourceImportController, "restoreEditorSurfaceFromPresentation");
        return false;
    }
    if (!commitRawEditorTextMutation(normalizedNextSourceText))
        return false;
    const QString committedText = committedEditorText(normalizedNextSourceText);
    invokeVoid(m_presentationRefreshController, "clearPendingWhileFocused");
    if (!property(m_contentsView, "showStructuredDocumentFlow").toBool())
        commitDocumentPresentationRefresh();
    else
        invokeVoid(m_eventPump, "stopDocumentPresentationRefreshTimer");
    if (focusRequestRequiresImmediatePersistence(focusRequest))
        persistRawEditorTextImmediately(committedText);
    if (m_structuredDocumentFlow != nullptr
        && m_structuredDocumentFlow->metaObject()->indexOfMethod("requestFocus(QVariant)") >= 0
        && invokeBool(
            m_editorInputPolicyAdapter,
            "shouldRestoreFocusForMutation",
            {focusRequest,
             QVariantMap{{QStringLiteral("compositionActive"), property(m_editorInputPolicyAdapter, "nativeCompositionActive")},
                         {QStringLiteral("editorInputFocused"), property(m_contentsView, "editorInputFocused")},
                         {QStringLiteral("nativeTextInputPriority"), property(m_contentsView, "nativeTextInputPriority")}}}))
    {
        requestStructuredFocusLater(focusRequest);
    }
    invokeVoid(m_contentsView, "editorTextEdited", {committedText});
    return true;
}

bool ContentsDisplayInteractionViewModel::setAgendaTaskDone(const int taskOpenTagStart, const int taskOpenTagEnd, const bool checked) const
{
    const QString currentSourceText = normalizedTextValue(property(m_contentsView, "editorText"));
    if (m_contentsAgendaBackend == nullptr)
        return false;
    const QString nextSourceText = invoke(
                                       m_contentsAgendaBackend,
                                       "rewriteTaskDoneAttribute",
                                       {currentSourceText, taskOpenTagStart, taskOpenTagEnd, checked})
                                       .toString();
    QVariantMap focusRequest;
    focusRequest.insert(QStringLiteral("taskOpenTagStart"), taskOpenTagStart);
    return applyDocumentSourceMutation(nextSourceText, focusRequest);
}

QVariant ContentsDisplayInteractionViewModel::selectedEditorRange() const
{
    return invoke(m_editorSelectionController, "selectedEditorRange");
}

bool ContentsDisplayInteractionViewModel::wrapSelectedEditorTextWithTag(const QString& tagName, const QVariant& explicitSelectionRange) const
{
    return invokeBool(m_editorSelectionController, "wrapSelectedEditorTextWithTag", {tagName, explicitSelectionRange});
}

void ContentsDisplayInteractionViewModel::scheduleSelectionModelSync(const QVariant& options) const
{
    invokeVoid(m_selectionMountViewModel, "scheduleSelectionModelSync", {options});
}

bool ContentsDisplayInteractionViewModel::executeSelectionDeliveryPlan(const QVariant& plan, const QString& fallbackKey) const
{
    return invokeBool(m_selectionMountViewModel, "executeSelectionDeliveryPlan", {plan, fallbackKey});
}

void ContentsDisplayInteractionViewModel::scheduleEditorFocusForNote(const QString& noteId) const
{
    invokeVoid(m_selectionMountViewModel, "scheduleEditorFocusForNote", {noteId});
}

void ContentsDisplayInteractionViewModel::applyPresentationRefreshPlan(const QVariant& plan) const
{
    const QVariantMap refreshPlan = safeMap(plan);
    invokeVoid(m_eventPump, "applyPresentationRefreshTimerPlan", {refreshPlan});
    if (refreshPlan.value(QStringLiteral("clearPresentation")).toBool()
        && !property(m_contentsView, "renderedEditorHtml").toString().isEmpty())
        setProperty(m_contentsView, "renderedEditorHtml", QString());
    if (refreshPlan.value(QStringLiteral("commitRefresh")).toBool())
        commitDocumentPresentationRefresh();
    if (refreshPlan.value(QStringLiteral("requestMinimapRefresh")).toBool())
        invokeVoid(m_contentsView, "scheduleMinimapSnapshotRefresh", {false});
    if (refreshPlan.value(QStringLiteral("requestMinimapRepaint")).toBool()
        && m_minimapLayer != nullptr
        && property(m_contentsView, "minimapRefreshEnabled").toBool())
        invokeVoid(m_minimapLayer, "requestRepaint");
}

void ContentsDisplayInteractionViewModel::executeRefreshPlan(const QVariant& plan) const
{
    const QVariantMap refreshPlan = safeMap(plan);
    if (refreshPlan.value(QStringLiteral("resetNoteEntryLineGeometry")).toBool())
        invokeVoid(m_contentsView, "resetNoteEntryLineGeometryState");
    if (refreshPlan.value(QStringLiteral("requestStructuredLayoutRefresh")).toBool()
        && m_structuredDocumentFlow != nullptr
        && !property(m_contentsView, "selectedNoteBodyLoading").toBool()
        && property(m_contentsView, "selectedNoteBodyNoteId") == property(m_contentsView, "selectedNoteId"))
    {
        scheduleStructuredDocumentOpenLayoutRefresh(refreshPlan.value(QStringLiteral("gutterReason")).toString());
    }
    if (refreshPlan.value(QStringLiteral("scheduleViewportGutterRefresh")).toBool())
        invokeVoid(m_contentsView, "scheduleViewportGutterRefresh");
    if (refreshPlan.contains(QStringLiteral("gutterPassCount")))
    {
        invokeVoid(
            m_contentsView,
            "scheduleGutterRefresh",
            {refreshPlan.value(QStringLiteral("gutterPassCount")).toInt(),
             refreshPlan.value(QStringLiteral("gutterReason")).toString()});
    }
}

void ContentsDisplayInteractionViewModel::scheduleStructuredDocumentOpenLayoutRefresh(const QString& reason) const
{
    if (m_structuredDocumentFlow == nullptr)
        return;
    if (m_structuredDocumentFlow->metaObject()->indexOfMethod("scheduleEditorOpenLayoutCacheRefresh(QVariant)") >= 0)
    {
        invokeVoid(m_structuredDocumentFlow, "scheduleEditorOpenLayoutCacheRefresh", {reason});
        return;
    }
    invokeVoid(m_structuredDocumentFlow, "scheduleLayoutCacheRefresh");
}

void ContentsDisplayInteractionViewModel::scheduleDeferredDocumentPresentationRefresh() const
{
    applyPresentationRefreshPlan(invoke(m_presentationRefreshController, "planDeferredRequest"));
}

void ContentsDisplayInteractionViewModel::scheduleDocumentPresentationRefresh(const bool forceImmediate) const
{
    applyPresentationRefreshPlan(invoke(m_presentationRefreshController, "planRefreshRequest", {forceImmediate}));
}

void ContentsDisplayInteractionViewModel::resetEditorSelectionCache() const
{
    invokeVoid(m_selectionMountViewModel, "resetEditorSelectionCache");
}

QString ContentsDisplayInteractionViewModel::normalizedTextValue(const QVariant& value) const
{
    return value.isValid() && !value.isNull() ? value.toString() : QString();
}

QString ContentsDisplayInteractionViewModel::normalizedTrimmedTextValue(const QVariant& value) const
{
    return normalizedTextValue(value).trimmed();
}

QString ContentsDisplayInteractionViewModel::selectedNoteDirectoryPath() const
{
    return normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteDirectoryPath"));
}

bool ContentsDisplayInteractionViewModel::editorSessionBoundToSelectedNote() const
{
    if (m_editorSession == nullptr || m_contentsView == nullptr)
        return false;
    const QString selectedNoteId = normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteId"));
    const QString editorBoundNoteId = normalizedTrimmedTextValue(property(m_editorSession, "editorBoundNoteId"));
    if (selectedNoteId.isEmpty() || editorBoundNoteId != selectedNoteId)
        return false;
    const QString selectedDirectory = selectedNoteDirectoryPath();
    if (selectedDirectory.isEmpty())
        return true;
    return normalizedTrimmedTextValue(property(m_editorSession, "editorBoundNoteDirectoryPath")) == selectedDirectory;
}

bool ContentsDisplayInteractionViewModel::ensureEditorSessionBoundForSourceMutation(const QString& currentSourceText) const
{
    if (editorSessionBoundToSelectedNote())
        return true;
    if (m_editorSession == nullptr || m_contentsView == nullptr)
        return false;
    const QString selectedNoteId = normalizedTrimmedTextValue(property(m_contentsView, "selectedNoteId"));
    if (selectedNoteId.isEmpty())
        return false;
    invokeBool(
        m_editorSession,
        "requestSyncEditorTextFromSelection",
        {selectedNoteId, normalizedTextValue(currentSourceText), selectedNoteId, selectedNoteDirectoryPath()});
    return editorSessionBoundToSelectedNote();
}

QString ContentsDisplayInteractionViewModel::currentDocumentSourceText() const
{
    const QVariant presentationSource = property(m_contentsView, "documentPresentationSourceText");
    if (presentationSource.isValid() && !presentationSource.isNull())
        return presentationSource.toString();
    return normalizedTextValue(property(m_contentsView, "editorText"));
}

bool ContentsDisplayInteractionViewModel::focusRequestRequiresImmediatePersistence(const QVariant& focusRequest) const
{
    return safeMap(focusRequest).value(QStringLiteral("immediatePersistence")).toBool();
}

bool ContentsDisplayInteractionViewModel::persistRawEditorTextImmediately(const QString& nextSourceText) const
{
    if (m_editorSession == nullptr)
        return false;
    const bool persistedWithText = invokeBool(m_editorSession, "persistEditorTextImmediatelyWithText", {nextSourceText});
    if (persistedWithText)
        return true;
    return invokeBool(m_editorSession, "persistEditorTextImmediately");
}

QString ContentsDisplayInteractionViewModel::committedEditorText(const QString& fallbackText) const
{
    const QVariant editorText = property(m_editorSession, "editorText");
    if (editorText.isValid() && !editorText.isNull())
        return editorText.toString();
    return fallbackText;
}

bool ContentsDisplayInteractionViewModel::commitRawEditorTextMutation(const QString& nextSourceText) const
{
    if (m_editorSession == nullptr)
        return false;
    return invokeBool(m_editorSession, "commitRawEditorTextMutation", {nextSourceText});
}

void ContentsDisplayInteractionViewModel::requestStructuredFocusLater(const QVariant& focusRequest) const
{
    if (m_structuredDocumentFlow == nullptr || m_structuredDocumentFlow->metaObject()->indexOfMethod("requestFocus(QVariant)") < 0)
        return;
    const QVariant normalizedRequest = focusRequest.isValid() ? focusRequest : QVariant(QVariantMap());
    QPointer<QObject> structuredFlow(m_structuredDocumentFlow);
    QTimer::singleShot(0, this, [this, structuredFlow, normalizedRequest]() {
        if (structuredFlow == nullptr)
            return;
        invokeVoid(structuredFlow, "requestFocus", {normalizedRequest});
    });
}

QVariant ContentsDisplayInteractionViewModel::invoke(
    QObject* target,
    const char* methodName,
    const QVariantList& arguments) const
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

bool ContentsDisplayInteractionViewModel::invokeBool(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    return invoke(target, methodName, arguments).toBool();
}

void ContentsDisplayInteractionViewModel::invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    Q_UNUSED(invoke(target, methodName, arguments));
}

QVariant ContentsDisplayInteractionViewModel::property(QObject* target, const char* propertyName) const
{
    return target == nullptr || propertyName == nullptr ? QVariant() : target->property(propertyName);
}

void ContentsDisplayInteractionViewModel::setProperty(QObject* target, const char* propertyName, const QVariant& value) const
{
    if (target == nullptr || propertyName == nullptr)
        return;
    target->setProperty(propertyName, value);
}

QString ContentsDisplayInteractionViewModel::bodyTagShortcutKind(const QJSValue& event) const
{
    const int modifiers = eventModifiers(event);
    const bool metaPressed = (modifiers & Qt::MetaModifier) != 0;
    const bool controlPressed = (modifiers & Qt::ControlModifier) != 0;
    const bool altPressed = (modifiers & Qt::AltModifier) != 0;
    const bool shiftPressed = (modifiers & Qt::ShiftModifier) != 0;
    if (!metaPressed && !controlPressed)
        return {};
    const int key = eventKey(event);
    const QString normalizedText = eventText(event).toUpper();
    if (altPressed && !shiftPressed)
    {
        if (key == keyT || normalizedText == QStringLiteral("T"))
            return QStringLiteral("agenda");
        if (key == keyC || normalizedText == QStringLiteral("C"))
            return QStringLiteral("callout");
        return {};
    }
    if (shiftPressed && !altPressed && (key == keyH || normalizedText == QStringLiteral("H")))
        return QStringLiteral("break");
    return {};
}

int ContentsDisplayInteractionViewModel::eventKey(const QJSValue& event) const
{
    return event.property(QStringLiteral("key")).toInt();
}

int ContentsDisplayInteractionViewModel::eventModifiers(const QJSValue& event) const
{
    return event.property(QStringLiteral("modifiers")).toInt();
}

QString ContentsDisplayInteractionViewModel::eventText(const QJSValue& event) const
{
    return event.property(QStringLiteral("text")).toString();
}

void ContentsDisplayInteractionViewModel::setEventAccepted(const QJSValue& event) const
{
    if (!event.isObject())
        return;
    QJSValue mutableEvent(event);
    mutableEvent.setProperty(QStringLiteral("accepted"), true);
}
