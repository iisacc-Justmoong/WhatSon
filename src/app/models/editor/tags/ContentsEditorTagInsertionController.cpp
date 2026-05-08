#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"

#include <Qt>

namespace
{
    int normalizedBodyShortcutKey(const int key)
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
} // namespace

ContentsEditorTagInsertionController::ContentsEditorTagInsertionController(QObject* parent)
    : QObject(parent)
    , m_mutationBuilder(this)
{
    connect(
        &m_mutationBuilder,
        &ContentsEditorTagMutationBuilder::tagInsertionPayloadBuilt,
        this,
        &ContentsEditorTagInsertionController::tagInsertionPayloadBuilt);
}

QObject* ContentsEditorTagInsertionController::mutationBuilder() noexcept
{
    return &m_mutationBuilder;
}

QString ContentsEditorTagInsertionController::normalizedTagName(const QVariant& tagName) const
{
    return m_mutationBuilder.normalizedTagName(tagName);
}

QString ContentsEditorTagInsertionController::tagNameForBodyShortcutKey(const int key) const
{
    switch (normalizedBodyShortcutKey(key))
    {
    case Qt::Key_A:
        return QStringLiteral("agenda");
    case Qt::Key_C:
        return QStringLiteral("callout");
    case Qt::Key_B:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return QStringLiteral("break");
    default:
        return {};
    }
}

QString ContentsEditorTagInsertionController::tagNameForShortcutKey(const int key) const
{
    switch (key)
    {
    case Qt::Key_B:
        return QStringLiteral("bold");
    case Qt::Key_I:
        return QStringLiteral("italic");
    case Qt::Key_U:
        return QStringLiteral("underline");
    case Qt::Key_E:
        return QStringLiteral("highlight");
    default:
        return {};
    }
}

QVariantMap ContentsEditorTagInsertionController::buildTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    return m_mutationBuilder.buildTagInsertionPayload(sourceText, selectionStart, selectionEnd, tagName);
}

QVariantMap ContentsEditorTagInsertionController::buildWrappedTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    return m_mutationBuilder.buildWrappedTagInsertionPayload(sourceText, selectionStart, selectionEnd, tagName);
}
