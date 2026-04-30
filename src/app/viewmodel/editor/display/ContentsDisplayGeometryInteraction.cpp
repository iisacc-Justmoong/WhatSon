#include "app/viewmodel/editor/display/ContentsDisplayGeometryInteraction.hpp"

#include <QMetaObject>
#include <QPointer>
#include <QTimer>

ContentsDisplayGeometryInteraction::ContentsDisplayGeometryInteraction(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayGeometryInteraction::~ContentsDisplayGeometryInteraction() = default;

QObject* ContentsDisplayGeometryInteraction::contentsView() const noexcept { return m_contentsView.data(); }
void ContentsDisplayGeometryInteraction::setContentsView(QObject* value)
{
    if (m_contentsView == value)
        return;
    m_contentsView = value;
    emit contentsViewChanged();
}

QObject* ContentsDisplayGeometryInteraction::eventPump() const noexcept { return m_eventPump.data(); }
void ContentsDisplayGeometryInteraction::setEventPump(QObject* value)
{
    if (m_eventPump == value)
        return;
    m_eventPump = value;
    emit eventPumpChanged();
}

QObject* ContentsDisplayGeometryInteraction::minimapLayer() const noexcept { return m_minimapLayer.data(); }
void ContentsDisplayGeometryInteraction::setMinimapLayer(QObject* value)
{
    if (m_minimapLayer == value)
        return;
    m_minimapLayer = value;
    emit minimapLayerChanged();
}

QObject* ContentsDisplayGeometryInteraction::refreshCoordinator() const noexcept { return m_refreshCoordinator.data(); }
void ContentsDisplayGeometryInteraction::setRefreshCoordinator(QObject* value)
{
    if (m_refreshCoordinator == value)
        return;
    m_refreshCoordinator = value;
    emit refreshCoordinatorChanged();
}

QObject* ContentsDisplayGeometryInteraction::viewportCoordinator() const noexcept { return m_viewportCoordinator.data(); }
void ContentsDisplayGeometryInteraction::setViewportCoordinator(QObject* value)
{
    if (m_viewportCoordinator == value)
        return;
    m_viewportCoordinator = value;
    emit viewportCoordinatorChanged();
}

void ContentsDisplayGeometryInteraction::refreshMinimapSnapshot() const
{
    if (!property(m_contentsView, "lineGeometryRefreshEnabled").toBool())
        return;
    const QString currentNoteId = invoke(m_contentsView, "activeLineGeometryNoteId").toString();
    const bool useStructuredMinimap = invokeBool(m_contentsView, "hasStructuredMinimapEntries");
    const QVariantList effectiveStructuredEntries = invoke(m_contentsView, "effectiveStructuredMinimapEntries").toList();
    const QVariant nextLineGroups = useStructuredMinimap
        ? invoke(m_contentsView,
                 "buildStructuredMinimapLineGroupsForRange",
                 {1, qMax(1, effectiveStructuredEntries.size())})
        : invoke(m_contentsView,
                 "buildMinimapLineGroupsForRange",
                 {1, property(m_contentsView, "logicalLineCount")});
    const QVariant nextRows = invoke(m_contentsView, "flattenMinimapLineGroups", {nextLineGroups});
    const int nextSilhouetteHeight = invoke(m_contentsView, "minimapSilhouetteHeight", {nextRows}).toInt();
    const int nextTrackHeight = qMin(property(m_contentsView, "minimapAvailableTrackHeight").toInt(), nextSilhouetteHeight);

    setProperty(m_contentsView, "minimapLineGroups", nextLineGroups);
    setProperty(m_contentsView, "minimapLineGroupsNoteId", currentNoteId);
    setProperty(m_contentsView, "minimapVisualRows", nextRows);
    setProperty(m_contentsView, "minimapResolvedSilhouetteHeight", nextSilhouetteHeight);
    setProperty(m_contentsView, "minimapResolvedTrackHeight", nextTrackHeight);
    refreshMinimapViewportTracking(nextTrackHeight);
    refreshMinimapCursorTracking(nextRows);
}

void ContentsDisplayGeometryInteraction::refreshMinimapCursorTracking(const QVariant& rowsOverride) const
{
    if (!property(m_contentsView, "minimapRefreshEnabled").toBool())
        return;
    const QVariantMap nextCurrentVisualRow = safeMap(invoke(m_contentsView, "minimapCurrentVisualRow", {rowsOverride}));
    setProperty(m_contentsView, "minimapResolvedCurrentLineHeight", invoke(m_contentsView, "minimapVisualRowPaintHeight", {nextCurrentVisualRow}));
    setProperty(
        m_contentsView,
        "minimapResolvedCurrentLineWidth",
        invoke(
            m_viewportCoordinator,
            "minimapLineBarWidth",
            {nextCurrentVisualRow.value(QStringLiteral("contentWidth")),
             nextCurrentVisualRow.value(QStringLiteral("contentAvailableWidth")),
             nextCurrentVisualRow.value(QStringLiteral("charCount")),
             property(m_contentsView, "minimapResolvedTrackWidth") }));
    setProperty(m_contentsView, "minimapResolvedCurrentLineY", invoke(m_contentsView, "minimapVisualRowPaintY", {nextCurrentVisualRow}));
}

void ContentsDisplayGeometryInteraction::refreshMinimapViewportTracking(const QVariant& trackHeightOverride) const
{
    Q_UNUSED(trackHeightOverride);
    if (!property(m_contentsView, "minimapRefreshEnabled").toBool())
        return;
    const bool nextScrollable = invokeBool(m_contentsView, "isMinimapScrollable");
    const bool hasFlickable = property(m_contentsView, "editorFlickable").isValid() && !property(m_contentsView, "editorFlickable").isNull();
    const QVariant editorFlickable = property(m_contentsView, "editorFlickable");
    QObject* flickable = editorFlickable.value<QObject*>();
    const QVariant nextViewportHeight = invoke(
        m_viewportCoordinator,
        "minimapViewportHeight",
        {hasFlickable,
         invoke(m_contentsView, "minimapContentHeight"),
         property(m_contentsView, "minimapViewportMinHeight")});
    const QVariant nextViewportY = invoke(
        m_viewportCoordinator,
        "minimapViewportY",
        {hasFlickable,
         flickable == nullptr ? QVariant(0) : flickable->property("contentY"),
         invoke(m_contentsView, "minimapContentHeight"),
         nextViewportHeight});
    setProperty(m_contentsView, "minimapScrollable", nextScrollable);
    setProperty(m_contentsView, "minimapResolvedViewportHeight", nextViewportHeight);
    setProperty(m_contentsView, "minimapResolvedViewportY", nextViewportY);
}

void ContentsDisplayGeometryInteraction::resetNoteEntryLineGeometryState() const
{
    resetGutterRefreshState();
    setProperty(m_contentsView, "minimapLineGroups", QVariantList());
    setProperty(m_contentsView, "minimapLineGroupsNoteId", QString());
    setProperty(m_contentsView, "minimapVisualRows", QVariantList());
    setProperty(m_contentsView, "minimapScrollable", false);
    setProperty(m_contentsView, "minimapResolvedCurrentLineHeight", 1);
    setProperty(m_contentsView, "minimapResolvedCurrentLineWidth", 0);
    setProperty(m_contentsView, "minimapResolvedCurrentLineY", 0);
    setProperty(m_contentsView, "minimapResolvedSilhouetteHeight", 1);
    setProperty(m_contentsView, "minimapResolvedTrackHeight", 1);
    setProperty(m_contentsView, "minimapResolvedViewportHeight", 0);
    setProperty(m_contentsView, "minimapResolvedViewportY", 0);
}

void ContentsDisplayGeometryInteraction::resetGutterRefreshState() const
{
    QVariantList visibleEntries;
    QVariantMap firstEntry;
    firstEntry.insert(QStringLiteral("lineNumber"), 1);
    firstEntry.insert(QStringLiteral("y"), 0);
    visibleEntries.append(firstEntry);
    setProperty(m_contentsView, "visibleGutterLineEntries", visibleEntries);
    setProperty(m_contentsView, "logicalLineDocumentYCache", QVariantList());
    setProperty(m_contentsView, "logicalLineDocumentYCacheRevision", -1);
    setProperty(m_contentsView, "logicalLineDocumentYCacheLineCount", 0);
    setProperty(m_contentsView, "logicalLineGutterDocumentYCache", QVariantList());
    setProperty(m_contentsView, "logicalLineGutterDocumentYCacheRevision", -1);
    setProperty(m_contentsView, "logicalLineGutterDocumentYCacheLineCount", 0);
    setProperty(m_contentsView, "structuredGutterGeometrySignature", QString());
}

bool ContentsDisplayGeometryInteraction::refreshLiveLogicalLineMetrics() const
{
    const QVariant metrics = invoke(
        m_viewportCoordinator,
        property(m_contentsView, "hasStructuredLogicalLineGeometry").toBool()
            ? "buildLogicalLineMetricsFromStructuredEntries"
            : "buildLogicalLineMetricsFromText",
        {property(m_contentsView, "hasStructuredLogicalLineGeometry").toBool()
             ? invoke(m_contentsView, "effectiveStructuredLogicalLineEntries")
             : invoke(m_contentsView, "activeLogicalTextSnapshot")});
    const QVariantMap metricsMap = safeMap(metrics);
    return invokeBool(
        m_contentsView,
        "applyLiveLogicalLineMetrics",
        {metricsMap.value(QStringLiteral("logicalTextLength")),
         metricsMap.value(QStringLiteral("lineStartOffsets")),
         metricsMap.value(QStringLiteral("lineCount"))});
}

int ContentsDisplayGeometryInteraction::activeLogicalLineCountSnapshot() const
{
    const QVariant metrics = invoke(
        m_viewportCoordinator,
        property(m_contentsView, "hasStructuredLogicalLineGeometry").toBool()
            ? "buildLogicalLineMetricsFromStructuredEntries"
            : "buildLogicalLineMetricsFromText",
        {property(m_contentsView, "hasStructuredLogicalLineGeometry").toBool()
             ? invoke(m_contentsView, "effectiveStructuredLogicalLineEntries")
             : invoke(m_contentsView, "activeLogicalTextSnapshot")});
    return qMax(1, safeMap(metrics).value(QStringLiteral("lineCount")).toInt());
}

void ContentsDisplayGeometryInteraction::scheduleGutterRefresh(const int passCount, const QString& reason) const
{
    const QVariantMap plan = safeMap(invoke(m_refreshCoordinator, "scheduleGutterRefresh", {passCount, reason, activeLogicalLineCountSnapshot()}));
    if (plan.value(QStringLiteral("startTimer")).toBool())
        invokeVoid(m_eventPump, "startGutterRefreshTimer");
}

void ContentsDisplayGeometryInteraction::scheduleNoteEntryGutterRefresh(const QString& noteId) const
{
    invokeVoid(m_contentsView, "executeRefreshPlan", {invoke(m_refreshCoordinator, "scheduleNoteEntryGutterRefresh", {noteId})});
}

void ContentsDisplayGeometryInteraction::scheduleCursorDrivenUiRefresh() const
{
    const QVariantMap plan = safeMap(invoke(m_refreshCoordinator, "scheduleCursorDrivenUiRefresh"));
    if (!plan.value(QStringLiteral("queueCallLater")).toBool())
        return;
    QPointer<QObject> refreshCoordinator(m_refreshCoordinator);
    QPointer<QObject> minimapLayer(m_minimapLayer);
    QPointer<QObject> contentsView(m_contentsView);
    QTimer::singleShot(0, this, [this, refreshCoordinator, minimapLayer, contentsView]() {
        if (refreshCoordinator == nullptr || contentsView == nullptr)
            return;
        invokeVoid(refreshCoordinator, "clearCursorDrivenUiRefreshQueued");
        refreshMinimapCursorTracking({});
        if (minimapLayer != nullptr && property(contentsView, "minimapRefreshEnabled").toBool())
            invokeVoid(minimapLayer, "requestRepaint");
    });
}

void ContentsDisplayGeometryInteraction::scheduleViewportGutterRefresh() const
{
    const QVariantMap plan = safeMap(invoke(m_refreshCoordinator, "scheduleViewportGutterRefresh"));
    if (!plan.value(QStringLiteral("queueCallLater")).toBool())
        return;
    QPointer<QObject> refreshCoordinator(m_refreshCoordinator);
    QPointer<QObject> contentsView(m_contentsView);
    QTimer::singleShot(0, this, [this, refreshCoordinator, contentsView]() {
        if (refreshCoordinator == nullptr || contentsView == nullptr)
            return;
        invokeVoid(refreshCoordinator, "clearViewportGutterRefreshQueued");
        invokeVoid(contentsView, "refreshVisibleGutterEntries");
    });
}

void ContentsDisplayGeometryInteraction::scheduleMinimapSnapshotRefresh(const bool forceFull) const
{
    const QVariantMap plan = safeMap(invoke(m_refreshCoordinator, "scheduleMinimapSnapshotRefresh", {forceFull}));
    if (!plan.value(QStringLiteral("queueCallLater")).toBool())
        return;
    QPointer<QObject> refreshCoordinator(m_refreshCoordinator);
    QTimer::singleShot(0, this, [this, refreshCoordinator]() {
        if (refreshCoordinator == nullptr)
            return;
        invokeVoid(refreshCoordinator, "clearMinimapSnapshotRefreshQueued");
        refreshMinimapSnapshot();
    });
}

void ContentsDisplayGeometryInteraction::scrollEditorViewportToMinimapPosition(const double localY) const
{
    QObject* flickable = property(m_contentsView, "editorFlickable").value<QObject*>();
    if (flickable == nullptr)
        return;
    const QVariantMap plan = safeMap(invoke(m_viewportCoordinator, "minimapScrollPlan", {localY, qMax(1, invoke(m_contentsView, "editorOccupiedContentHeight").toInt())}));
    if (plan.value(QStringLiteral("apply")).toBool())
        flickable->setProperty("contentY", plan.value(QStringLiteral("contentY")));
}

void ContentsDisplayGeometryInteraction::correctTypingViewport(const bool forceAnchor) const
{
    QObject* flickable = property(m_contentsView, "editorFlickable").value<QObject*>();
    if (flickable == nullptr || !flickable->property("contentY").isValid())
        return;
    const QVariantMap plan = safeMap(invoke(
        m_viewportCoordinator,
        "typingViewportCorrectionPlan",
        {forceAnchor,
         flickable->property("height").isValid() ? flickable->property("height") : property(m_contentsView, "editorViewportHeight"),
         qMax(flickable->property("contentHeight").toInt(), invoke(m_contentsView, "editorOccupiedContentHeight").toInt()),
         flickable->property("contentY"),
         invoke(m_contentsView, "currentCursorVisualRowRect")}));
    if (plan.value(QStringLiteral("apply")).toBool())
        flickable->setProperty("contentY", plan.value(QStringLiteral("contentY")));
}

void ContentsDisplayGeometryInteraction::scheduleTypingViewportCorrection(const bool forceAnchor) const
{
    const QVariantMap plan = safeMap(invoke(m_refreshCoordinator, "scheduleTypingViewportCorrection", {forceAnchor}));
    if (!plan.value(QStringLiteral("queueCallLater")).toBool())
        return;
    QPointer<QObject> refreshCoordinator(m_refreshCoordinator);
    QTimer::singleShot(0, this, [this, refreshCoordinator]() {
        if (refreshCoordinator == nullptr)
            return;
        const bool forceCorrection = invokeBool(refreshCoordinator, "takeTypingViewportForceCorrectionRequested");
        invokeVoid(refreshCoordinator, "clearTypingViewportCorrectionQueued");
        correctTypingViewport(forceCorrection);
    });
}

QVariant ContentsDisplayGeometryInteraction::invoke(QObject* target, const char* methodName, const QVariantList& arguments) const
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

bool ContentsDisplayGeometryInteraction::invokeBool(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    return invoke(target, methodName, arguments).toBool();
}

void ContentsDisplayGeometryInteraction::invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments) const
{
    Q_UNUSED(invoke(target, methodName, arguments));
}

QVariant ContentsDisplayGeometryInteraction::property(QObject* target, const char* propertyName) const
{
    return target == nullptr || propertyName == nullptr ? QVariant() : target->property(propertyName);
}

void ContentsDisplayGeometryInteraction::setProperty(QObject* target, const char* propertyName, const QVariant& value) const
{
    if (target == nullptr || propertyName == nullptr)
        return;
    target->setProperty(propertyName, value);
}

QVariantMap ContentsDisplayGeometryInteraction::safeMap(const QVariant& value) const
{
    return value.toMap();
}
