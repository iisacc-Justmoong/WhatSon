#pragma once

#include <QObject>

class ContentsDisplayHostModePolicy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool mobileHost READ mobileHost WRITE setMobileHost NOTIFY stateChanged)
    Q_PROPERTY(bool minimapVisible READ minimapVisible WRITE setMinimapVisible NOTIFY stateChanged)
    Q_PROPERTY(bool preferNativeInputHandling READ preferNativeInputHandling WRITE setPreferNativeInputHandling NOTIFY stateChanged)
    Q_PROPERTY(bool showDedicatedResourceViewer READ showDedicatedResourceViewer WRITE setShowDedicatedResourceViewer NOTIFY stateChanged)
    Q_PROPERTY(bool showFormattedTextRenderer READ showFormattedTextRenderer WRITE setShowFormattedTextRenderer NOTIFY stateChanged)
    Q_PROPERTY(bool showPrintEditorLayout READ showPrintEditorLayout WRITE setShowPrintEditorLayout NOTIFY stateChanged)
    Q_PROPERTY(bool showStructuredDocumentFlow READ showStructuredDocumentFlow WRITE setShowStructuredDocumentFlow NOTIFY stateChanged)
    Q_PROPERTY(int editorFontWeight READ editorFontWeight NOTIFY stateChanged)
    Q_PROPERTY(int editorHorizontalInset READ editorHorizontalInset NOTIFY stateChanged)
    Q_PROPERTY(bool lineGeometryRefreshEnabled READ lineGeometryRefreshEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool showEditorGutter READ showEditorGutter NOTIFY stateChanged)
    Q_PROPERTY(bool showMinimapRail READ showMinimapRail NOTIFY stateChanged)
    Q_PROPERTY(bool minimapRefreshEnabled READ minimapRefreshEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive NOTIFY stateChanged)

public:
    explicit ContentsDisplayHostModePolicy(QObject* parent = nullptr);
    ~ContentsDisplayHostModePolicy() override;

    bool mobileHost() const noexcept;
    bool minimapVisible() const noexcept;
    bool preferNativeInputHandling() const noexcept;
    bool showDedicatedResourceViewer() const noexcept;
    bool showFormattedTextRenderer() const noexcept;
    bool showPrintEditorLayout() const noexcept;
    bool showStructuredDocumentFlow() const noexcept;

    int editorFontWeight() const noexcept;
    int editorHorizontalInset() const noexcept;
    bool lineGeometryRefreshEnabled() const noexcept;
    bool showEditorGutter() const noexcept;
    bool showMinimapRail() const noexcept;
    bool minimapRefreshEnabled() const noexcept;
    bool structuredHostGeometryActive() const noexcept;

public slots:
    void setMobileHost(bool value);
    void setMinimapVisible(bool value);
    void setPreferNativeInputHandling(bool value);
    void setShowDedicatedResourceViewer(bool value);
    void setShowFormattedTextRenderer(bool value);
    void setShowPrintEditorLayout(bool value);
    void setShowStructuredDocumentFlow(bool value);

signals:
    void stateChanged();

private:
    bool m_mobileHost = false;
    bool m_minimapVisible = true;
    bool m_preferNativeInputHandling = false;
    bool m_showDedicatedResourceViewer = false;
    bool m_showFormattedTextRenderer = false;
    bool m_showPrintEditorLayout = false;
    bool m_showStructuredDocumentFlow = false;
};
