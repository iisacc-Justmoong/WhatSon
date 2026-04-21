#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

class ContentsDisplayViewportCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive WRITE setStructuredHostGeometryActive NOTIFY structuredHostGeometryActiveChanged)
    Q_PROPERTY(bool showPrintEditorLayout READ showPrintEditorLayout WRITE setShowPrintEditorLayout NOTIFY showPrintEditorLayoutChanged)
    Q_PROPERTY(bool editorInputFocused READ editorInputFocused WRITE setEditorInputFocused NOTIFY editorInputFocusedChanged)
    Q_PROPERTY(double editorLineHeight READ editorLineHeight WRITE setEditorLineHeight NOTIFY editorLineHeightChanged)
    Q_PROPERTY(double editorSurfaceHeight READ editorSurfaceHeight WRITE setEditorSurfaceHeight NOTIFY editorSurfaceHeightChanged)
    Q_PROPERTY(double editorDocumentStartY READ editorDocumentStartY WRITE setEditorDocumentStartY NOTIFY editorDocumentStartYChanged)
    Q_PROPERTY(double editorViewportHeight READ editorViewportHeight WRITE setEditorViewportHeight NOTIFY editorViewportHeightChanged)
    Q_PROPERTY(double editorContentOffsetY READ editorContentOffsetY WRITE setEditorContentOffsetY NOTIFY editorContentOffsetYChanged)
    Q_PROPERTY(double minimapResolvedTrackHeight READ minimapResolvedTrackHeight WRITE setMinimapResolvedTrackHeight NOTIFY minimapResolvedTrackHeightChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(int currentCursorLineNumber READ currentCursorLineNumber WRITE setCurrentCursorLineNumber NOTIFY currentCursorLineNumberChanged)
    Q_PROPERTY(int currentCursorOffset READ currentCursorOffset WRITE setCurrentCursorOffset NOTIFY currentCursorOffsetChanged)
    Q_PROPERTY(int logicalTextLength READ logicalTextLength WRITE setLogicalTextLength NOTIFY logicalTextLengthChanged)

public:
    explicit ContentsDisplayViewportCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayViewportCoordinator() override;

    bool structuredHostGeometryActive() const noexcept;
    void setStructuredHostGeometryActive(bool active);

    bool showPrintEditorLayout() const noexcept;
    void setShowPrintEditorLayout(bool active);

    bool editorInputFocused() const noexcept;
    void setEditorInputFocused(bool focused);

    double editorLineHeight() const noexcept;
    void setEditorLineHeight(double value);

    double editorSurfaceHeight() const noexcept;
    void setEditorSurfaceHeight(double value);

    double editorDocumentStartY() const noexcept;
    void setEditorDocumentStartY(double value);

    double editorViewportHeight() const noexcept;
    void setEditorViewportHeight(double value);

    double editorContentOffsetY() const noexcept;
    void setEditorContentOffsetY(double value);

    double minimapResolvedTrackHeight() const noexcept;
    void setMinimapResolvedTrackHeight(double value);

    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);

    int currentCursorLineNumber() const noexcept;
    void setCurrentCursorLineNumber(int value);

    int currentCursorOffset() const noexcept;
    void setCurrentCursorOffset(int value);

    int logicalTextLength() const noexcept;
    void setLogicalTextLength(int value);

    Q_INVOKABLE QString normalizedNoteId(const QString& noteId) const;
    Q_INVOKABLE bool hasPendingNoteEntryGutterRefresh(const QString& pendingNoteId, const QString& noteId = QString()) const;
    Q_INVOKABLE QVariantMap finalizePendingNoteEntryGutterRefresh(
        const QString& pendingNoteId,
        const QString& selectedNoteId,
        bool selectedNoteBodyLoading,
        const QString& reason,
        bool refreshStructuredLayout) const;
    Q_INVOKABLE QVariantMap buildLogicalLineMetricsFromStructuredEntries(const QVariantList& lineEntries) const;
    Q_INVOKABLE QVariantMap buildLogicalLineMetricsFromText(const QString& text) const;
    Q_INVOKABLE QString structuredGutterGeometrySignature(const QVariantList& lineEntries) const;
    Q_INVOKABLE QVariantMap consumeStructuredGutterGeometryChange(const QString& previousSignature, const QVariantList& lineEntries) const;
    Q_INVOKABLE QVariantMap minimapScrollPlan(double localY, double contentHeight) const;
    Q_INVOKABLE QVariantMap typingViewportCorrectionPlan(
        bool forceAnchor,
        double flickableHeight,
        double flickableContentHeight,
        double currentContentY,
        const QVariantMap& cursorRect) const;

signals:
    void structuredHostGeometryActiveChanged();
    void showPrintEditorLayoutChanged();
    void editorInputFocusedChanged();
    void editorLineHeightChanged();
    void editorSurfaceHeightChanged();
    void editorDocumentStartYChanged();
    void editorViewportHeightChanged();
    void editorContentOffsetYChanged();
    void minimapResolvedTrackHeightChanged();
    void logicalLineCountChanged();
    void currentCursorLineNumberChanged();
    void currentCursorOffsetChanged();
    void logicalTextLengthChanged();

private:
    static double clampUnit(double value) noexcept;
    double typingViewportBandTop(double cursorHeight) const noexcept;
    double typingViewportBandBottom(double cursorHeight) const noexcept;
    double typingViewportAnchorCenter(double cursorHeight) const noexcept;
    double editorViewportYForDocumentY(double documentY) const noexcept;

    bool m_structuredHostGeometryActive = false;
    bool m_showPrintEditorLayout = false;
    bool m_editorInputFocused = false;
    double m_editorLineHeight = 0.0;
    double m_editorSurfaceHeight = 0.0;
    double m_editorDocumentStartY = 0.0;
    double m_editorViewportHeight = 0.0;
    double m_editorContentOffsetY = 0.0;
    double m_minimapResolvedTrackHeight = 1.0;
    int m_logicalLineCount = 1;
    int m_currentCursorLineNumber = 1;
    int m_currentCursorOffset = 0;
    int m_logicalTextLength = 0;
};
