#include "app/models/editor/display/ContentsDisplayGeometryState.hpp"

#include <QtGlobal>

namespace
{
    template <typename T>
    bool assignIfChanged(T& target, const T& value)
    {
        if (target == value)
            return false;
        target = value;
        return true;
    }
}

ContentsDisplayGeometryState::ContentsDisplayGeometryState(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayGeometryState::~ContentsDisplayGeometryState() = default;

QVariantList ContentsDisplayGeometryState::gutterMarkers() const { return m_gutterMarkers; }
int ContentsDisplayGeometryState::gutterRefreshPassesRemaining() const noexcept { return m_gutterRefreshPassesRemaining; }
int ContentsDisplayGeometryState::gutterRefreshRevision() const noexcept { return m_gutterRefreshRevision; }
int ContentsDisplayGeometryState::liveLogicalLineCount() const noexcept { return m_liveLogicalLineCount; }
QVariantList ContentsDisplayGeometryState::logicalLineDocumentYCache() const { return m_logicalLineDocumentYCache; }
int ContentsDisplayGeometryState::logicalLineDocumentYCacheLineCount() const noexcept { return m_logicalLineDocumentYCacheLineCount; }
int ContentsDisplayGeometryState::logicalLineDocumentYCacheRevision() const noexcept { return m_logicalLineDocumentYCacheRevision; }
QVariantList ContentsDisplayGeometryState::logicalLineGutterDocumentYCache() const { return m_logicalLineGutterDocumentYCache; }
int ContentsDisplayGeometryState::logicalLineGutterDocumentYCacheLineCount() const noexcept { return m_logicalLineGutterDocumentYCacheLineCount; }
int ContentsDisplayGeometryState::logicalLineGutterDocumentYCacheRevision() const noexcept { return m_logicalLineGutterDocumentYCacheRevision; }
QString ContentsDisplayGeometryState::structuredGutterGeometrySignature() const { return m_structuredGutterGeometrySignature; }
QVariantList ContentsDisplayGeometryState::liveLogicalLineStartOffsets() const { return m_liveLogicalLineStartOffsets; }
int ContentsDisplayGeometryState::liveLogicalTextLength() const noexcept { return m_liveLogicalTextLength; }
QVariantList ContentsDisplayGeometryState::visibleGutterLineEntries() const { return m_visibleGutterLineEntries; }
QString ContentsDisplayGeometryState::pendingNoteEntryGutterRefreshNoteId() const { return m_pendingNoteEntryGutterRefreshNoteId; }
double ContentsDisplayGeometryState::minimapResolvedCurrentLineHeight() const noexcept { return m_minimapResolvedCurrentLineHeight; }
double ContentsDisplayGeometryState::minimapResolvedCurrentLineWidth() const noexcept { return m_minimapResolvedCurrentLineWidth; }
double ContentsDisplayGeometryState::minimapResolvedCurrentLineY() const noexcept { return m_minimapResolvedCurrentLineY; }
double ContentsDisplayGeometryState::minimapResolvedSilhouetteHeight() const noexcept { return m_minimapResolvedSilhouetteHeight; }
double ContentsDisplayGeometryState::minimapResolvedTrackHeight() const noexcept { return m_minimapResolvedTrackHeight; }
double ContentsDisplayGeometryState::minimapResolvedViewportHeight() const noexcept { return m_minimapResolvedViewportHeight; }
double ContentsDisplayGeometryState::minimapResolvedViewportY() const noexcept { return m_minimapResolvedViewportY; }
QVariantList ContentsDisplayGeometryState::minimapLineGroups() const { return m_minimapLineGroups; }
QString ContentsDisplayGeometryState::minimapLineGroupsNoteId() const { return m_minimapLineGroupsNoteId; }
bool ContentsDisplayGeometryState::minimapScrollable() const noexcept { return m_minimapScrollable; }
bool ContentsDisplayGeometryState::minimapVisible() const noexcept { return m_minimapVisible; }
QVariantList ContentsDisplayGeometryState::minimapVisualRows() const { return m_minimapVisualRows; }
bool ContentsDisplayGeometryState::cursorDrivenUiRefreshQueued() const noexcept { return m_cursorDrivenUiRefreshQueued; }
bool ContentsDisplayGeometryState::typingViewportCorrectionQueued() const noexcept { return m_typingViewportCorrectionQueued; }
bool ContentsDisplayGeometryState::typingViewportForceCorrectionRequested() const noexcept { return m_typingViewportForceCorrectionRequested; }
bool ContentsDisplayGeometryState::viewportGutterRefreshQueued() const noexcept { return m_viewportGutterRefreshQueued; }
bool ContentsDisplayGeometryState::minimapSnapshotRefreshQueued() const noexcept { return m_minimapSnapshotRefreshQueued; }

#define WHATSON_SETTER(name, field, type) \
void ContentsDisplayGeometryState::name(type value) \
{ \
    if (!assignIfChanged(field, value)) \
        return; \
    emit stateChanged(); \
}

WHATSON_SETTER(setGutterMarkers, m_gutterMarkers, const QVariantList&)
WHATSON_SETTER(setGutterRefreshPassesRemaining, m_gutterRefreshPassesRemaining, int)
WHATSON_SETTER(setGutterRefreshRevision, m_gutterRefreshRevision, int)
WHATSON_SETTER(setLiveLogicalLineCount, m_liveLogicalLineCount, int)
WHATSON_SETTER(setLogicalLineDocumentYCache, m_logicalLineDocumentYCache, const QVariantList&)
WHATSON_SETTER(setLogicalLineDocumentYCacheLineCount, m_logicalLineDocumentYCacheLineCount, int)
WHATSON_SETTER(setLogicalLineDocumentYCacheRevision, m_logicalLineDocumentYCacheRevision, int)
WHATSON_SETTER(setLogicalLineGutterDocumentYCache, m_logicalLineGutterDocumentYCache, const QVariantList&)
WHATSON_SETTER(setLogicalLineGutterDocumentYCacheLineCount, m_logicalLineGutterDocumentYCacheLineCount, int)
WHATSON_SETTER(setLogicalLineGutterDocumentYCacheRevision, m_logicalLineGutterDocumentYCacheRevision, int)
WHATSON_SETTER(setStructuredGutterGeometrySignature, m_structuredGutterGeometrySignature, const QString&)
WHATSON_SETTER(setLiveLogicalLineStartOffsets, m_liveLogicalLineStartOffsets, const QVariantList&)
WHATSON_SETTER(setLiveLogicalTextLength, m_liveLogicalTextLength, int)
WHATSON_SETTER(setVisibleGutterLineEntries, m_visibleGutterLineEntries, const QVariantList&)
WHATSON_SETTER(setPendingNoteEntryGutterRefreshNoteId, m_pendingNoteEntryGutterRefreshNoteId, const QString&)
WHATSON_SETTER(setMinimapResolvedCurrentLineHeight, m_minimapResolvedCurrentLineHeight, double)
WHATSON_SETTER(setMinimapResolvedCurrentLineWidth, m_minimapResolvedCurrentLineWidth, double)
WHATSON_SETTER(setMinimapResolvedCurrentLineY, m_minimapResolvedCurrentLineY, double)
WHATSON_SETTER(setMinimapResolvedSilhouetteHeight, m_minimapResolvedSilhouetteHeight, double)
WHATSON_SETTER(setMinimapResolvedTrackHeight, m_minimapResolvedTrackHeight, double)
WHATSON_SETTER(setMinimapResolvedViewportHeight, m_minimapResolvedViewportHeight, double)
WHATSON_SETTER(setMinimapResolvedViewportY, m_minimapResolvedViewportY, double)
WHATSON_SETTER(setMinimapLineGroups, m_minimapLineGroups, const QVariantList&)
WHATSON_SETTER(setMinimapLineGroupsNoteId, m_minimapLineGroupsNoteId, const QString&)
WHATSON_SETTER(setMinimapScrollable, m_minimapScrollable, bool)
WHATSON_SETTER(setMinimapVisible, m_minimapVisible, bool)
WHATSON_SETTER(setMinimapVisualRows, m_minimapVisualRows, const QVariantList&)
WHATSON_SETTER(setCursorDrivenUiRefreshQueued, m_cursorDrivenUiRefreshQueued, bool)
WHATSON_SETTER(setTypingViewportCorrectionQueued, m_typingViewportCorrectionQueued, bool)
WHATSON_SETTER(setTypingViewportForceCorrectionRequested, m_typingViewportForceCorrectionRequested, bool)
WHATSON_SETTER(setViewportGutterRefreshQueued, m_viewportGutterRefreshQueued, bool)
WHATSON_SETTER(setMinimapSnapshotRefreshQueued, m_minimapSnapshotRefreshQueued, bool)

#undef WHATSON_SETTER
