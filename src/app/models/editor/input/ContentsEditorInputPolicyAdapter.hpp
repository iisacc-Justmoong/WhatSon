#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsEditorInputPolicyAdapter : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorInputPolicyAdapter)

    Q_PROPERTY(bool editorCompositionActive READ editorCompositionActive WRITE setEditorCompositionActive NOTIFY editorCompositionActiveChanged FINAL)
    Q_PROPERTY(bool editorInputFocused READ editorInputFocused WRITE setEditorInputFocused NOTIFY editorInputFocusedChanged FINAL)
    Q_PROPERTY(bool editorTagManagementInputEnabled READ editorTagManagementInputEnabled WRITE setEditorTagManagementInputEnabled NOTIFY editorTagManagementInputEnabledChanged FINAL)
    Q_PROPERTY(bool nativeTextInputPriority READ nativeTextInputPriority WRITE setNativeTextInputPriority NOTIFY nativeTextInputPriorityChanged FINAL)
    Q_PROPERTY(bool noteDocumentParseMounted READ noteDocumentParseMounted WRITE setNoteDocumentParseMounted NOTIFY noteDocumentParseMountedChanged FINAL)
    Q_PROPERTY(bool structuredCompositionActive READ structuredCompositionActive WRITE setStructuredCompositionActive NOTIFY structuredCompositionActiveChanged FINAL)
    Q_PROPERTY(bool nativeCompositionActive READ nativeCompositionActive NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool nativeTextInputSessionActive READ nativeTextInputSessionActive NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool commandTargetActive READ commandTargetActive NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool shortcutSurfaceEnabled READ shortcutSurfaceEnabled NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool tagManagementShortcutSurfaceEnabled READ tagManagementShortcutSurfaceEnabled NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool contextMenuLongPressEnabled READ contextMenuLongPressEnabled NOTIFY derivedStateChanged FINAL)
    Q_PROPERTY(bool contextMenuSurfaceEnabled READ contextMenuSurfaceEnabled NOTIFY derivedStateChanged FINAL)

public:
    explicit ContentsEditorInputPolicyAdapter(QObject* parent = nullptr);

    bool editorCompositionActive() const noexcept;
    bool editorInputFocused() const noexcept;
    bool editorTagManagementInputEnabled() const noexcept;
    bool nativeTextInputPriority() const noexcept;
    bool noteDocumentParseMounted() const noexcept;
    bool structuredCompositionActive() const noexcept;

    bool nativeCompositionActive() const noexcept;
    bool nativeTextInputSessionActive() const noexcept;
    bool commandTargetActive() const noexcept;
    bool shortcutSurfaceEnabled() const noexcept;
    bool tagManagementShortcutSurfaceEnabled() const noexcept;
    bool contextMenuLongPressEnabled() const noexcept;
    bool contextMenuSurfaceEnabled() const noexcept;

    Q_INVOKABLE QString normalizedText(const QVariant& value) const;
    Q_INVOKABLE QVariantMap programmaticTextSyncPolicy(
        const QVariant& currentText,
        const QVariant& nextText,
        bool compositionActive,
        bool focused,
        bool preferNativeInputHandling,
        bool localTextEditActive,
        bool localSelectionInteractionActive) const;
    Q_INVOKABLE bool shouldDeferProgrammaticTextSync(
        const QVariant& currentText,
        const QVariant& nextText,
        bool compositionActive,
        bool focused,
        bool preferNativeInputHandling,
        bool localTextEditActive,
        bool localSelectionInteractionActive) const;
    Q_INVOKABLE bool shouldApplyProgrammaticTextSync(
        const QVariant& currentText,
        const QVariant& nextText,
        bool compositionActive,
        bool focused,
        bool preferNativeInputHandling,
        bool localTextEditActive,
        bool localSelectionInteractionActive) const;
    Q_INVOKABLE bool shouldRestoreFocusForMutation(const QVariantMap& focusRequest, const QVariantMap& options = {}) const;

public slots:
    void setEditorCompositionActive(bool value);
    void setEditorInputFocused(bool value);
    void setEditorTagManagementInputEnabled(bool value);
    void setNativeTextInputPriority(bool value);
    void setNoteDocumentParseMounted(bool value);
    void setStructuredCompositionActive(bool value);

signals:
    void editorCompositionActiveChanged();
    void editorInputFocusedChanged();
    void editorTagManagementInputEnabledChanged();
    void nativeTextInputPriorityChanged();
    void noteDocumentParseMountedChanged();
    void structuredCompositionActiveChanged();
    void derivedStateChanged();

private:
    void emitDerivedStateChanged();

    bool m_editorCompositionActive = false;
    bool m_editorInputFocused = false;
    bool m_editorTagManagementInputEnabled = true;
    bool m_nativeTextInputPriority = false;
    bool m_noteDocumentParseMounted = false;
    bool m_structuredCompositionActive = false;
};
