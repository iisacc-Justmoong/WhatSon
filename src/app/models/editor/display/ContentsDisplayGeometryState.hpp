#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsDisplayGeometryState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList gutterMarkers READ gutterMarkers WRITE setGutterMarkers NOTIFY stateChanged)
    Q_PROPERTY(int gutterRefreshPassesRemaining READ gutterRefreshPassesRemaining WRITE setGutterRefreshPassesRemaining NOTIFY stateChanged)
    Q_PROPERTY(int gutterRefreshRevision READ gutterRefreshRevision WRITE setGutterRefreshRevision NOTIFY stateChanged)
    Q_PROPERTY(int liveLogicalLineCount READ liveLogicalLineCount WRITE setLiveLogicalLineCount NOTIFY stateChanged)
    Q_PROPERTY(QVariantList logicalLineDocumentYCache READ logicalLineDocumentYCache WRITE setLogicalLineDocumentYCache NOTIFY stateChanged)
    Q_PROPERTY(int logicalLineDocumentYCacheLineCount READ logicalLineDocumentYCacheLineCount WRITE setLogicalLineDocumentYCacheLineCount NOTIFY stateChanged)
    Q_PROPERTY(int logicalLineDocumentYCacheRevision READ logicalLineDocumentYCacheRevision WRITE setLogicalLineDocumentYCacheRevision NOTIFY stateChanged)
    Q_PROPERTY(QVariantList logicalLineGutterDocumentYCache READ logicalLineGutterDocumentYCache WRITE setLogicalLineGutterDocumentYCache NOTIFY stateChanged)
    Q_PROPERTY(int logicalLineGutterDocumentYCacheLineCount READ logicalLineGutterDocumentYCacheLineCount WRITE setLogicalLineGutterDocumentYCacheLineCount NOTIFY stateChanged)
    Q_PROPERTY(int logicalLineGutterDocumentYCacheRevision READ logicalLineGutterDocumentYCacheRevision WRITE setLogicalLineGutterDocumentYCacheRevision NOTIFY stateChanged)
    Q_PROPERTY(QString structuredGutterGeometrySignature READ structuredGutterGeometrySignature WRITE setStructuredGutterGeometrySignature NOTIFY stateChanged)
    Q_PROPERTY(QVariantList liveLogicalLineStartOffsets READ liveLogicalLineStartOffsets WRITE setLiveLogicalLineStartOffsets NOTIFY stateChanged)
    Q_PROPERTY(int liveLogicalTextLength READ liveLogicalTextLength WRITE setLiveLogicalTextLength NOTIFY stateChanged)
    Q_PROPERTY(QVariantList visibleGutterLineEntries READ visibleGutterLineEntries WRITE setVisibleGutterLineEntries NOTIFY stateChanged)
    Q_PROPERTY(QString pendingNoteEntryGutterRefreshNoteId READ pendingNoteEntryGutterRefreshNoteId WRITE setPendingNoteEntryGutterRefreshNoteId NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedCurrentLineHeight READ minimapResolvedCurrentLineHeight WRITE setMinimapResolvedCurrentLineHeight NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedCurrentLineWidth READ minimapResolvedCurrentLineWidth WRITE setMinimapResolvedCurrentLineWidth NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedCurrentLineY READ minimapResolvedCurrentLineY WRITE setMinimapResolvedCurrentLineY NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedSilhouetteHeight READ minimapResolvedSilhouetteHeight WRITE setMinimapResolvedSilhouetteHeight NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedTrackHeight READ minimapResolvedTrackHeight WRITE setMinimapResolvedTrackHeight NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedViewportHeight READ minimapResolvedViewportHeight WRITE setMinimapResolvedViewportHeight NOTIFY stateChanged)
    Q_PROPERTY(double minimapResolvedViewportY READ minimapResolvedViewportY WRITE setMinimapResolvedViewportY NOTIFY stateChanged)
    Q_PROPERTY(QVariantList minimapLineGroups READ minimapLineGroups WRITE setMinimapLineGroups NOTIFY stateChanged)
    Q_PROPERTY(QString minimapLineGroupsNoteId READ minimapLineGroupsNoteId WRITE setMinimapLineGroupsNoteId NOTIFY stateChanged)
    Q_PROPERTY(bool minimapScrollable READ minimapScrollable WRITE setMinimapScrollable NOTIFY stateChanged)
    Q_PROPERTY(bool minimapVisible READ minimapVisible WRITE setMinimapVisible NOTIFY stateChanged)
    Q_PROPERTY(QVariantList minimapVisualRows READ minimapVisualRows WRITE setMinimapVisualRows NOTIFY stateChanged)
    Q_PROPERTY(bool cursorDrivenUiRefreshQueued READ cursorDrivenUiRefreshQueued WRITE setCursorDrivenUiRefreshQueued NOTIFY stateChanged)
    Q_PROPERTY(bool typingViewportCorrectionQueued READ typingViewportCorrectionQueued WRITE setTypingViewportCorrectionQueued NOTIFY stateChanged)
    Q_PROPERTY(bool typingViewportForceCorrectionRequested READ typingViewportForceCorrectionRequested WRITE setTypingViewportForceCorrectionRequested NOTIFY stateChanged)
    Q_PROPERTY(bool viewportGutterRefreshQueued READ viewportGutterRefreshQueued WRITE setViewportGutterRefreshQueued NOTIFY stateChanged)
    Q_PROPERTY(bool minimapSnapshotRefreshQueued READ minimapSnapshotRefreshQueued WRITE setMinimapSnapshotRefreshQueued NOTIFY stateChanged)

public:
    explicit ContentsDisplayGeometryState(QObject* parent = nullptr);
    ~ContentsDisplayGeometryState() override;

    QVariantList gutterMarkers() const;
    int gutterRefreshPassesRemaining() const noexcept;
    int gutterRefreshRevision() const noexcept;
    int liveLogicalLineCount() const noexcept;
    QVariantList logicalLineDocumentYCache() const;
    int logicalLineDocumentYCacheLineCount() const noexcept;
    int logicalLineDocumentYCacheRevision() const noexcept;
    QVariantList logicalLineGutterDocumentYCache() const;
    int logicalLineGutterDocumentYCacheLineCount() const noexcept;
    int logicalLineGutterDocumentYCacheRevision() const noexcept;
    QString structuredGutterGeometrySignature() const;
    QVariantList liveLogicalLineStartOffsets() const;
    int liveLogicalTextLength() const noexcept;
    QVariantList visibleGutterLineEntries() const;
    QString pendingNoteEntryGutterRefreshNoteId() const;
    double minimapResolvedCurrentLineHeight() const noexcept;
    double minimapResolvedCurrentLineWidth() const noexcept;
    double minimapResolvedCurrentLineY() const noexcept;
    double minimapResolvedSilhouetteHeight() const noexcept;
    double minimapResolvedTrackHeight() const noexcept;
    double minimapResolvedViewportHeight() const noexcept;
    double minimapResolvedViewportY() const noexcept;
    QVariantList minimapLineGroups() const;
    QString minimapLineGroupsNoteId() const;
    bool minimapScrollable() const noexcept;
    bool minimapVisible() const noexcept;
    QVariantList minimapVisualRows() const;
    bool cursorDrivenUiRefreshQueued() const noexcept;
    bool typingViewportCorrectionQueued() const noexcept;
    bool typingViewportForceCorrectionRequested() const noexcept;
    bool viewportGutterRefreshQueued() const noexcept;
    bool minimapSnapshotRefreshQueued() const noexcept;

public slots:
    void setGutterMarkers(const QVariantList& value);
    void setGutterRefreshPassesRemaining(int value);
    void setGutterRefreshRevision(int value);
    void setLiveLogicalLineCount(int value);
    void setLogicalLineDocumentYCache(const QVariantList& value);
    void setLogicalLineDocumentYCacheLineCount(int value);
    void setLogicalLineDocumentYCacheRevision(int value);
    void setLogicalLineGutterDocumentYCache(const QVariantList& value);
    void setLogicalLineGutterDocumentYCacheLineCount(int value);
    void setLogicalLineGutterDocumentYCacheRevision(int value);
    void setStructuredGutterGeometrySignature(const QString& value);
    void setLiveLogicalLineStartOffsets(const QVariantList& value);
    void setLiveLogicalTextLength(int value);
    void setVisibleGutterLineEntries(const QVariantList& value);
    void setPendingNoteEntryGutterRefreshNoteId(const QString& value);
    void setMinimapResolvedCurrentLineHeight(double value);
    void setMinimapResolvedCurrentLineWidth(double value);
    void setMinimapResolvedCurrentLineY(double value);
    void setMinimapResolvedSilhouetteHeight(double value);
    void setMinimapResolvedTrackHeight(double value);
    void setMinimapResolvedViewportHeight(double value);
    void setMinimapResolvedViewportY(double value);
    void setMinimapLineGroups(const QVariantList& value);
    void setMinimapLineGroupsNoteId(const QString& value);
    void setMinimapScrollable(bool value);
    void setMinimapVisible(bool value);
    void setMinimapVisualRows(const QVariantList& value);
    void setCursorDrivenUiRefreshQueued(bool value);
    void setTypingViewportCorrectionQueued(bool value);
    void setTypingViewportForceCorrectionRequested(bool value);
    void setViewportGutterRefreshQueued(bool value);
    void setMinimapSnapshotRefreshQueued(bool value);

signals:
    void stateChanged();

private:
    QVariantList m_gutterMarkers;
    int m_gutterRefreshPassesRemaining = 0;
    int m_gutterRefreshRevision = 0;
    int m_liveLogicalLineCount = 1;
    QVariantList m_logicalLineDocumentYCache;
    int m_logicalLineDocumentYCacheLineCount = 0;
    int m_logicalLineDocumentYCacheRevision = -1;
    QVariantList m_logicalLineGutterDocumentYCache;
    int m_logicalLineGutterDocumentYCacheLineCount = 0;
    int m_logicalLineGutterDocumentYCacheRevision = -1;
    QString m_structuredGutterGeometrySignature;
    QVariantList m_liveLogicalLineStartOffsets{0};
    int m_liveLogicalTextLength = 0;
    QVariantList m_visibleGutterLineEntries{QVariantMap{{QStringLiteral("lineNumber"), 1}, {QStringLiteral("y"), 0}}};
    QString m_pendingNoteEntryGutterRefreshNoteId;
    double m_minimapResolvedCurrentLineHeight = 1.0;
    double m_minimapResolvedCurrentLineWidth = 0.0;
    double m_minimapResolvedCurrentLineY = 0.0;
    double m_minimapResolvedSilhouetteHeight = 1.0;
    double m_minimapResolvedTrackHeight = 1.0;
    double m_minimapResolvedViewportHeight = 0.0;
    double m_minimapResolvedViewportY = 0.0;
    QVariantList m_minimapLineGroups;
    QString m_minimapLineGroupsNoteId;
    bool m_minimapScrollable = false;
    bool m_minimapVisible = true;
    QVariantList m_minimapVisualRows;
    bool m_cursorDrivenUiRefreshQueued = false;
    bool m_typingViewportCorrectionQueued = false;
    bool m_typingViewportForceCorrectionRequested = false;
    bool m_viewportGutterRefreshQueued = false;
    bool m_minimapSnapshotRefreshQueued = false;
};
