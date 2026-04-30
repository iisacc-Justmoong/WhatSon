#include "app/models/editor/display/ContentsDisplayHostModePolicy.hpp"

#include <QFont>

ContentsDisplayHostModePolicy::ContentsDisplayHostModePolicy(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayHostModePolicy::~ContentsDisplayHostModePolicy() = default;

bool ContentsDisplayHostModePolicy::mobileHost() const noexcept { return m_mobileHost; }
bool ContentsDisplayHostModePolicy::minimapVisible() const noexcept { return m_minimapVisible; }
bool ContentsDisplayHostModePolicy::preferNativeInputHandling() const noexcept { return m_preferNativeInputHandling; }
bool ContentsDisplayHostModePolicy::showDedicatedResourceViewer() const noexcept { return m_showDedicatedResourceViewer; }
bool ContentsDisplayHostModePolicy::showFormattedTextRenderer() const noexcept { return m_showFormattedTextRenderer; }
bool ContentsDisplayHostModePolicy::showPrintEditorLayout() const noexcept { return m_showPrintEditorLayout; }
bool ContentsDisplayHostModePolicy::showStructuredDocumentFlow() const noexcept { return m_showStructuredDocumentFlow; }
int ContentsDisplayHostModePolicy::editorFontWeight() const noexcept { return m_mobileHost ? QFont::Medium : QFont::Normal; }
int ContentsDisplayHostModePolicy::editorHorizontalInset() const noexcept { return m_mobileHost ? 0 : 16; }
bool ContentsDisplayHostModePolicy::lineGeometryRefreshEnabled() const noexcept
{
    return !m_showDedicatedResourceViewer && !m_showFormattedTextRenderer && (!m_mobileHost || !m_showStructuredDocumentFlow);
}
bool ContentsDisplayHostModePolicy::showEditorGutter() const noexcept
{
    return !m_mobileHost && !m_showDedicatedResourceViewer && !m_showFormattedTextRenderer;
}
bool ContentsDisplayHostModePolicy::showMinimapRail() const noexcept
{
    return m_minimapVisible
        && !m_showDedicatedResourceViewer
        && !m_showPrintEditorLayout
        && !m_showFormattedTextRenderer
        && (!m_mobileHost || !m_showStructuredDocumentFlow);
}
bool ContentsDisplayHostModePolicy::minimapRefreshEnabled() const noexcept { return showMinimapRail(); }
bool ContentsDisplayHostModePolicy::structuredHostGeometryActive() const noexcept { return !m_mobileHost && m_showStructuredDocumentFlow; }

#define WHATSON_SETTER(name, field) \
void ContentsDisplayHostModePolicy::name(const bool value) \
{ \
    if (field == value) \
        return; \
    field = value; \
    emit stateChanged(); \
}

WHATSON_SETTER(setMobileHost, m_mobileHost)
WHATSON_SETTER(setMinimapVisible, m_minimapVisible)
WHATSON_SETTER(setPreferNativeInputHandling, m_preferNativeInputHandling)
WHATSON_SETTER(setShowDedicatedResourceViewer, m_showDedicatedResourceViewer)
WHATSON_SETTER(setShowFormattedTextRenderer, m_showFormattedTextRenderer)
WHATSON_SETTER(setShowPrintEditorLayout, m_showPrintEditorLayout)
WHATSON_SETTER(setShowStructuredDocumentFlow, m_showStructuredDocumentFlow)

#undef WHATSON_SETTER
