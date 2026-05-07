#include "app/models/editor/display/ContentsEditorDisplayBackend.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QDir>
#include <QMetaProperty>

#include <algorithm>

using namespace WhatSon::Editor::DynamicObjectSupport;

ContentsEditorDisplayBackend::ContentsEditorDisplayBackend(QObject* parent)
    : QObject(parent)
    , m_editorSession(this)
    , m_editorSaveCoordinator(this)
    , m_presentationProjection(this)
    , m_structuredBlockRenderer(this)
    , m_bodyResourceRenderer(this)
    , m_resourceTagController(this)
    , m_inlineResourcePresentation(this)
    , m_minimapLayoutMetrics(this)
{
    m_editorSession.setObjectName(QStringLiteral("contentsDisplayEditorSession"));
    m_editorSaveCoordinator.setObjectName(QStringLiteral("contentsDisplayEditorSaveCoordinator"));
    m_editorSaveCoordinator.setEditorSession(&m_editorSession);
    m_presentationProjection.setObjectName(QStringLiteral("contentsDisplayPresentationProjection"));
    m_structuredBlockRenderer.setObjectName(QStringLiteral("contentsDisplayStructuredBlockRenderer"));
    m_bodyResourceRenderer.setObjectName(QStringLiteral("contentsDisplayBodyResourceRenderer"));
    m_minimapLayoutMetrics.setObjectName(QStringLiteral("contentsDisplayMinimapLayoutMetrics"));
    m_bodyResourceRenderer.setMaxRenderCount(0);
    m_inlineResourcePresentation.setProperty("editorHorizontalInset", 0);
    m_inlineResourcePresentation.setProperty("inlineHtmlImageRenderingEnabled", true);
    m_inlineResourcePresentation.setProperty("resourceEditorPlaceholderLineCount", 1);
    m_resourceTagController.setProperty("view", QVariant::fromValue(static_cast<QObject*>(this)));
    m_inlineResourcePresentation.setProperty("view", QVariant::fromValue(static_cast<QObject*>(this)));
    m_inlineResourcePresentation.setProperty("contentEditor", QVariant::fromValue(static_cast<QObject*>(this)));
    m_inlineResourcePresentation.setProperty("editorViewport", QVariant::fromValue(static_cast<QObject*>(this)));
    m_inlineResourcePresentation.setProperty("bodyResourceRenderer", QVariant::fromValue(static_cast<QObject*>(&m_bodyResourceRenderer)));
    m_inlineResourcePresentation.setProperty("resourceTagController", QVariant::fromValue(static_cast<QObject*>(&m_resourceTagController)));
    syncProjectionInputs();
    syncResourceInputs();

    connect(
        &m_editorSession,
        &ContentsEditorSessionController::editorTextChanged,
        this,
        [this]() {
            syncProjectionInputs();
            syncResourceInputs();
        });
    connect(
        &m_structuredBlockRenderer,
        &ContentsStructuredBlockRenderer::renderedBlocksChanged,
        this,
        [this]() {
            m_bodyResourceRenderer.setDocumentBlocks(m_structuredBlockRenderer.renderedDocumentBlocks());
            syncResourceInputs();
        });
}

ContentsEditorDisplayBackend::~ContentsEditorDisplayBackend()
{
    clearNoteActiveStateSession(m_noteActiveState);
    clearNoteActiveStateSaveCoordinator(m_noteActiveState);
}

QObject* ContentsEditorDisplayBackend::contentController() const noexcept { return m_contentController.data(); }

void ContentsEditorDisplayBackend::setContentController(QObject* value)
{
    if (m_contentController == value)
        return;
    m_contentController = value;
    m_editorSaveCoordinator.setContentController(value);
    m_bodyResourceRenderer.setContentController(value);
    emit contentControllerChanged();
}

QObject* ContentsEditorDisplayBackend::libraryHierarchyController() const noexcept
{
    return m_libraryHierarchyController.data();
}

void ContentsEditorDisplayBackend::setLibraryHierarchyController(QObject* value)
{
    if (m_libraryHierarchyController == value)
        return;
    m_libraryHierarchyController = value;
    m_bodyResourceRenderer.setFallbackContentController(value);
    emit libraryHierarchyControllerChanged();
}

QObject* ContentsEditorDisplayBackend::noteListModel() const noexcept { return m_noteListModel.data(); }

void ContentsEditorDisplayBackend::setNoteListModel(QObject* value)
{
    if (m_noteListModel == value)
        return;
    disconnectNoteListModel();
    m_noteListModel = value;
    if (m_noteListModel)
    {
        m_noteListConnections.append(connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            [this]() {
                m_noteListModel = nullptr;
                publishCurrentNoteChanged(SyncReset::ResetViewport);
                emit noteListModelChanged();
            }));
        const auto connectReset = [this](const char* propertyName) {
            connectPropertyNotify(m_noteListModel, propertyName, "handleResetCurrentNoteFromModel()");
        };
        const auto connectPreserve = [this](const char* propertyName) {
            connectPropertyNotify(m_noteListModel, propertyName, "handlePreserveCurrentNoteFromModel()");
        };
        connectPreserve("currentBodyText");
        connectReset("currentIndex");
        connectReset("currentNoteDirectoryPath");
        connectReset("currentNoteEntry");
        connectReset("currentNoteId");
    }
    emit noteListModelChanged();
    publishCurrentNoteChanged(SyncReset::ResetViewport);
}

QObject* ContentsEditorDisplayBackend::noteActiveState() const noexcept { return m_noteActiveState.data(); }

void ContentsEditorDisplayBackend::setNoteActiveState(QObject* value)
{
    if (m_noteActiveState == value)
        return;
    clearNoteActiveStateSession(m_noteActiveState);
    disconnectNoteActiveState();
    m_noteActiveState = value;
    if (m_noteActiveState)
    {
        m_noteActiveStateConnections.append(connect(
            m_noteActiveState,
            &QObject::destroyed,
            this,
            [this]() {
                m_noteActiveState = nullptr;
                publishCurrentNoteChanged(SyncReset::ResetViewport);
                emit noteActiveStateChanged();
            }));
        connectPropertyNotify(m_noteActiveState, "activeNoteId", "handleResetCurrentNoteFromActiveState()");
        connectPropertyNotify(m_noteActiveState, "activeNoteDirectoryPath", "handleResetCurrentNoteFromActiveState()");
        connectPropertyNotify(m_noteActiveState, "activeNoteEntry", "handleResetCurrentNoteFromActiveState()");
        connectPropertyNotify(m_noteActiveState, "activeNoteBodyText", "handlePreserveCurrentNoteFromActiveState()");
        const int signalIndex = m_noteActiveState->metaObject()->indexOfSignal("activeNoteStateChanged()");
        const int slotIndex = metaObject()->indexOfSlot("handlePreserveCurrentNoteFromActiveState()");
        if (signalIndex >= 0 && slotIndex >= 0)
        {
            m_noteActiveStateConnections.append(QObject::connect(
                m_noteActiveState,
                m_noteActiveState->metaObject()->method(signalIndex),
                this,
                metaObject()->method(slotIndex),
                Qt::UniqueConnection));
        }
    }
    attachEditorSessionToActiveState();
    emit noteActiveStateChanged();
    publishCurrentNoteChanged(SyncReset::ResetViewport);
}

bool ContentsEditorDisplayBackend::paperPaletteEnabled() const noexcept { return m_paperPaletteEnabled; }

void ContentsEditorDisplayBackend::setPaperPaletteEnabled(const bool value)
{
    if (m_paperPaletteEnabled == value)
        return;
    m_paperPaletteEnabled = value;
    m_presentationProjection.setPaperPaletteEnabled(value);
    emit paperPaletteEnabledChanged();
}

bool ContentsEditorDisplayBackend::minimapVisible() const noexcept { return m_minimapVisible; }

void ContentsEditorDisplayBackend::setMinimapVisible(const bool value)
{
    if (m_minimapVisible == value)
        return;
    m_minimapVisible = value;
    m_minimapLayoutMetrics.setMinimapVisible(value);
    emit minimapVisibleChanged();
}

int ContentsEditorDisplayBackend::editorCursorPosition() const noexcept { return m_editorCursorPosition; }

void ContentsEditorDisplayBackend::setEditorCursorPosition(const int value)
{
    if (m_editorCursorPosition == value)
        return;
    m_editorCursorPosition = value;
    m_presentationProjection.setSourceCursorPosition(value);
    emit editorCursorPositionChanged();
}

QObject* ContentsEditorDisplayBackend::structuredDocumentFlow() const noexcept
{
    return m_structuredDocumentFlow.data();
}

void ContentsEditorDisplayBackend::setStructuredDocumentFlow(QObject* value)
{
    if (m_structuredDocumentFlow == value)
        return;
    m_structuredDocumentFlow = value;
    m_resourceTagController.setProperty("structuredDocumentFlow", QVariant::fromValue(value));
    m_resourceTagController.setProperty("showStructuredDocumentFlow", value != nullptr);
    emit structuredDocumentFlowChanged();
}

QObject* ContentsEditorDisplayBackend::editorSession() noexcept { return &m_editorSession; }
QObject* ContentsEditorDisplayBackend::presentationProjection() noexcept { return &m_presentationProjection; }
QObject* ContentsEditorDisplayBackend::structuredBlockRenderer() noexcept { return &m_structuredBlockRenderer; }
QObject* ContentsEditorDisplayBackend::bodyResourceRenderer() noexcept { return &m_bodyResourceRenderer; }
QObject* ContentsEditorDisplayBackend::resourceTagController() noexcept { return &m_resourceTagController; }
QObject* ContentsEditorDisplayBackend::inlineResourcePresentation() noexcept { return &m_inlineResourcePresentation; }
QObject* ContentsEditorDisplayBackend::minimapLayoutMetrics() noexcept { return &m_minimapLayoutMetrics; }

QString ContentsEditorDisplayBackend::currentNoteId() const
{
    if (m_noteActiveState)
        return activeStateStringProperty("activeNoteId");
    return stringValue(currentNoteEntry(), QStringLiteral("noteId"), modelStringProperty("currentNoteId"));
}

QString ContentsEditorDisplayBackend::currentNoteDirectoryPath() const
{
    if (m_noteActiveState)
        return activeStateStringProperty("activeNoteDirectoryPath");
    return stringValue(
        currentNoteEntry(),
        QStringLiteral("noteDirectoryPath"),
        modelStringProperty("currentNoteDirectoryPath"));
}

QString ContentsEditorDisplayBackend::currentRawBodyText() const
{
    if (m_noteActiveState)
        return activeStateStringProperty("activeNoteBodyText");
    return stringValue(currentNoteEntry(), QStringLiteral("bodyText"), modelStringProperty("currentBodyText"));
}

bool ContentsEditorDisplayBackend::noteDocumentParseMounted() const
{
    return !currentNoteId().isEmpty();
}

bool ContentsEditorDisplayBackend::structuredBlockBackgroundRefreshEnabled() const
{
    return noteDocumentParseMounted();
}

QVariant ContentsEditorDisplayBackend::applyDocumentSourceMutation(const QVariant& payload, const QVariant& focusRequest)
{
    if (!payload.isValid() || payload.isNull())
        return false;

    QString nextText;
    if (payload.metaType().id() == QMetaType::QVariantMap || payload.canConvert<QVariantMap>())
    {
        const QVariantMap payloadMap = payload.toMap();
        const QVariant nextSourceText = payloadMap.value(QStringLiteral("nextSourceText"));
        if (!nextSourceText.isValid() || nextSourceText.isNull())
            return false;
        nextText = nextSourceText.toString();
    }
    else
    {
        nextText = payload.toString();
    }

    Q_UNUSED(focusRequest);
    return commitEditedSourceText(nextText);
}

bool ContentsEditorDisplayBackend::commitEditedSourceText(const QString& text)
{
    const bool changed = m_editorSaveCoordinator.commitEditedSourceText(text);
    if (!changed)
        return false;

    syncProjectionInputs();
    syncResourceInputs();
    emit editorTextEdited(m_editorSession.editorText());
    return true;
}

void ContentsEditorDisplayBackend::commitDocumentPresentationRefresh()
{
    syncProjectionInputs();
}

QVariant ContentsEditorDisplayBackend::encodeXmlAttributeValue(const QVariant& value) const
{
    QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
    text.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    text.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    text.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    text.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    text.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    return text;
}

QVariant ContentsEditorDisplayBackend::currentEditorCursorPosition() const noexcept
{
    return m_editorCursorPosition;
}

QVariant ContentsEditorDisplayBackend::terminalBodyClickSourceOffset() const noexcept
{
    return m_editorSession.editorText().size();
}

QVariant ContentsEditorDisplayBackend::requestEditorSelectionContextMenuFromPointer(
    const QVariant& pointerKind) const
{
    const QString normalizedKind = pointerKind.isValid() && !pointerKind.isNull()
        ? pointerKind.toString()
        : QString{};
    return normalizedKind == QStringLiteral("right-click")
        || normalizedKind == QStringLiteral("long-press");
}

QVariantList ContentsEditorDisplayBackend::inlineResourceVisualBlocks(
    const QVariant& renderedResources,
    const int targetFrameWidth) const
{
    return m_inlineResourcePresentation.inlineResourceVisualBlocks(
        renderedResources,
        std::max(120, targetFrameWidth));
}

bool ContentsEditorDisplayBackend::syncSessionFromCurrentNote(const bool resetViewport)
{
    if (currentNoteId().isEmpty())
        return false;
    if (resetViewport && currentNoteIdentityDiffersFromBoundSession())
        emit editorViewportResetRequested();
    if (m_noteActiveState && invokeNoArgBool(m_noteActiveState, "syncEditorSessionFromActiveNote"))
    {
        syncProjectionInputs();
        syncResourceInputs();
        return true;
    }
    const bool synced = m_editorSaveCoordinator.syncEditorSessionFromSelection(
        currentNoteId(),
        currentRawBodyText(),
        currentNoteId(),
        currentNoteDirectoryPath());
    syncProjectionInputs();
    syncResourceInputs();
    return synced;
}

bool ContentsEditorDisplayBackend::connectPropertyNotify(
    QObject* source,
    const char* propertyName,
    const char* slotSignature)
{
    if (!source || !propertyName || !slotSignature)
        return false;
    const QMetaObject* sourceMetaObject = source->metaObject();
    const int propertyIndex = sourceMetaObject->indexOfProperty(propertyName);
    if (propertyIndex < 0)
        return false;
    const QMetaProperty property = sourceMetaObject->property(propertyIndex);
    if (!property.hasNotifySignal())
        return false;
    const int slotIndex = metaObject()->indexOfSlot(slotSignature);
    if (slotIndex < 0)
        return false;
    const QMetaObject::Connection connection = QObject::connect(
        source,
        property.notifySignal(),
        this,
        metaObject()->method(slotIndex),
        Qt::UniqueConnection);
    if (!connection)
        return false;
    if (source == m_noteListModel)
        m_noteListConnections.append(connection);
    else if (source == m_noteActiveState)
        m_noteActiveStateConnections.append(connection);
    return true;
}

bool ContentsEditorDisplayBackend::invokeNoArgBool(QObject* target, const char* methodName) const
{
    if (!target || !methodName)
        return false;

    bool boolResult = false;
    if (QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(bool, boolResult)))
        return boolResult;

    QVariant variantResult;
    if (QMetaObject::invokeMethod(target, methodName, Q_RETURN_ARG(QVariant, variantResult)))
        return variantResult.toBool();

    return QMetaObject::invokeMethod(target, methodName);
}

void ContentsEditorDisplayBackend::handleResetCurrentNoteFromModel()
{
    publishCurrentNoteChanged(SyncReset::ResetViewport);
}

void ContentsEditorDisplayBackend::handlePreserveCurrentNoteFromModel()
{
    publishCurrentNoteChanged(SyncReset::PreserveViewport);
}

void ContentsEditorDisplayBackend::handleResetCurrentNoteFromActiveState()
{
    publishCurrentNoteChanged(SyncReset::ResetViewport);
}

void ContentsEditorDisplayBackend::handlePreserveCurrentNoteFromActiveState()
{
    publishCurrentNoteChanged(SyncReset::PreserveViewport);
}

void ContentsEditorDisplayBackend::clearNoteActiveStateSession(QObject* state)
{
    if (state && state->property("editorSession").value<QObject*>() == &m_editorSession)
        state->setProperty("editorSession", QVariant::fromValue(static_cast<QObject*>(nullptr)));
}

void ContentsEditorDisplayBackend::clearNoteActiveStateSaveCoordinator(QObject* state)
{
    if (state && state->property("editorSaveCoordinator").value<QObject*>() == &m_editorSaveCoordinator)
        state->setProperty("editorSaveCoordinator", QVariant::fromValue(static_cast<QObject*>(nullptr)));
}

void ContentsEditorDisplayBackend::attachEditorSessionToActiveState()
{
    if (m_noteActiveState)
        m_noteActiveState->setProperty("editorSession", QVariant::fromValue(static_cast<QObject*>(&m_editorSession)));
    attachEditorSaveCoordinatorToActiveState();
}

void ContentsEditorDisplayBackend::attachEditorSaveCoordinatorToActiveState()
{
    if (m_noteActiveState)
        m_noteActiveState->setProperty(
            "editorSaveCoordinator",
            QVariant::fromValue(static_cast<QObject*>(&m_editorSaveCoordinator)));
}

void ContentsEditorDisplayBackend::disconnectNoteListModel()
{
    for (const QMetaObject::Connection& connection : std::as_const(m_noteListConnections))
        QObject::disconnect(connection);
    m_noteListConnections.clear();
}

void ContentsEditorDisplayBackend::disconnectNoteActiveState()
{
    clearNoteActiveStateSaveCoordinator(m_noteActiveState);
    for (const QMetaObject::Connection& connection : std::as_const(m_noteActiveStateConnections))
        QObject::disconnect(connection);
    m_noteActiveStateConnections.clear();
}

QVariantMap ContentsEditorDisplayBackend::currentNoteEntry() const
{
    if (m_noteActiveState)
    {
        const QVariant entry = propertyValue(m_noteActiveState, "activeNoteEntry");
        if (entry.isValid() && !entry.isNull())
            return entry.toMap();
    }
    if (m_noteListModel)
    {
        const QVariant entry = propertyValue(m_noteListModel, "currentNoteEntry");
        if (entry.isValid() && !entry.isNull())
            return entry.toMap();
    }
    return {};
}

QString ContentsEditorDisplayBackend::modelStringProperty(const char* propertyName) const
{
    return stringProperty(m_noteListModel, propertyName);
}

QString ContentsEditorDisplayBackend::activeStateStringProperty(const char* propertyName) const
{
    return stringProperty(m_noteActiveState, propertyName);
}

QString ContentsEditorDisplayBackend::stringValue(
    const QVariantMap& payload,
    const QString& key,
    const QString& fallback) const
{
    const QVariant value = payload.value(key);
    return value.isValid() && !value.isNull() ? value.toString() : fallback;
}

void ContentsEditorDisplayBackend::publishCurrentNoteChanged(const SyncReset reset)
{
    emit currentNoteChanged();
    syncSessionFromCurrentNote(reset == SyncReset::ResetViewport);
}

bool ContentsEditorDisplayBackend::currentNoteIdentityDiffersFromBoundSession() const
{
    const QString nextNoteId = currentNoteId().trimmed();
    if (nextNoteId.isEmpty())
        return false;

    const auto normalizedPath = [](const QString& path) {
        const QString normalized = QDir::cleanPath(path.trimmed());
        return normalized == QStringLiteral(".") ? QString() : normalized;
    };

    return m_editorSession.editorBoundNoteId().trimmed() != nextNoteId
        || normalizedPath(m_editorSession.editorBoundNoteDirectoryPath()) != normalizedPath(currentNoteDirectoryPath());
}

void ContentsEditorDisplayBackend::syncProjectionInputs()
{
    const QString text = m_editorSession.editorText();
    m_presentationProjection.setSourceText(text);
    m_structuredBlockRenderer.setBackgroundRefreshEnabled(structuredBlockBackgroundRefreshEnabled());
    m_structuredBlockRenderer.setSourceText(text);
}

void ContentsEditorDisplayBackend::syncResourceInputs()
{
    m_bodyResourceRenderer.setNoteId(currentNoteId());
    m_bodyResourceRenderer.setNoteDirectoryPath(currentNoteDirectoryPath());
    m_bodyResourceRenderer.setDocumentBlocks(m_structuredBlockRenderer.renderedDocumentBlocks());
    m_resourceTagController.setProperty("bodyResourceRenderer", QVariant::fromValue(static_cast<QObject*>(&m_bodyResourceRenderer)));
    m_resourceTagController.setProperty("editorSession", QVariant::fromValue(static_cast<QObject*>(&m_editorSession)));
    m_resourceTagController.setProperty("editorText", m_editorSession.editorText());
    m_resourceTagController.setProperty("documentPresentationSourceText", m_editorSession.editorText());
    m_resourceTagController.setProperty("selectedNoteId", currentNoteId());
    m_resourceTagController.setProperty("selectedNoteBodyNoteId", currentNoteId());
    m_resourceTagController.setProperty("selectedNoteBodyText", currentRawBodyText());
}

#include "moc_ContentsEditorDisplayBackend.cpp"
