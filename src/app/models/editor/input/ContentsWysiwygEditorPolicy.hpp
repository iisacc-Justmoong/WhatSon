#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsWysiwygEditorPolicy : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsWysiwygEditorPolicy)

public:
    explicit ContentsWysiwygEditorPolicy(QObject* parent = nullptr);

    Q_INVOKABLE int boundedOffset(int position, int length) const noexcept;
    Q_INVOKABLE QString normalizedSourceTagName(const QString& tagToken) const;
    Q_INVOKABLE bool sourceTagProducesVisibleSelection(const QString& tagName, bool closingTag) const;
    Q_INVOKABLE bool sourceOffsetIsInsideTagToken(const QString& sourceText, int sourceOffset) const;
    Q_INVOKABLE QVariantMap sourceTagTokenBoundsForCursor(const QString& sourceText, int sourceOffset) const;
    Q_INVOKABLE bool sourceRangeContainsVisibleLogicalContent(
        const QString& sourceText,
        int selectionStart,
        int selectionEnd) const;
    Q_INVOKABLE int htmlBlockSourceStart(const QVariant& block) const noexcept;
    Q_INVOKABLE int htmlBlockSourceEnd(const QVariant& block) const noexcept;
    Q_INVOKABLE bool htmlBlockIntersectsSourceRange(
        const QVariant& block,
        int selectionStart,
        int selectionEnd) const;
    Q_INVOKABLE bool htmlBlockIsAtomicResourceBlock(const QVariant& block) const;
    Q_INVOKABLE bool sourceRangeIntersectsAtomicResourceBlock(
        const QVariant& normalizedBlocks,
        int selectionStart,
        int selectionEnd) const;
    Q_INVOKABLE bool hasAtomicRenderedResourceBlocks(const QVariant& normalizedBlocks) const;
    Q_INVOKABLE QVariantMap resourceLogicalRangeForBlock(
        const QVariant& block,
        QObject* coordinateMapper) const;
    Q_INVOKABLE QVariantMap renderedLogicalSelectionRange(
        const QString& sourceText,
        const QVariant& normalizedBlocks,
        QObject* coordinateMapper,
        int selectionStart,
        int selectionEnd,
        int renderedLength) const;
    Q_INVOKABLE QVariantMap rawSelectionForVisibleSurfaceSelection(
        QObject* coordinateMapper,
        int surfaceSelectionStart,
        int surfaceSelectionEnd,
        int surfaceCursor,
        int renderedLength,
        int sourceLength) const;
    Q_INVOKABLE QVariantMap visibleContentSourceSelectionRange(
        const QString& sourceText,
        int selectionStart,
        int selectionEnd) const;
    Q_INVOKABLE QVariantMap visibleBackspaceMutationPayload(
        const QString& sourceText,
        QObject* coordinateMapper,
        int surfaceCursor,
        int renderedLength) const;
    Q_INVOKABLE QVariantMap visibleTextMutationPayload(
        const QString& sourceText,
        QObject* coordinateMapper,
        const QString& previousVisibleText,
        const QString& nextVisibleText,
        int surfaceCursor) const;
    Q_INVOKABLE QVariantMap visibleLogicalLineRange(
        const QString& visibleText,
        int logicalOffset) const;
    Q_INVOKABLE QVariantMap visibleLogicalParagraphRange(
        const QString& visibleText,
        int logicalOffset) const;
    Q_INVOKABLE QVariantMap hiddenTagCursorNormalizationPlan(
        const QString& sourceText,
        int currentCursorPosition,
        int previousRawCursorPosition,
        bool renderedOverlayVisible,
        bool nativeCompositionActive,
        bool nativeSelectionActive,
        bool visiblePointerCursorUpdateActive) const;

private:
    int sourceOffsetForVisibleLogicalOffset(
        QObject* coordinateMapper,
        int logicalOffset,
        int visibleLength) const;
    int logicalOffsetForSourceOffsetWithAffinity(
        QObject* coordinateMapper,
        int sourceOffset,
        bool preferAfter) const;
};
