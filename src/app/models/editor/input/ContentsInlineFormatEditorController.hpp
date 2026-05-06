#pragma once

#include "app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsInlineFormatEditorController : public QObject
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

private slots:
    void handleControlFocusedChanged();
    void handleNativeCompositionSettled();
    void handleTextInputCursorPositionChanged();
    void handleTextInputSelectionChanged();
    void handleTextInputTextChanged();

private:
    bool connectPropertyNotify(QObject* source, const char* propertyName, const char* slotSignature);
    bool controlFocused() const;
    bool nativeSelectionActive() const;
    bool invokeMoveCursorSelection(int position) const;
    bool invokeSelectRange(int selectionStart, int selectionEnd) const;
    bool setCursorPosition(int position) const;
    QString normalizedText(const QVariant& value) const;
    QVariantMap policyMap(const QString& action, bool apply, const QString& text, bool defer, bool reject) const;

    QPointer<QObject> m_control;
    QPointer<QObject> m_textInput;
    mutable int m_programmaticTextSyncDepth = 0;
    mutable bool m_hasDeferredProgrammaticText = false;
    mutable QString m_deferredProgrammaticText;
    mutable bool m_localSelectionInteractionSinceFocus = false;
    mutable bool m_localTextEditSinceFocus = false;
    ContentsEditorInputPolicyAdapter m_inputPolicyAdapter;
};
