#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsStructuredEditorFormattingController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsStructuredEditorFormattingController)

    Q_PROPERTY(QObject* blockRepeater READ blockRepeater WRITE setBlockRepeater NOTIFY blockRepeaterChanged FINAL)
    Q_PROPERTY(QObject* documentFlow READ documentFlow WRITE setDocumentFlow NOTIFY documentFlowChanged FINAL)

public:
    explicit ContentsStructuredEditorFormattingController(QObject* parent = nullptr);

    QObject* blockRepeater() const noexcept;
    void setBlockRepeater(QObject* value);

    QObject* documentFlow() const noexcept;
    void setDocumentFlow(QObject* value);

    Q_INVOKABLE QString normalizedInlineStyleTag(const QVariant& tagName) const;
    Q_INVOKABLE int adjustedCursorPositionForSelectionMutation(const QVariantMap& selectionSnapshot, int previousSelectionStart, int previousSelectionEnd, int nextSelectionStart, int nextSelectionEnd) const;
    Q_INVOKABLE bool selectionSnapshotIsValid(const QVariantMap& selectionSnapshot) const;
    Q_INVOKABLE bool cursorSnapshotIsValid(const QVariantMap& selectionSnapshot) const;
    Q_INVOKABLE int normalizedBlockIndex(const QVariant& blockIndex) const;
    Q_INVOKABLE QVariantMap interactiveBlockEntry(const QVariant& blockIndex) const;
    Q_INVOKABLE QObject* delegateItemForBlockIndex(const QVariant& blockIndex) const;
    Q_INVOKABLE QVariantMap blockSelectionSnapshot(const QVariant& blockIndex) const;
    Q_INVOKABLE bool blockDelegateFocused(const QVariant& blockIndex) const;
    Q_INVOKABLE QVariantMap blockInlineFormatTargetState(const QVariant& blockIndex) const;
    Q_INVOKABLE QVariantMap firstMountedSelectionTargetState() const;
    Q_INVOKABLE QVariantMap focusedCursorTargetState() const;
    Q_INVOKABLE QVariantMap inlineFormatTargetState() const;
    Q_INVOKABLE bool applyInlineFormatToBlockSelection(const QVariant& blockIndex, const QVariant& tagName, const QVariantMap& explicitSelectionSnapshot = {}) const;
    Q_INVOKABLE bool applyInlineFormatToActiveSelection(const QVariant& tagName) const;

signals:
    void blockRepeaterChanged();
    void documentFlowChanged();

private:
    bool applyInlineFormatAtCollapsedCursor(int blockIndex,
                                            const QString& tagName,
                                            const QVariantMap& selectionSnapshot,
                                            int blockSourceStart,
                                            int blockSourceEnd,
                                            const QString& blockSourceText,
                                            const QString& currentPlainText) const;

    QObject* m_blockRepeater = nullptr;
    QObject* m_documentFlow = nullptr;
};
