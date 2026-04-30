#include "app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"

#include <QVariantMap>

namespace
{
QString normalizeTextValue(const QVariant& value)
{
    QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.replace(QChar(0x2028), QLatin1Char('\n'));
    text.replace(QChar(0x2029), QLatin1Char('\n'));
    return text;
}
}

ContentsEditorInputPolicyAdapter::ContentsEditorInputPolicyAdapter(QObject* parent)
    : QObject(parent)
{
}

bool ContentsEditorInputPolicyAdapter::editorCompositionActive() const noexcept { return m_editorCompositionActive; }
bool ContentsEditorInputPolicyAdapter::editorInputFocused() const noexcept { return m_editorInputFocused; }
bool ContentsEditorInputPolicyAdapter::editorTagManagementInputEnabled() const noexcept { return m_editorTagManagementInputEnabled; }
bool ContentsEditorInputPolicyAdapter::nativeTextInputPriority() const noexcept { return m_nativeTextInputPriority; }
bool ContentsEditorInputPolicyAdapter::noteDocumentParseMounted() const noexcept { return m_noteDocumentParseMounted; }
bool ContentsEditorInputPolicyAdapter::structuredCompositionActive() const noexcept { return m_structuredCompositionActive; }

bool ContentsEditorInputPolicyAdapter::nativeCompositionActive() const noexcept
{
    return m_editorCompositionActive || m_structuredCompositionActive;
}

bool ContentsEditorInputPolicyAdapter::nativeTextInputSessionActive() const noexcept
{
    return nativeCompositionActive() || m_editorInputFocused;
}

bool ContentsEditorInputPolicyAdapter::commandTargetActive() const noexcept
{
    return m_noteDocumentParseMounted && m_nativeTextInputPriority;
}

bool ContentsEditorInputPolicyAdapter::shortcutSurfaceEnabled() const noexcept
{
    return commandTargetActive() && !nativeTextInputSessionActive();
}

bool ContentsEditorInputPolicyAdapter::tagManagementShortcutSurfaceEnabled() const noexcept
{
    return m_editorTagManagementInputEnabled && commandTargetActive() && !nativeCompositionActive();
}

bool ContentsEditorInputPolicyAdapter::contextMenuLongPressEnabled() const noexcept
{
    return commandTargetActive() && !nativeTextInputSessionActive();
}

bool ContentsEditorInputPolicyAdapter::contextMenuSurfaceEnabled() const noexcept
{
    return m_editorTagManagementInputEnabled && commandTargetActive() && !nativeCompositionActive();
}

QString ContentsEditorInputPolicyAdapter::normalizedText(const QVariant& value) const
{
    return normalizeTextValue(value);
}

QVariantMap ContentsEditorInputPolicyAdapter::programmaticTextSyncPolicy(
    const QVariant& currentText,
    const QVariant& nextText,
    const bool compositionActive,
    const bool focused,
    const bool preferNativeInputHandling,
    const bool localTextEditActive,
    const bool localSelectionInteractionActive) const
{
    const QString current = normalizeTextValue(currentText);
    const QString next = normalizeTextValue(nextText);
    const bool nativeFocusedLocalInteraction = preferNativeInputHandling && focused
        && (localTextEditActive || localSelectionInteractionActive);

    QVariantMap policy;
    if (compositionActive)
    {
        policy.insert(QStringLiteral("apply"), false);
        policy.insert(QStringLiteral("defer"), true);
        return policy;
    }
    if (nativeFocusedLocalInteraction)
    {
        policy.insert(QStringLiteral("apply"), false);
        policy.insert(QStringLiteral("defer"), current != next);
        return policy;
    }

    policy.insert(QStringLiteral("apply"), current != next);
    policy.insert(QStringLiteral("defer"), false);
    return policy;
}

bool ContentsEditorInputPolicyAdapter::shouldDeferProgrammaticTextSync(
    const QVariant& currentText,
    const QVariant& nextText,
    const bool compositionActive,
    const bool focused,
    const bool preferNativeInputHandling,
    const bool localTextEditActive,
    const bool localSelectionInteractionActive) const
{
    return programmaticTextSyncPolicy(
               currentText,
               nextText,
               compositionActive,
               focused,
               preferNativeInputHandling,
               localTextEditActive,
               localSelectionInteractionActive)
        .value(QStringLiteral("defer")).toBool();
}

bool ContentsEditorInputPolicyAdapter::shouldApplyProgrammaticTextSync(
    const QVariant& currentText,
    const QVariant& nextText,
    const bool compositionActive,
    const bool focused,
    const bool preferNativeInputHandling,
    const bool localTextEditActive,
    const bool localSelectionInteractionActive) const
{
    return programmaticTextSyncPolicy(
               currentText,
               nextText,
               compositionActive,
               focused,
               preferNativeInputHandling,
               localTextEditActive,
               localSelectionInteractionActive)
        .value(QStringLiteral("apply")).toBool();
}

bool ContentsEditorInputPolicyAdapter::shouldRestoreFocusForMutation(
    const QVariantMap& focusRequest,
    const QVariantMap& options) const
{
    if (focusRequest.isEmpty())
    {
        return false;
    }

    const bool compositionActive = options.contains(QStringLiteral("compositionActive"))
        ? options.value(QStringLiteral("compositionActive")).toBool()
        : nativeCompositionActive();
    if (compositionActive)
    {
        return false;
    }

    const bool nativePriority = options.contains(QStringLiteral("nativeTextInputPriority"))
        ? options.value(QStringLiteral("nativeTextInputPriority")).toBool()
        : m_nativeTextInputPriority;
    const bool focused = options.contains(QStringLiteral("editorInputFocused"))
        ? options.value(QStringLiteral("editorInputFocused")).toBool()
        : m_editorInputFocused;
    const QString reason = normalizeTextValue(focusRequest.value(QStringLiteral("reason"))).trimmed();

    return !(nativePriority && focused && reason == QLatin1String("text-edit"));
}

void ContentsEditorInputPolicyAdapter::setEditorCompositionActive(const bool value)
{
    if (m_editorCompositionActive == value)
        return;
    m_editorCompositionActive = value;
    emit editorCompositionActiveChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::setEditorInputFocused(const bool value)
{
    if (m_editorInputFocused == value)
        return;
    m_editorInputFocused = value;
    emit editorInputFocusedChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::setEditorTagManagementInputEnabled(const bool value)
{
    if (m_editorTagManagementInputEnabled == value)
        return;
    m_editorTagManagementInputEnabled = value;
    emit editorTagManagementInputEnabledChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::setNativeTextInputPriority(const bool value)
{
    if (m_nativeTextInputPriority == value)
        return;
    m_nativeTextInputPriority = value;
    emit nativeTextInputPriorityChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::setNoteDocumentParseMounted(const bool value)
{
    if (m_noteDocumentParseMounted == value)
        return;
    m_noteDocumentParseMounted = value;
    emit noteDocumentParseMountedChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::setStructuredCompositionActive(const bool value)
{
    if (m_structuredCompositionActive == value)
        return;
    m_structuredCompositionActive = value;
    emit structuredCompositionActiveChanged();
    emitDerivedStateChanged();
}

void ContentsEditorInputPolicyAdapter::emitDerivedStateChanged()
{
    emit derivedStateChanged();
}
