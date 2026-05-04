#pragma once

#include "app/models/editor/input/ContentsQmlBackedInputControllerBase.hpp"

#include <qqmlregistration.h>

class ContentsInlineFormatEditorController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsInlineFormatEditorController)
    Q_PROPERTY(QObject* control READ control WRITE setControl NOTIFY controlChanged FINAL)
    Q_PROPERTY(QObject* textInput READ textInput WRITE setTextInput NOTIFY textInputChanged FINAL)

public:
    explicit ContentsInlineFormatEditorController(QObject* parent = nullptr);

    QObject* control() const noexcept;
    void setControl(QObject* value);
    QObject* textInput() const noexcept;
    void setTextInput(QObject* value);

    Q_INVOKABLE void forceActiveFocus() const;
    Q_INVOKABLE QString currentPlainText() const;
    Q_INVOKABLE QVariantMap selectionSnapshot() const;
    Q_INVOKABLE void clearSelection() const;
    Q_INVOKABLE void clearCachedSelectionSnapshot() const;
    Q_INVOKABLE QVariantMap cacheCurrentSelectionSnapshot() const;
    Q_INVOKABLE void maybeDiscardCachedSelectionSnapshot() const;
    Q_INVOKABLE QVariantMap inlineFormatSelectionSnapshot() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE int clampLogicalPosition(int position, int maximumLength) const;
    Q_INVOKABLE int setCursorPositionPreservingNativeInput(int position) const;
    Q_INVOKABLE bool selectionCursorUsesStartEdge(int cursorPosition, int selectionStart, int selectionEnd) const;
    Q_INVOKABLE bool restoreSelectionRange(int selectionStart, int selectionEnd, int cursorPosition) const;
    Q_INVOKABLE QVariantMap programmaticTextSyncPolicy(const QString& nextText) const;
    Q_INVOKABLE bool canDeferProgrammaticTextSync(const QString& nextText) const;
    Q_INVOKABLE bool shouldRejectFocusedProgrammaticTextSync(const QString& nextText) const;
    Q_INVOKABLE void flushDeferredProgrammaticText(bool force) const;
    Q_INVOKABLE void clearDeferredProgrammaticText() const;
    Q_INVOKABLE bool dispatchCommittedTextEditedIfReady() const;
    Q_INVOKABLE void applyImmediateProgrammaticText(const QString& nextText) const;
    Q_INVOKABLE void setProgrammaticText(const QString& nextText) const;
    Q_INVOKABLE void scheduleCommittedTextEditedDispatch() const;

signals:
    void controlChanged();
    void textInputChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_control = nullptr;
    QObject* m_textInput = nullptr;
};
