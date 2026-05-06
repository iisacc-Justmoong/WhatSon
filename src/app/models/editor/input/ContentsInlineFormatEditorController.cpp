#include "app/models/editor/input/ContentsInlineFormatEditorController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QEvent>
#include <QKeyEvent>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>

#include <algorithm>

namespace
{
constexpr int selectCharactersMode = 0;

int clampedPosition(const int position, const int maximumLength)
{
    return std::clamp(position, 0, std::max(0, maximumLength));
}

bool invokeNoArg(QObject* object, const char* methodName)
{
    return object && QMetaObject::invokeMethod(object, methodName);
}

QString normalizedVariantText(const QVariant& value)
{
    QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.replace(QChar(0x2028), QLatin1Char('\n'));
    text.replace(QChar(0x2029), QLatin1Char('\n'));
    return text;
}

bool invokeTextEdited(QObject* control, const QString& text)
{
    if (!control)
        return false;

    if (QMetaObject::invokeMethod(control, "textEdited", Q_ARG(QString, text)))
        return true;

    return QMetaObject::invokeMethod(control, "textEdited", Q_ARG(QVariant, QVariant(text)));
}

bool invokeRenderedBackspaceMutation(QObject* control, const QKeyEvent& event)
{
    if (!control)
        return false;

    QVariantMap eventMap;
    eventMap.insert(QStringLiteral("accepted"), false);
    eventMap.insert(QStringLiteral("key"), event.key());
    eventMap.insert(QStringLiteral("modifiers"), static_cast<int>(event.modifiers()));

    QVariant handled;
    if (!QMetaObject::invokeMethod(
            control,
            "applyRenderedBackspaceMutation",
            Q_RETURN_ARG(QVariant, handled),
            Q_ARG(QVariant, QVariant(eventMap))))
    {
        return false;
    }
    return handled.toBool();
}
}

ContentsInlineFormatEditorController::ContentsInlineFormatEditorController(QObject* parent)
    : QObject(parent)
    , m_inputPolicyAdapter(this)
{
}

QObject* ContentsInlineFormatEditorController::control() const noexcept { return m_control.data(); }

void ContentsInlineFormatEditorController::setControl(QObject* value)
{
    if (m_control == value)
        return;

    if (m_control)
        QObject::disconnect(m_control, nullptr, this, nullptr);

    m_control = value;
    if (m_control)
    {
        connectPropertyNotify(m_control, "focused", "handleControlFocusedChanged()");
        QObject::connect(m_control, &QObject::destroyed, this, [this]() {
            if (m_control)
                return;
            emit controlChanged();
        });
    }

    emit controlChanged();
}

QObject* ContentsInlineFormatEditorController::textInput() const noexcept { return m_textInput.data(); }

void ContentsInlineFormatEditorController::setTextInput(QObject* value)
{
    if (m_textInput == value)
        return;

    if (m_textInput)
    {
        m_textInput->removeEventFilter(this);
        QObject::disconnect(m_textInput, nullptr, this, nullptr);
    }

    m_textInput = value;
    if (m_textInput)
    {
        m_textInput->installEventFilter(this);
        connectPropertyNotify(m_textInput, "cursorPosition", "handleTextInputCursorPositionChanged()");
        connectPropertyNotify(m_textInput, "selectionStart", "handleTextInputSelectionChanged()");
        connectPropertyNotify(m_textInput, "selectionEnd", "handleTextInputSelectionChanged()");
        connectPropertyNotify(m_textInput, "text", "handleTextInputTextChanged()");
        connectPropertyNotify(m_textInput, "inputMethodComposing", "handleNativeCompositionSettled()");
        connectPropertyNotify(m_textInput, "preeditText", "handleNativeCompositionSettled()");
        QObject::connect(m_textInput, &QObject::destroyed, this, [this]() {
            if (m_textInput)
                return;
            emit textInputChanged();
        });
    }

    emit textInputChanged();
}

bool ContentsInlineFormatEditorController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_textInput
        && event != nullptr
        && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Backspace
            && controlFocused()
            && invokeRenderedBackspaceMutation(m_control, *keyEvent))
        {
            keyEvent->accept();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ContentsInlineFormatEditorController::forceActiveFocus() const
{
    invokeNoArg(m_control, "forceActiveFocus");
    invokeNoArg(m_textInput, "forceActiveFocus");
}

QString ContentsInlineFormatEditorController::currentPlainText() const
{
    if (!m_textInput)
        return {};

    const int maximumLength = std::max(0, WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "length"));
    QString text;
    if (QMetaObject::invokeMethod(
            m_textInput,
            "getText",
            Q_RETURN_ARG(QString, text),
            Q_ARG(int, 0),
            Q_ARG(int, maximumLength)))
    {
        return normalizedText(text);
    }

    QVariant variantText;
    if (QMetaObject::invokeMethod(
            m_textInput,
            "getText",
            Q_RETURN_ARG(QVariant, variantText),
            Q_ARG(QVariant, QVariant(0)),
            Q_ARG(QVariant, QVariant(maximumLength))))
    {
        return normalizedText(variantText);
    }

    return normalizedText(WhatSon::Editor::DynamicObjectSupport::propertyValue(m_textInput, "text"));
}

QVariantMap ContentsInlineFormatEditorController::selectionSnapshot() const
{
    QVariantMap snapshot;
    if (!m_textInput)
        return snapshot;

    snapshot.insert(
        QStringLiteral("cursorPosition"),
        WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "cursorPosition"));
    snapshot.insert(
        QStringLiteral("selectionStart"),
        WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "selectionStart"));
    snapshot.insert(
        QStringLiteral("selectionEnd"),
        WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "selectionEnd"));
    snapshot.insert(
        QStringLiteral("selectedText"),
        WhatSon::Editor::DynamicObjectSupport::stringProperty(m_textInput, "selectedText"));
    return snapshot;
}

void ContentsInlineFormatEditorController::clearSelection() const
{
    invokeNoArg(m_textInput, "deselect");
}

void ContentsInlineFormatEditorController::clearCachedSelectionSnapshot() const {}

QVariantMap ContentsInlineFormatEditorController::cacheCurrentSelectionSnapshot() const
{
    return selectionSnapshot();
}

void ContentsInlineFormatEditorController::maybeDiscardCachedSelectionSnapshot() const {}

QVariantMap ContentsInlineFormatEditorController::inlineFormatSelectionSnapshot() const
{
    return selectionSnapshot();
}

bool ContentsInlineFormatEditorController::nativeCompositionActive() const
{
    return m_textInput
        && (WhatSon::Editor::DynamicObjectSupport::boolProperty(m_textInput, "inputMethodComposing")
            || !WhatSon::Editor::DynamicObjectSupport::stringProperty(m_textInput, "preeditText").isEmpty());
}

int ContentsInlineFormatEditorController::clampLogicalPosition(const int position, const int maximumLength) const
{
    return clampedPosition(position, maximumLength);
}

int ContentsInlineFormatEditorController::setCursorPositionPreservingNativeInput(const int position) const
{
    if (!m_textInput)
        return 0;
    if (nativeCompositionActive())
    {
        return WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "cursorPosition");
    }

    const int clampedCursor = clampLogicalPosition(
        position,
        WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "length"));
    setCursorPosition(clampedCursor);
    return WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "cursorPosition", clampedCursor);
}

bool ContentsInlineFormatEditorController::selectionCursorUsesStartEdge(
    const int cursorPosition,
    const int selectionStart,
    const int selectionEnd) const
{
    return cursorPosition == selectionStart && selectionStart != selectionEnd;
}

bool ContentsInlineFormatEditorController::restoreSelectionRange(
    const int selectionStart,
    const int selectionEnd,
    const int cursorPosition) const
{
    if (!m_textInput)
        return false;

    const int textLength = WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "length");
    const int start = clampLogicalPosition(selectionStart, textLength);
    const int end = std::max(start, clampLogicalPosition(selectionEnd, textLength));
    const int cursor = std::clamp(cursorPosition, start, end);
    if (start == end)
    {
        invokeNoArg(m_textInput, "deselect");
        setCursorPosition(cursor);
        return true;
    }

    if (cursor == start)
    {
        setCursorPosition(end);
        if (!invokeMoveCursorSelection(start))
            invokeSelectRange(start, end);
    }
    else
    {
        setCursorPosition(start);
        if (!invokeMoveCursorSelection(end))
            invokeSelectRange(start, end);
    }
    return true;
}

QVariantMap ContentsInlineFormatEditorController::programmaticTextSyncPolicy(const QString& nextText) const
{
    const QString text = normalizedText(nextText);
    if (!m_textInput)
        return policyMap(QStringLiteral("ignore"), false, text, false, false);

    const QString currentText = currentPlainText();
    if (currentText == text)
        return policyMap(QStringLiteral("noop"), false, text, false, false);

    const QVariantMap policy = m_inputPolicyAdapter.programmaticTextSyncPolicy(
        currentText,
        text,
        nativeCompositionActive(),
        controlFocused(),
        WhatSon::Editor::DynamicObjectSupport::boolProperty(m_control, "preferNativeInputHandling"),
        m_localTextEditSinceFocus,
        m_localSelectionInteractionSinceFocus || nativeSelectionActive());
    const bool apply = policy.value(QStringLiteral("apply")).toBool();
    const bool defer = policy.value(QStringLiteral("defer")).toBool();
    return policyMap(defer ? QStringLiteral("defer") : (apply ? QStringLiteral("apply") : QStringLiteral("reject")),
                     apply,
                     text,
                     defer,
                     !apply && !defer);
}

bool ContentsInlineFormatEditorController::canDeferProgrammaticTextSync(const QString& nextText) const
{
    return programmaticTextSyncPolicy(nextText).value(QStringLiteral("defer")).toBool();
}

bool ContentsInlineFormatEditorController::shouldRejectFocusedProgrammaticTextSync(const QString& nextText) const
{
    const QVariantMap policy = programmaticTextSyncPolicy(nextText);
    return policy.value(QStringLiteral("action")).toString() != QLatin1String("noop")
        && !policy.value(QStringLiteral("apply")).toBool();
}

void ContentsInlineFormatEditorController::flushDeferredProgrammaticText(const bool force) const
{
    if (!m_textInput || !m_hasDeferredProgrammaticText)
        return;
    if (!force && canDeferProgrammaticTextSync(m_deferredProgrammaticText))
        return;

    const QString deferredText = m_deferredProgrammaticText;
    clearDeferredProgrammaticText();
    if (force)
        applyImmediateProgrammaticText(deferredText);
    else
        setProgrammaticText(deferredText);
}

void ContentsInlineFormatEditorController::clearDeferredProgrammaticText() const
{
    m_hasDeferredProgrammaticText = false;
    m_deferredProgrammaticText.clear();
}

bool ContentsInlineFormatEditorController::dispatchCommittedTextEditedIfReady() const
{
    if (m_programmaticTextSyncDepth > 0)
        return false;
    if (!m_control || !m_textInput || nativeCompositionActive())
        return false;

    return invokeTextEdited(
        m_control,
        WhatSon::Editor::DynamicObjectSupport::stringProperty(m_textInput, "text"));
}

void ContentsInlineFormatEditorController::applyImmediateProgrammaticText(const QString& nextText) const
{
    clearDeferredProgrammaticText();
    if (!m_textInput)
        return;

    const QString normalizedNextText = normalizedText(nextText);
    if (currentPlainText() == normalizedNextText)
        return;

    const int previousCursorPosition = WhatSon::Editor::DynamicObjectSupport::intProperty(
        m_textInput,
        "cursorPosition");
    const int previousSelectionStart = WhatSon::Editor::DynamicObjectSupport::intProperty(
        m_textInput,
        "selectionStart");
    const int previousSelectionEnd = WhatSon::Editor::DynamicObjectSupport::intProperty(
        m_textInput,
        "selectionEnd");
    const bool hadSelection = previousSelectionEnd > previousSelectionStart;

    ++m_programmaticTextSyncDepth;
    m_textInput->setProperty("text", normalizedNextText);
    if (hadSelection)
    {
        restoreSelectionRange(previousSelectionStart, previousSelectionEnd, previousCursorPosition);
    }
    else
    {
        const int restoredCursorPosition = clampLogicalPosition(
            previousCursorPosition,
            WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "length"));
        invokeNoArg(m_textInput, "deselect");
        setCursorPositionPreservingNativeInput(restoredCursorPosition);
    }
    m_programmaticTextSyncDepth = std::max(0, m_programmaticTextSyncDepth - 1);
}

void ContentsInlineFormatEditorController::setProgrammaticText(const QString& nextText) const
{
    const QVariantMap policy = programmaticTextSyncPolicy(nextText);
    if (policy.value(QStringLiteral("defer")).toBool())
    {
        m_deferredProgrammaticText = policy.value(QStringLiteral("text")).toString();
        m_hasDeferredProgrammaticText = true;
        return;
    }
    if (!m_textInput
        || policy.value(QStringLiteral("reject")).toBool()
        || policy.value(QStringLiteral("action")).toString() == QLatin1String("noop"))
    {
        return;
    }

    applyImmediateProgrammaticText(policy.value(QStringLiteral("text")).toString());
}

void ContentsInlineFormatEditorController::scheduleCommittedTextEditedDispatch() const
{
    dispatchCommittedTextEditedIfReady();
}

void ContentsInlineFormatEditorController::handleControlFocusedChanged()
{
    if (controlFocused())
        return;

    flushDeferredProgrammaticText(true);
    m_localSelectionInteractionSinceFocus = false;
    m_localTextEditSinceFocus = false;
}

void ContentsInlineFormatEditorController::handleNativeCompositionSettled()
{
    if (!nativeCompositionActive())
        flushDeferredProgrammaticText(false);
}

void ContentsInlineFormatEditorController::handleTextInputCursorPositionChanged()
{
    if (controlFocused())
        m_localSelectionInteractionSinceFocus = true;
}

void ContentsInlineFormatEditorController::handleTextInputSelectionChanged()
{
    if (controlFocused())
        m_localSelectionInteractionSinceFocus = true;
}

void ContentsInlineFormatEditorController::handleTextInputTextChanged()
{
    if (m_programmaticTextSyncDepth > 0)
        return;
    if (controlFocused())
    {
        m_localSelectionInteractionSinceFocus = true;
        m_localTextEditSinceFocus = true;
    }
    clearDeferredProgrammaticText();
}

bool ContentsInlineFormatEditorController::connectPropertyNotify(
    QObject* source,
    const char* propertyName,
    const char* slotSignature)
{
    if (!source || !propertyName || !slotSignature)
        return false;

    const QMetaObject* sourceMetaObject = source->metaObject();
    const int propertyIndex = sourceMetaObject->indexOfProperty(propertyName);
    if (propertyIndex < 0)
        return false;

    const QMetaProperty property = sourceMetaObject->property(propertyIndex);
    if (!property.hasNotifySignal())
        return false;

    const int slotIndex = metaObject()->indexOfSlot(slotSignature);
    if (slotIndex < 0)
        return false;

    return QObject::connect(
        source,
        property.notifySignal(),
        this,
        metaObject()->method(slotIndex),
        Qt::UniqueConnection);
}

bool ContentsInlineFormatEditorController::controlFocused() const
{
    return WhatSon::Editor::DynamicObjectSupport::boolProperty(m_control, "focused");
}

bool ContentsInlineFormatEditorController::nativeSelectionActive() const
{
    if (!m_textInput)
        return false;
    return WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "selectionStart")
        != WhatSon::Editor::DynamicObjectSupport::intProperty(m_textInput, "selectionEnd");
}

bool ContentsInlineFormatEditorController::invokeMoveCursorSelection(const int position) const
{
    if (!m_textInput)
        return false;

    if (QMetaObject::invokeMethod(
            m_textInput,
            "moveCursorSelection",
            Q_ARG(int, position),
            Q_ARG(int, selectCharactersMode)))
    {
        return true;
    }

    return QMetaObject::invokeMethod(
        m_textInput,
        "moveCursorSelection",
        Q_ARG(QVariant, QVariant(position)),
        Q_ARG(QVariant, QVariant(selectCharactersMode)));
}

bool ContentsInlineFormatEditorController::invokeSelectRange(const int selectionStart, const int selectionEnd) const
{
    if (!m_textInput)
        return false;

    if (QMetaObject::invokeMethod(
            m_textInput,
            "select",
            Q_ARG(int, selectionStart),
            Q_ARG(int, selectionEnd)))
    {
        return true;
    }

    return QMetaObject::invokeMethod(
        m_textInput,
        "select",
        Q_ARG(QVariant, QVariant(selectionStart)),
        Q_ARG(QVariant, QVariant(selectionEnd)));
}

bool ContentsInlineFormatEditorController::setCursorPosition(const int position) const
{
    return m_textInput && m_textInput->setProperty("cursorPosition", position);
}

QString ContentsInlineFormatEditorController::normalizedText(const QVariant& value) const
{
    return normalizedVariantText(value);
}

QVariantMap ContentsInlineFormatEditorController::policyMap(
    const QString& action,
    const bool apply,
    const QString& text,
    const bool defer,
    const bool reject) const
{
    QVariantMap policy;
    policy.insert(QStringLiteral("action"), action);
    policy.insert(QStringLiteral("apply"), apply);
    policy.insert(QStringLiteral("text"), text);
    policy.insert(QStringLiteral("defer"), defer);
    policy.insert(QStringLiteral("reject"), reject);
    return policy;
}
