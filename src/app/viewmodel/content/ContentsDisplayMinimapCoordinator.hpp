#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ContentsDisplayMinimapCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive WRITE setStructuredHostGeometryActive NOTIFY structuredHostGeometryActiveChanged)
    Q_PROPERTY(double editorLineHeight READ editorLineHeight WRITE setEditorLineHeight NOTIFY editorLineHeightChanged)
    Q_PROPERTY(double editorDocumentStartY READ editorDocumentStartY WRITE setEditorDocumentStartY NOTIFY editorDocumentStartYChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)

public:
    explicit ContentsDisplayMinimapCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayMinimapCoordinator() override;

    bool structuredHostGeometryActive() const noexcept;
    void setStructuredHostGeometryActive(bool active);

    double editorLineHeight() const noexcept;
    void setEditorLineHeight(double value);

    double editorDocumentStartY() const noexcept;
    void setEditorDocumentStartY(double value);

    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);

    Q_INVOKABLE QVariantMap currentCursorVisualRowRectFromStructuredRect(const QVariantMap& rawRect, int fallbackLineNumber, double fallbackLineY, double fallbackLineHeight) const;
    Q_INVOKABLE QVariantMap currentCursorVisualRowRectFromTextRect(const QVariantMap& rawRect, int safeOffset, double fallbackDocumentY) const;
    Q_INVOKABLE QVariantList buildStructuredMinimapLineGroupsForRange(const QVariantList& lineEntries, int startLineNumber, int endLineNumber) const;
    Q_INVOKABLE QVariantList buildFallbackMinimapLineGroupsForRange(const QVariantList& lineCharacterCounts, const QVariantList& lineDocumentYs, const QVariantList& lineVisualHeights, int startLineNumber, int endLineNumber) const;
    Q_INVOKABLE QVariantList buildEditorMinimapLineGroupsForRange(const QVariantList& lineCharacterCounts, const QVariantList& lineStartOffsets, const QVariantList& fallbackLineDocumentYs, const QVariantList& fallbackLineVisualHeights, const QVariantList& editorRects, int logicalTextLength, int startLineNumber, int endLineNumber, double editorWidth, double editorContentHeight) const;
    Q_INVOKABLE QVariantMap buildNextMinimapSnapshotPlan(
        const QVariant& currentLineGroups,
        const QString& currentLineGroupsNoteId,
        const QString& currentNoteId,
        const QString& previousSourceText,
        const QString& currentSourceText,
        bool forceFullRefresh,
        bool noteEntryRefreshPending,
        int structuredLineCount,
        int plainLogicalLineCount) const;

signals:
    void structuredHostGeometryActiveChanged();
    void editorLineHeightChanged();
    void editorDocumentStartYChanged();
    void logicalLineCountChanged();

private:
    QVariantMap normalizeCursorRect(const QVariantMap& rawRect, double fallbackY, double fallbackHeight, bool preserveWidth) const;
    QVariantMap minimapGroup(int lineNumber, int charCount, double contentY, double contentHeight, const QString& minimapVisualKind = QStringLiteral("text"), int minimapRowCharCount = 0, int rowCount = 1) const;

    bool m_structuredHostGeometryActive = false;
    double m_editorLineHeight = 0.0;
    double m_editorDocumentStartY = 0.0;
    int m_logicalLineCount = 1;
};
