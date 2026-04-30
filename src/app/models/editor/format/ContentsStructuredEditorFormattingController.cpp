#include "app/models/editor/format/ContentsStructuredEditorFormattingController.hpp"

#include <QMetaObject>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QVariantList>

namespace
{
QString normalizedText(const QVariant& value)
{
    QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.replace(QChar(0x2028), QLatin1Char('\n'));
    text.replace(QChar(0x2029), QLatin1Char('\n'));
    text.replace(QChar(0x00A0), QLatin1Char(' '));
    return text;
}

QVariant readProperty(QObject* object, const char* name)
{
    return object ? object->property(name) : QVariant{};
}

QVariantMap mapValue(const QVariant& value)
{
    return value.metaType().id() == QMetaType::QVariantMap ? value.toMap() : QVariantMap{};
}

QVariantList listValue(const QVariant& value)
{
    return value.metaType().id() == QMetaType::QVariantList ? value.toList() : QVariantList{};
}

QString normalizeInlineStyleTagName(const QVariant& rawTagName)
{
    const QString tag = normalizedText(rawTagName).trimmed().toLower();
    if (tag == QLatin1String("plain") || tag == QLatin1String("clear") || tag == QLatin1String("none"))
        return QStringLiteral("plain");
    if (tag == QLatin1String("bold") || tag == QLatin1String("b") || tag == QLatin1String("strong"))
        return QStringLiteral("bold");
    if (tag == QLatin1String("italic") || tag == QLatin1String("i") || tag == QLatin1String("em"))
        return QStringLiteral("italic");
    if (tag == QLatin1String("underline") || tag == QLatin1String("u"))
        return QStringLiteral("underline");
    if (tag == QLatin1String("strikethrough") || tag == QLatin1String("strike") || tag == QLatin1String("s") || tag == QLatin1String("del"))
        return QStringLiteral("strikethrough");
    if (tag == QLatin1String("highlight") || tag == QLatin1String("mark"))
        return QStringLiteral("highlight");
    return {};
}

QString plainTextFromInlineTaggedSource(const QString& sourceText)
{
    QString plainText;
    int offset = 0;
    static const QRegularExpression entityPattern(QStringLiteral(R"(^&(?:#[0-9]+|#x[0-9A-Fa-f]+|[A-Za-z][A-Za-z0-9]{0,31});)"));
    while (offset < sourceText.length())
    {
        if (sourceText.at(offset) == QLatin1Char('<'))
        {
            const int end = sourceText.indexOf(QLatin1Char('>'), offset);
            if (end >= 0)
            {
                const QString token = sourceText.mid(offset, end - offset + 1);
                static const QRegularExpression tagPattern(QStringLiteral(R"(^<\s*(/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>$)"));
                const QRegularExpressionMatch match = tagPattern.match(token);
                if (match.hasMatch())
                {
                    const QString name = normalizeInlineStyleTagName(match.captured(2));
                    if (!name.isEmpty())
                    {
                        offset = end + 1;
                        continue;
                    }
                }
            }
        }
        if (sourceText.at(offset) == QLatin1Char('&'))
        {
            const QRegularExpressionMatch match = entityPattern.match(sourceText.mid(offset));
            if (match.hasMatch())
            {
                const QString token = match.captured(0);
                if (token == QLatin1String("&amp;")) plainText += QLatin1Char('&');
                else if (token == QLatin1String("&lt;")) plainText += QLatin1Char('<');
                else if (token == QLatin1String("&gt;")) plainText += QLatin1Char('>');
                else if (token == QLatin1String("&quot;")) plainText += QLatin1Char('"');
                else if (token == QLatin1String("&#39;") || token == QLatin1String("&apos;")) plainText += QLatin1Char('\'');
                else if (token == QLatin1String("&nbsp;")) plainText += QLatin1Char(' ');
                else plainText += token;
                offset += token.length();
                continue;
            }
        }
        plainText += sourceText.at(offset);
        ++offset;
    }
    return normalizedText(plainText);
}

int sourceOffsetForInlineTaggedLogicalOffset(const QString& sourceText, const int logicalOffset)
{
    int visible = 0;
    int offset = 0;
    static const QRegularExpression entityPattern(QStringLiteral(R"(^&(?:#[0-9]+|#x[0-9A-Fa-f]+|[A-Za-z][A-Za-z0-9]{0,31});)"));
    static const QRegularExpression tagPattern(QStringLiteral(R"(^<\s*(/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>$)"));
    while (offset < sourceText.length() && visible < logicalOffset)
    {
        if (sourceText.at(offset) == QLatin1Char('<'))
        {
            const int end = sourceText.indexOf(QLatin1Char('>'), offset);
            if (end >= 0)
            {
                const QRegularExpressionMatch tagMatch = tagPattern.match(sourceText.mid(offset, end - offset + 1));
                if (tagMatch.hasMatch() && !normalizeInlineStyleTagName(tagMatch.captured(2)).isEmpty())
                {
                    offset = end + 1;
                    continue;
                }
            }
        }
        if (sourceText.at(offset) == QLatin1Char('&'))
        {
            const QRegularExpressionMatch entityMatch = entityPattern.match(sourceText.mid(offset));
            if (entityMatch.hasMatch())
            {
                offset += entityMatch.captured(0).length();
                ++visible;
                continue;
            }
        }
        ++offset;
        ++visible;
    }
    while (offset < sourceText.length() && sourceText.at(offset) == QLatin1Char('<'))
    {
        const int end = sourceText.indexOf(QLatin1Char('>'), offset);
        if (end < 0)
            break;
        const QRegularExpressionMatch tagMatch = tagPattern.match(sourceText.mid(offset, end - offset + 1));
        if (!tagMatch.hasMatch() || tagMatch.captured(1) != QLatin1String("/"))
            break;
        if (normalizeInlineStyleTagName(tagMatch.captured(2)).isEmpty())
            break;
        offset = end + 1;
    }
    return offset;
}

int invokeInt(QObject* object, const char* method, const QVariant& arg = QVariant{})
{
    int result = -1;
    if (!object)
        return result;
    if (arg.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(int, result), Q_ARG(QVariant, arg));
    else
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(int, result));
    return result;
}

QVariant invokeVariant(QObject* object, const char* method, const QVariant& arg1 = QVariant(), const QVariant& arg2 = QVariant(), const QVariant& arg3 = QVariant())
{
    QVariant result;
    if (!object)
        return result;
    if (arg1.isValid() && arg2.isValid() && arg3.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2), Q_ARG(QVariant, arg3));
    else if (arg1.isValid() && arg2.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2));
    else if (arg1.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(QVariant, result), Q_ARG(QVariant, arg1));
    else
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(QVariant, result));
    return result;
}

bool invokeBool(QObject* object, const char* method,
                const QVariant& arg1 = QVariant(), const QVariant& arg2 = QVariant(),
                const QVariant& arg3 = QVariant(), const QVariant& arg4 = QVariant())
{
    bool result = false;
    if (!object)
        return false;
    if (arg1.isValid() && arg2.isValid() && arg3.isValid() && arg4.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(bool, result), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2), Q_ARG(QVariant, arg3), Q_ARG(QVariant, arg4));
    else if (arg1.isValid() && arg2.isValid() && arg3.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(bool, result), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2), Q_ARG(QVariant, arg3));
    else if (arg1.isValid() && arg2.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(bool, result), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2));
    else if (arg1.isValid())
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(bool, result), Q_ARG(QVariant, arg1));
    else
        QMetaObject::invokeMethod(object, method, Q_RETURN_ARG(bool, result));
    return result;
}

int intValueOr(const QVariant& value, const int fallback)
{
    bool ok = false;
    const int converted = value.toInt(&ok);
    return ok ? converted : fallback;
}

QVariantMap buildInlineStyleSelectionPayload(const QString& sourceText, const int selectionStart, const int selectionEnd, const QString& styleTag)
{
    QVariantMap payload;
    const QString normalizedStyleTag = normalizeInlineStyleTagName(styleTag);
    if (sourceText.isEmpty() || normalizedStyleTag.isEmpty() || selectionEnd <= selectionStart)
    {
        payload.insert(QStringLiteral("applied"), false);
        return payload;
    }
    const int sourceStart = sourceOffsetForInlineTaggedLogicalOffset(sourceText, selectionStart);
    const int sourceEnd = sourceOffsetForInlineTaggedLogicalOffset(sourceText, selectionEnd);
    if (normalizedStyleTag == QLatin1String("plain"))
    {
        payload.insert(QStringLiteral("applied"), false);
        return payload;
    }
    const QString nextSourceText = sourceText.left(sourceStart)
        + QStringLiteral("<%1>").arg(normalizedStyleTag)
        + sourceText.mid(sourceStart, sourceEnd - sourceStart)
        + QStringLiteral("</%1>").arg(normalizedStyleTag)
        + sourceText.mid(sourceEnd);
    payload.insert(QStringLiteral("applied"), nextSourceText != sourceText);
    payload.insert(QStringLiteral("nextSourceText"), nextSourceText);
    return payload;
}
}

ContentsStructuredEditorFormattingController::ContentsStructuredEditorFormattingController(QObject* parent)
    : QObject(parent)
{
}

QObject* ContentsStructuredEditorFormattingController::blockRepeater() const noexcept { return m_blockRepeater; }
void ContentsStructuredEditorFormattingController::setBlockRepeater(QObject* value)
{
    if (m_blockRepeater == value)
        return;
    m_blockRepeater = value;
    emit blockRepeaterChanged();
}

QObject* ContentsStructuredEditorFormattingController::documentFlow() const noexcept { return m_documentFlow; }
void ContentsStructuredEditorFormattingController::setDocumentFlow(QObject* value)
{
    if (m_documentFlow == value)
        return;
    m_documentFlow = value;
    emit documentFlowChanged();
}

QString ContentsStructuredEditorFormattingController::normalizedInlineStyleTag(const QVariant& tagName) const
{
    return normalizeInlineStyleTagName(tagName);
}

int ContentsStructuredEditorFormattingController::adjustedCursorPositionForSelectionMutation(const QVariantMap& selectionSnapshot, const int previousSelectionStart, const int previousSelectionEnd, const int nextSelectionStart, const int nextSelectionEnd) const
{
    const int previousCursor = qBound(previousSelectionStart,
                                      intValueOr(selectionSnapshot.value(QStringLiteral("cursorPosition")), previousSelectionEnd),
                                      previousSelectionEnd);
    return qBound(nextSelectionStart, nextSelectionStart + qMax(0, previousCursor - previousSelectionStart), nextSelectionEnd);
}

bool ContentsStructuredEditorFormattingController::selectionSnapshotIsValid(const QVariantMap& selectionSnapshot) const
{
    return m_documentFlow && invokeBool(m_documentFlow, "selectionSnapshotIsValid", selectionSnapshot);
}

bool ContentsStructuredEditorFormattingController::cursorSnapshotIsValid(const QVariantMap& selectionSnapshot) const
{
    if (selectionSnapshot.isEmpty())
        return false;
    if (selectionSnapshot.value(QStringLiteral("cursorPosition")).canConvert<int>())
        return true;
    return selectionSnapshot.value(QStringLiteral("selectionStart")).canConvert<int>()
        || selectionSnapshot.value(QStringLiteral("selectionEnd")).canConvert<int>();
}

int ContentsStructuredEditorFormattingController::normalizedBlockIndex(const QVariant& blockIndex) const
{
    const QVariantList blocks = listValue(invokeVariant(m_documentFlow, "normalizedBlocks"));
    if (blocks.isEmpty())
        return -1;
    bool ok = false;
    const int index = blockIndex.toInt(&ok);
    if (!ok)
        return -1;
    return qBound(0, index, blocks.size() - 1);
}

QVariantMap ContentsStructuredEditorFormattingController::interactiveBlockEntry(const QVariant& blockIndex) const
{
    const int safeIndex = normalizedBlockIndex(blockIndex);
    const QVariantList blocks = listValue(invokeVariant(m_documentFlow, "normalizedBlocks"));
    if (safeIndex < 0 || safeIndex >= blocks.size())
        return {};
    return mapValue(blocks.at(safeIndex));
}

QObject* ContentsStructuredEditorFormattingController::delegateItemForBlockIndex(const QVariant& blockIndex) const
{
    const int safeIndex = normalizedBlockIndex(blockIndex);
    if (safeIndex < 0 || !m_blockRepeater || !m_documentFlow)
        return nullptr;
    QObject* blockHost = nullptr;
    QMetaObject::invokeMethod(m_blockRepeater, "itemAt", Q_RETURN_ARG(QObject*, blockHost), Q_ARG(int, safeIndex));
    if (!blockHost)
        return nullptr;
    QObject* delegate = nullptr;
    QMetaObject::invokeMethod(m_documentFlow, "delegateItemForBlockHost", Q_RETURN_ARG(QObject*, delegate), Q_ARG(QObject*, blockHost));
    return delegate;
}

QVariantMap ContentsStructuredEditorFormattingController::blockSelectionSnapshot(const QVariant& blockIndex) const
{
    if (QObject* delegateItem = delegateItemForBlockIndex(blockIndex))
    {
        QVariant result;
        if (QMetaObject::invokeMethod(delegateItem, "inlineFormatSelectionSnapshot", Q_RETURN_ARG(QVariant, result)))
            return mapValue(result);
        if (QMetaObject::invokeMethod(delegateItem, "selectionSnapshot", Q_RETURN_ARG(QVariant, result)))
            return mapValue(result);
    }
    return {};
}

bool ContentsStructuredEditorFormattingController::blockDelegateFocused(const QVariant& blockIndex) const
{
    if (QObject* delegateItem = delegateItemForBlockIndex(blockIndex))
    {
        const QVariant focused = readProperty(delegateItem, "focused");
        if (focused.isValid())
            return focused.toBool();
        return readProperty(delegateItem, "activeFocus").toBool();
    }
    return false;
}

QVariantMap ContentsStructuredEditorFormattingController::blockInlineFormatTargetState(const QVariant& blockIndex) const
{
    const int safeIndex = normalizedBlockIndex(blockIndex);
    QVariantMap state;
    if (safeIndex < 0)
    {
        state.insert(QStringLiteral("valid"), false);
        return state;
    }
    const QVariantMap snapshot = blockSelectionSnapshot(safeIndex);
    const bool selectionValid = selectionSnapshotIsValid(snapshot);
    const bool focused = blockDelegateFocused(safeIndex);
    const bool cursorValid = cursorSnapshotIsValid(snapshot);
    state.insert(QStringLiteral("blockIndex"), safeIndex);
    state.insert(QStringLiteral("cursorValid"), cursorValid);
    state.insert(QStringLiteral("focused"), focused);
    state.insert(QStringLiteral("selectionSnapshot"), snapshot);
    state.insert(QStringLiteral("selectionValid"), selectionValid);
    state.insert(QStringLiteral("valid"), selectionValid || (focused && cursorValid));
    return state;
}

QVariantMap ContentsStructuredEditorFormattingController::firstMountedSelectionTargetState() const
{
    const QVariantList blocks = listValue(invokeVariant(m_documentFlow, "normalizedBlocks"));
    for (int index = 0; index < blocks.size(); ++index)
    {
        const QVariantMap state = blockInlineFormatTargetState(index);
        if (state.value(QStringLiteral("selectionValid")).toBool())
            return state;
    }
    return QVariantMap{{QStringLiteral("valid"), false}};
}

QVariantMap ContentsStructuredEditorFormattingController::focusedCursorTargetState() const
{
    const int focusedBlockIndex = invokeInt(m_documentFlow, "focusedBlockIndex");
    const QVariantMap focused = blockInlineFormatTargetState(focusedBlockIndex);
    if (focused.value(QStringLiteral("valid")).toBool())
        return focused;
    const QVariantList blocks = listValue(invokeVariant(m_documentFlow, "normalizedBlocks"));
    for (int index = 0; index < blocks.size(); ++index)
    {
        const QVariantMap state = blockInlineFormatTargetState(index);
        if (state.value(QStringLiteral("focused")).toBool() && state.value(QStringLiteral("cursorValid")).toBool())
            return state;
    }
    return QVariantMap{{QStringLiteral("valid"), false}};
}

QVariantMap ContentsStructuredEditorFormattingController::inlineFormatTargetState() const
{
    const QVariantList blocks = listValue(invokeVariant(m_documentFlow, "normalizedBlocks"));
    if (blocks.isEmpty())
        return QVariantMap{{QStringLiteral("valid"), false}};
    const int resolvedIndex = invokeInt(m_documentFlow, "normalizedResolvedInteractiveBlockIndex");
    const QVariantMap active = blockInlineFormatTargetState(resolvedIndex);
    if (active.value(QStringLiteral("valid")).toBool())
        return active;
    const QVariantMap selected = firstMountedSelectionTargetState();
    if (selected.value(QStringLiteral("valid")).toBool())
        return selected;
    return focusedCursorTargetState();
}

bool ContentsStructuredEditorFormattingController::applyInlineFormatAtCollapsedCursor(const int blockIndex, const QString& tagName, const QVariantMap& selectionSnapshot, const int blockSourceStart, const int blockSourceEnd, const QString& blockSourceText, const QString& currentPlainText) const
{
    const int fallbackCursor = intValueOr(selectionSnapshot.value(QStringLiteral("selectionEnd")), currentPlainText.length());
    const int cursorPosition = qBound(0, intValueOr(selectionSnapshot.value(QStringLiteral("cursorPosition")), fallbackCursor), currentPlainText.length());
    const int sourceInsertionOffset = blockSourceStart + sourceOffsetForInlineTaggedLogicalOffset(blockSourceText, cursorPosition);
    const int localInsertionOffset = qBound(0, sourceInsertionOffset - blockSourceStart, blockSourceText.length());
    const QString openTag = QStringLiteral("<%1>").arg(tagName);
    const QString closeTag = QStringLiteral("</%1>").arg(tagName);
    const QString nextBlockSourceText = blockSourceText.left(localInsertionOffset) + openTag + closeTag + blockSourceText.mid(localInsertionOffset);
    if (nextBlockSourceText == blockSourceText)
        return false;
    QVariantMap options;
    options.insert(QStringLiteral("immediatePersistence"), true);
    options.insert(QStringLiteral("localCursorPosition"), cursorPosition);
    options.insert(QStringLiteral("mutationKind"), QStringLiteral("inline-format"));
    options.insert(QStringLiteral("sourceOffset"), sourceInsertionOffset + openTag.length());
    options.insert(QStringLiteral("targetBlockIndex"), blockIndex);
    return invokeBool(m_documentFlow, "replaceSourceRange", blockSourceStart, blockSourceEnd, nextBlockSourceText, options);
}

bool ContentsStructuredEditorFormattingController::applyInlineFormatToBlockSelection(const QVariant& blockIndex, const QVariant& tagName, const QVariantMap& explicitSelectionSnapshot) const
{
    if (!m_documentFlow)
        return false;
    const QString normalizedTagName = normalizeInlineStyleTagName(tagName);
    const int safeBlockIndex = normalizedBlockIndex(blockIndex);
    const QVariantMap blockEntry = interactiveBlockEntry(safeBlockIndex);
    if (normalizedTagName.isEmpty() || safeBlockIndex < 0 || blockEntry.isEmpty())
        return false;
    if (invokeBool(m_documentFlow, "blockTextEditable", QVariant::fromValue(static_cast<QObject*>(nullptr)), blockEntry) == false)
        return false;

    const QString currentSourceText = normalizedText(invokeVariant(m_documentFlow, "normalizedSourceText", readProperty(m_documentFlow, "sourceText")));
    const int blockSourceStart = qBound(0, blockEntry.value(QStringLiteral("sourceStart")).toInt(), currentSourceText.length());
    const int blockSourceEnd = qBound(blockSourceStart, intValueOr(blockEntry.value(QStringLiteral("sourceEnd")), blockSourceStart), currentSourceText.length());
    const QString blockSourceText = currentSourceText.mid(blockSourceStart, blockSourceEnd - blockSourceStart);
    const QString currentPlainText = plainTextFromInlineTaggedSource(blockSourceText);
    const QVariantMap snapshot = (!explicitSelectionSnapshot.isEmpty() && (selectionSnapshotIsValid(explicitSelectionSnapshot) || cursorSnapshotIsValid(explicitSelectionSnapshot)))
        ? explicitSelectionSnapshot
        : blockSelectionSnapshot(safeBlockIndex);
    const int selectionStart = qBound(0, snapshot.value(QStringLiteral("selectionStart")).toInt(), currentPlainText.length());
    const int selectionEnd = qBound(selectionStart, snapshot.value(QStringLiteral("selectionEnd")).toInt(), currentPlainText.length());

    if (selectionEnd <= selectionStart)
    {
        if (!cursorSnapshotIsValid(snapshot))
            return false;
        return applyInlineFormatAtCollapsedCursor(safeBlockIndex, normalizedTagName, snapshot, blockSourceStart, blockSourceEnd, blockSourceText, currentPlainText);
    }

    const QVariantMap mutationPayload = buildInlineStyleSelectionPayload(blockSourceText, selectionStart, selectionEnd, normalizedTagName);
    if (!mutationPayload.value(QStringLiteral("applied")).toBool())
        return false;
    const QString nextBlockSourceText = mutationPayload.value(QStringLiteral("nextSourceText")).toString();
    const int cursorPosition = adjustedCursorPositionForSelectionMutation(snapshot, selectionStart, selectionEnd, selectionStart, selectionEnd);
    QVariantMap options;
    options.insert(QStringLiteral("immediatePersistence"), true);
    options.insert(QStringLiteral("localCursorPosition"), cursorPosition);
    options.insert(QStringLiteral("mutationKind"), QStringLiteral("inline-format"));
    options.insert(QStringLiteral("selectionEnd"), selectionEnd);
    options.insert(QStringLiteral("selectionStart"), selectionStart);
    options.insert(QStringLiteral("sourceOffset"), blockSourceStart + sourceOffsetForInlineTaggedLogicalOffset(nextBlockSourceText, cursorPosition));
    options.insert(QStringLiteral("targetBlockIndex"), safeBlockIndex);
    return invokeBool(m_documentFlow, "replaceSourceRange", blockSourceStart, blockSourceEnd, nextBlockSourceText, options);
}

bool ContentsStructuredEditorFormattingController::applyInlineFormatToActiveSelection(const QVariant& tagName) const
{
    const QVariantMap state = inlineFormatTargetState();
    if (!state.value(QStringLiteral("valid")).toBool())
        return false;
    return applyInlineFormatToBlockSelection(state.value(QStringLiteral("blockIndex")), tagName, mapValue(state.value(QStringLiteral("selectionSnapshot"))));
}
