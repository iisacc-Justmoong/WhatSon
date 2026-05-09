#include "app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"

#include <Qt>
#include <QVariantMap>

namespace
{
constexpr int standardPrimaryShortcutModifierValue = static_cast<int>(Qt::ControlModifier);

QString normalizeTextValue(const QVariant& value)
{
    QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.replace(QChar(0x2028), QLatin1Char('\n'));
    text.replace(QChar(0x2029), QLatin1Char('\n'));
    return text;
}

bool shortcutKeyIsPureModifier(const int key) noexcept
{
    return key == Qt::Key_Alt
        || key == Qt::Key_Control
        || key == Qt::Key_Meta
        || key == Qt::Key_Shift;
}

int tagManagementShortcutKey(const int key) noexcept
{
    switch (key)
    {
    case Qt::Key_Ccedilla:
    case 0x00e7:
        return Qt::Key_C;
    case Qt::Key_Aring:
    case 0x00e5:
        return Qt::Key_A;
    case 0x222b:
        return Qt::Key_B;
    default:
        return key;
    }
}

int tagManagementShortcutKey(const int key, const QString& shortcutText)
{
    const QString text = shortcutText.trimmed();
    if (text.size() == 1)
    {
        switch (text.at(0).toLower().unicode())
        {
        case 0x00e7:
            return Qt::Key_C;
        case 0x00e5:
            return Qt::Key_A;
        case 0x222b:
            return Qt::Key_B;
        default:
            break;
        }
    }

    return tagManagementShortcutKey(key);
}

int primaryModifierCleared(const int modifiers) noexcept
{
    return modifiers
        & ~static_cast<int>(Qt::ControlModifier)
        & ~static_cast<int>(Qt::MetaModifier);
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

int ContentsEditorInputPolicyAdapter::standardPrimaryShortcutModifier() const noexcept
{
    return standardPrimaryShortcutModifierValue;
}

int ContentsEditorInputPolicyAdapter::platformPrimaryShortcutModifier(const QString& platformName) const noexcept
{
    Q_UNUSED(platformName);
    return standardPrimaryShortcutModifierValue;
}

int ContentsEditorInputPolicyAdapter::standardShortcutModifiers(
    const int nativeModifiers,
    const QString& platformName) const noexcept
{
    int standardModifiers = primaryModifierCleared(nativeModifiers);
    if ((nativeModifiers & platformPrimaryShortcutModifier(platformName)) != 0)
    {
        standardModifiers |= standardPrimaryShortcutModifierValue;
    }
    return standardModifiers;
}

int ContentsEditorInputPolicyAdapter::platformShortcutModifiers(
    const int standardModifiers,
    const QString& platformName) const noexcept
{
    int nativeModifiers = primaryModifierCleared(standardModifiers);
    if ((standardModifiers & standardPrimaryShortcutModifierValue) != 0)
    {
        nativeModifiers |= platformPrimaryShortcutModifier(platformName);
    }
    return nativeModifiers;
}

bool ContentsEditorInputPolicyAdapter::modifiersContainPlatformPrimaryShortcut(
    const int modifiers,
    const QString& platformName) const noexcept
{
    return (modifiers & platformPrimaryShortcutModifier(platformName)) != 0;
}

QString ContentsEditorInputPolicyAdapter::platformShortcutSequence(
    const QString& suffix,
    const QString& platformName) const
{
    Q_UNUSED(platformName);
    const QString keySuffix = suffix.trimmed();
    const QString primaryName = QStringLiteral("Ctrl");
    return keySuffix.isEmpty() ? primaryName : primaryName + QLatin1Char('+') + keySuffix;
}

QVariantMap ContentsEditorInputPolicyAdapter::tagManagementShortcutRequest(
    const int key,
    const int nativeModifiers,
    const QString& platformName) const
{
    return tagManagementShortcutRequestWithText(key, nativeModifiers, platformName, QString());
}

QVariantMap ContentsEditorInputPolicyAdapter::tagManagementShortcutRequestWithText(
    const int key,
    const int nativeModifiers,
    const QString& platformName,
    const QVariant& shortcutText) const
{
    const int shortcutKey = tagManagementShortcutKey(key, normalizedText(shortcutText));
    const int standardModifiers = standardShortcutModifiers(nativeModifiers, platformName);
    const bool pureModifierKey = shortcutKeyIsPureModifier(shortcutKey);
    const bool primaryHeld = (standardModifiers & standardPrimaryShortcutModifierValue) != 0;
    const bool optionHeld = (standardModifiers & static_cast<int>(Qt::AltModifier)) != 0;
    const bool shiftHeld = (standardModifiers & static_cast<int>(Qt::ShiftModifier)) != 0;
    const bool returnKey = shortcutKey == Qt::Key_Return || shortcutKey == Qt::Key_Enter;
    const bool inlineFormatShortcut = !pureModifierKey
        && primaryHeld
        && !optionHeld
        && (shortcutKey == Qt::Key_B
            || shortcutKey == Qt::Key_I
            || shortcutKey == Qt::Key_U
            || (shiftHeld && shortcutKey == Qt::Key_E));
    const bool bodyTagShortcut = !pureModifierKey
        && ((standardModifiers != 0 && returnKey) || (primaryHeld && optionHeld));

    return {
        {QStringLiteral("bodyTagShortcut"), bodyTagShortcut},
        {QStringLiteral("inlineFormatShortcut"), inlineFormatShortcut},
        {QStringLiteral("key"), shortcutKey},
        {QStringLiteral("nativeKey"), key},
        {QStringLiteral("nativeModifiers"), nativeModifiers},
        {QStringLiteral("platformName"), platformName},
        {QStringLiteral("platformPrimaryModifier"), platformPrimaryShortcutModifier(platformName)},
        {QStringLiteral("primaryHeld"), primaryHeld},
        {QStringLiteral("pureModifierKey"), pureModifierKey},
        {QStringLiteral("standardModifiers"), standardModifiers},
        {QStringLiteral("tagManagementShortcut"), inlineFormatShortcut || bodyTagShortcut}
    };
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
