#include "app/models/editor/tags/ContentsResourceTagController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentMutationPolicy.hpp"
#include "app/models/editor/tags/ContentsResourceTagTextGenerator.hpp"

#include <QRegularExpression>

using namespace WhatSon::Editor::DynamicObjectSupport;

ContentsResourceTagController::ContentsResourceTagController(QObject* parent)
    : QObject(parent)
    , m_resourceInsertionPolicy(new ContentsStructuredDocumentMutationPolicy(this))
    , m_resourceTagTextGenerator(new ContentsResourceTagTextGenerator(this))
{
}

ContentsResourceTagController::~ContentsResourceTagController() = default;

int ContentsResourceTagController::currentEditorCursorPosition() const
{
    return invokeInt(m_view, "currentEditorCursorPosition");
}

QVariantList ContentsResourceTagController::normalizedImportedResourceEntries(const QVariant& importedEntries) const
{
    return normalizeSequentialVariant(importedEntries);
}

int ContentsResourceTagController::currentResourceInsertionSourceOffset() const
{
    const QString currentSourceText = m_editorText;
    if (m_showStructuredDocumentFlow && m_structuredDocumentFlow)
    {
        const int structuredOffset = invokeInt(
            m_structuredDocumentFlow,
            "shortcutInsertionSourceOffset",
            {},
            -1);
        if (structuredOffset >= 0)
        {
            return std::clamp(structuredOffset, 0, int(currentSourceText.size()));
        }
    }

    const int logicalCursorOffset = std::max(0, currentEditorCursorPosition());
    if (m_editorTypingController)
    {
        const int collapsedOffset = invokeInt(
            m_editorTypingController,
            "sourceOffsetForCollapsedLogicalInsertion",
            { currentSourceText, logicalCursorOffset },
            -1);
        if (collapsedOffset >= 0)
        {
            return std::clamp(collapsedOffset, 0, int(currentSourceText.size()));
        }

        const int logicalOffset = invokeInt(
            m_editorTypingController,
            "sourceOffsetForLogicalOffset",
            { logicalCursorOffset },
            -1);
        if (logicalOffset >= 0)
        {
            return std::clamp(logicalOffset, 0, int(currentSourceText.size()));
        }
    }

    return std::clamp(logicalCursorOffset, 0, int(currentSourceText.size()));
}

bool ContentsResourceTagController::sourceContainsCanonicalResourceTag(const QString& sourceText) const
{
    if (sourceText.isEmpty())
    {
        return false;
    }

    static const QRegularExpression resourceTagExpression(
        QStringLiteral(R"(<resource\b[^>]*\/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    return resourceTagExpression.match(sourceText).hasMatch();
}

int ContentsResourceTagController::canonicalResourceTagCount(const QString& sourceText) const
{
    if (sourceText.isEmpty())
    {
        return 0;
    }

    static const QRegularExpression resourceTagExpression(
        QStringLiteral(R"(<resource\b[^>]*\/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    int count = 0;
    auto match = resourceTagExpression.globalMatch(sourceText);
    while (match.hasNext())
    {
        match.next();
        ++count;
    }
    return count;
}

bool ContentsResourceTagController::resourceTagLossDetected(
    const QString& previousSourceText,
    const QString& nextSourceText) const
{
    if (m_showStructuredDocumentFlow)
    {
        return false;
    }

    const bool localEditorAuthority = boolProperty(m_editorSession, "localEditorAuthority");
    const int selectedBodyCount = (!localEditorAuthority && m_selectedNoteBodyNoteId == m_selectedNoteId)
        ? canonicalResourceTagCount(m_selectedNoteBodyText)
        : 0;
    const int baselineCount = std::max(
        { canonicalResourceTagCount(previousSourceText),
          canonicalResourceTagCount(m_documentPresentationSourceText),
          selectedBodyCount });
    return baselineCount > canonicalResourceTagCount(nextSourceText);
}

bool ContentsResourceTagController::insertImportedResourceTags(const QVariant& importedEntries)
{
    const QVariantList normalizedEntries = normalizedImportedResourceEntries(importedEntries);
    if (normalizedEntries.isEmpty())
    {
        return false;
    }

    QStringList tagTexts;
    for (const QVariant& entry : normalizedEntries)
    {
        const QString tagText = m_resourceTagTextGenerator->buildCanonicalResourceTag(entry);
        if (!tagText.isEmpty())
        {
            tagTexts.append(tagText);
        }
    }

    if (tagTexts.isEmpty())
    {
        return false;
    }

    const QString currentSourceText = m_editorText;
    const int insertionOffset = currentResourceInsertionSourceOffset();
    const QVariantMap insertionPayload = m_resourceInsertionPolicy->buildResourceInsertionPayload(
        currentSourceText,
        insertionOffset,
        tagTexts);

    bool inserted = false;
    const QString nextSourceText = insertionPayload.value(QStringLiteral("nextSourceText")).toString();
    if (!nextSourceText.isEmpty() && nextSourceText != currentSourceText && m_view)
    {
        inserted = invokeBool(
            m_view,
            "applyDocumentSourceMutation",
            { nextSourceText, insertionPayload.value(QStringLiteral("focusRequest")).toMap() });
    }
    else if (m_editorTypingController)
    {
        inserted = invokeBool(
            m_editorTypingController,
            "insertRawSourceTextAtCursor",
            { tagTexts.join(QStringLiteral("\n")), tagTexts.join(QStringLiteral("\n")).size() });
    }

    if (inserted)
    {
        m_editorText = nextSourceText.isEmpty() ? m_editorText : nextSourceText;
    }
    return inserted;
}
