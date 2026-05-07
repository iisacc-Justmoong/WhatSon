#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(
        QStringLiteral(
            "<title>Heading</title>\n"
            "<highlight>Glow</highlight>\n"
            "# Print title\n"
            "> Quote\n"
            "`Code`\n"
            "[Doc](https://example.com)"));
    renderer.setPreviewEnabled(true);

    const QString screenEditorHtml = renderer.editorSurfaceHtml();
    const QString screenPreviewHtml = renderer.renderedHtml();
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#D6AE58")));

    renderer.setPaperPaletteEnabled(true);

    const QString paperEditorHtml = renderer.editorSurfaceHtml();
    const QString paperPreviewHtml = renderer.renderedHtml();
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("background-color:#F4D37A")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#1F5FBF")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("background-color:#E7EAEE")));
}

void WhatSonCppRegressionTests::plainTextSourceMutator_wrapsCommittedUrlsIntoCanonicalWebLinks()
{
    ContentsPlainTextSourceMutator sourceMutator;

    const QString notYetCommittedSource = sourceMutator.applyPlainTextReplacementToSource(
        QStringLiteral("www.iisacc.co"),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("m"));
    QCOMPARE(notYetCommittedSource, QStringLiteral("www.iisacc.com"));

    const QString committedSource = sourceMutator.applyPlainTextReplacementToSource(
        notYetCommittedSource,
        notYetCommittedSource.size(),
        notYetCommittedSource.size(),
        QStringLiteral(" "));
    QCOMPARE(
        committedSource,
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink> "));

    const QString pastedSource = sourceMutator.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("Visit https://www.iisacc.com/path?q=1"));
    QCOMPARE(
        pastedSource,
        QStringLiteral(
            "Visit <weblink href=\"https://www.iisacc.com/path?q=1\">https://www.iisacc.com/path?q=1</weblink>"));

    const QString markdownSource = sourceMutator.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("[Doc](https://example.com)"));
    QCOMPARE(markdownSource, QStringLiteral("[Doc](https://example.com)"));
}

void WhatSonCppRegressionTests::inlineStyleOverlayRenderer_republishesHtmlOverlayVisibility()
{
    ContentsInlineStyleOverlayRenderer renderer;
    renderer.setSourceText(QStringLiteral("Plain"));
    QVERIFY(!renderer.htmlOverlayVisible());

    renderer.setSourceText(QStringLiteral("<bold>Alpha</bold>"));
    QVERIFY(renderer.htmlOverlayVisible());
    QVERIFY(!renderer.editorSurfaceHtml().isEmpty());
}

void WhatSonCppRegressionTests::editorPresentationProjection_publishesHtmlBlockPipelineToQmlHost()
{
    ContentsEditorPresentationProjection projection;
    projection.setSourceText(
        QStringLiteral(
            "<paragraph>Alpha</paragraph>\n"
            "<resource type=\"image\" path=\"cover.png\" />\n"
            "<callout>Beta</callout>"));

    QVERIFY(!projection.editorSurfaceHtml().isEmpty());
    QVERIFY(projection.htmlTokens().size() >= 3);
    QVERIFY(projection.normalizedHtmlBlocks().size() >= 3);

    bool foundResourceBlock = false;
    for (const QVariant& blockValue : projection.normalizedHtmlBlocks())
    {
        const QVariantMap block = blockValue.toMap();
        QCOMPARE(
            block.value(QStringLiteral("htmlBlockObjectSource")).toString(),
            QStringLiteral("iiHtmlBlock"));
        QVERIFY(!block.value(QStringLiteral("htmlBlockTagName")).toString().isEmpty());
        QVERIFY(block.value(QStringLiteral("htmlBlockIsDisplayBlock")).toBool());
        if (block.value(QStringLiteral("renderDelegateType")).toString() == QStringLiteral("resource"))
        {
            foundResourceBlock = true;
            QCOMPARE(block.value(QStringLiteral("resourceType")).toString(), QStringLiteral("image"));
            QCOMPARE(block.value(QStringLiteral("resourcePath")).toString(), QStringLiteral("cover.png"));
        }
    }
    QVERIFY(foundResourceBlock);

    const QString projectionHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/projection/ContentsEditorPresentationProjection.hpp"));
    const QString projectionCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/projection/ContentsEditorPresentationProjection.cpp"));
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(projectionHeaderSource.contains(QStringLiteral("Q_PROPERTY(QVariantList htmlTokens")));
    QVERIFY(projectionHeaderSource.contains(QStringLiteral("Q_PROPERTY(QVariantList normalizedHtmlBlocks")));
    QVERIFY(projectionHeaderSource.contains(QStringLiteral("Q_PROPERTY(int sourceCursorPosition")));
    QVERIFY(projectionHeaderSource.contains(QStringLiteral("Q_PROPERTY(int logicalCursorPosition")));
    QVERIFY(projectionHeaderSource.contains(QStringLiteral("sourceOffsetForVisibleLogicalOffset")));
    QVERIFY(projectionHeaderSource.contains(QStringLiteral("logicalOffsetForSourceOffsetWithAffinity")));
    QVERIFY(!projectionHeaderSource.contains(QStringLiteral("Q_PROPERTY(QVariantList logicalToSourceOffsets")));
    QVERIFY(projectionCppSource.contains(QStringLiteral("ContentsTextFormatRenderer::htmlTokensChanged")));
    QVERIFY(projectionCppSource.contains(QStringLiteral("ContentsTextFormatRenderer::normalizedHtmlBlocksChanged")));
    QVERIFY(projectionCppSource.contains(QStringLiteral("ContentsLogicalTextBridge::logicalToSourceOffsetsChanged")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsEditorPresentationProjection m_presentationProjection")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("inlineResourceVisualBlocks(")));
    QVERIFY(!displayBackendHeader.contains(QStringLiteral("inlineResourceVisualHeights(")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("htmlTokens: editorDisplayBackend.presentationProjection.htmlTokens")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("normalizedHtmlBlocks: editorDisplayBackend.presentationProjection.normalizedHtmlBlocks")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorCursorPosition: structuredDocumentFlow.editorCursorPosition")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("coordinateMapper: editorDisplayBackend.presentationProjection")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("logicalToSourceOffsets")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("renderInlineResourceEditorSurfaceHtml(")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("inlineResourceVisualBlocks(")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("documentBlocks: editorDisplayBackend.structuredBlockRenderer.renderedDocumentBlocks")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorDisplayBackend.presentationProjection.editorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("ContentsInlineFormatEditor {")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string lastReadyEditorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var resourceVisualBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var documentBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property string resolvedEditorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("renderedText: documentFlow.resolvedEditorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("resourceVisualBlocks: documentFlow.resourceVisualBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("documentBlocks: documentFlow.documentBlocks")));
}

void WhatSonCppRegressionTests::textFormatRenderer_preservesMarkdownUnorderedListMarkersWithoutRegexWarnings()
{
    ContentsTextFormatRenderer renderer;
    renderer.setPreviewEnabled(true);
    renderer.setSourceText(QStringLiteral("- alpha\n+ beta\n* gamma\n• delta"));

    const QString previewHtml = renderer.renderedHtml();
    QVERIFY(previewHtml.contains(QStringLiteral(">-</span>&nbsp;alpha")));
    QVERIFY(previewHtml.contains(QStringLiteral(">+</span>&nbsp;beta")));
    QVERIFY(previewHtml.contains(QStringLiteral(">*</span>&nbsp;gamma")));
    QVERIFY(previewHtml.contains(QStringLiteral(">•</span>&nbsp;delta")));
}

void WhatSonCppRegressionTests::textFormatRenderer_keepsEnterNewlinesAsEditorParagraphSlots()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(
        QStringLiteral(
            "<paragraph>Alpha\n"
            "\n"
            "Beta</paragraph>\n"
            "<paragraph>Gamma</paragraph>"));

    const QString editorHtml = renderer.editorSurfaceHtml();
    QVERIFY(!editorHtml.contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(editorHtml.contains(QStringLiteral(">Alpha</p>")));
    QVERIFY(editorHtml.contains(QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">&nbsp;</p>")));
    QVERIFY(editorHtml.contains(QStringLiteral(">Beta</p>")));
    QVERIFY(editorHtml.contains(QStringLiteral(">Gamma</p>")));
}

void WhatSonCppRegressionTests::editorTagInsertionController_replacesLegacyInlineStyleMutationSupport()
{
    QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();

    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js.md"))));

    ContentsEditorTagInsertionController controller;

    const QVariantMap boldPayload = controller.buildTagInsertionPayload(
        QStringLiteral("Alpha beta"),
        0,
        5,
        QStringLiteral("bold"));
    QVERIFY(boldPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        boldPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Alpha</bold> beta"));

    const QVariantMap markdownPayload = controller.buildTagInsertionPayload(
        QStringLiteral("# Title"),
        0,
        7,
        QStringLiteral("bold"));
    QVERIFY(markdownPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        markdownPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold># Title</bold>"));

    const QVariantMap unsupportedPayload = controller.buildTagInsertionPayload(
        QStringLiteral("Alpha beta"),
        0,
        5,
        QStringLiteral("unsupported"));
    QVERIFY(!unsupportedPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        unsupportedPayload.value(QStringLiteral("reason")).toString(),
        QStringLiteral("unsupported-tag-kind"));

    const QString rendererHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRenderer.hpp"));
    const QString rendererCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRenderer.cpp"));
    const QString editorSurfaceRendererCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRendererEditorSurface.cpp"));
    const QString markdownRendererCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRendererMarkdown.cpp"));
    const QString plainTextSourceMutatorHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsPlainTextSourceMutator.hpp"));
    const QString plainTextSourceMutatorCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsPlainTextSourceMutator.cpp"));
    const QString projectionHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/projection/ContentsEditorPresentationProjection.hpp"));
    const QString projectionCppSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/projection/ContentsEditorPresentationProjection.cpp"));
    QVERIFY(!rendererHeaderSource.contains(QStringLiteral("applyInlineStyleToLogicalSelectionSource")));
    QVERIFY(!rendererCppSource.contains(QStringLiteral("ContentsTextFormatRenderer::applyInlineStyleToLogicalSelectionSource")));
    QVERIFY(!rendererHeaderSource.contains(QStringLiteral("applyPlainTextReplacementToSource")));
    QVERIFY(!rendererCppSource.contains(QStringLiteral("ContentsTextFormatRenderer::applyPlainTextReplacementToSource")));
    QVERIFY(!rendererCppSource.contains(QStringLiteral("QString renderMarkdownAwareTextToHtml(")));
    QVERIFY(!rendererCppSource.contains(QStringLiteral("QString renderInlineStyleEditingSurfaceHtml(")));
    QVERIFY(editorSurfaceRendererCppSource.contains(QStringLiteral("QString renderInlineStyleEditingSurfaceHtmlImpl(")));
    QVERIFY(markdownRendererCppSource.contains(QStringLiteral("QString renderMarkdownAwareTextToHtml(")));
    QVERIFY(markdownRendererCppSource.contains(QStringLiteral("applyPaperPaletteToHtml")));
    QVERIFY(plainTextSourceMutatorHeaderSource.contains(QStringLiteral("applyPlainTextReplacementToSource")));
    QVERIFY(plainTextSourceMutatorCppSource.contains(QStringLiteral("ContentsPlainTextSourceMutator::applyPlainTextReplacementToSource")));
    QVERIFY(!projectionHeaderSource.contains(QStringLiteral("applyInlineStyleToLogicalSelectionSource")));
    QVERIFY(!projectionCppSource.contains(QStringLiteral("ContentsEditorPresentationProjection::applyInlineStyleToLogicalSelectionSource")));
    QVERIFY(!projectionHeaderSource.contains(QStringLiteral("applyPlainTextReplacementToSource")));
    QVERIFY(!projectionCppSource.contains(QStringLiteral("ContentsEditorPresentationProjection::applyPlainTextReplacementToSource")));

    const QString formatReadme = readUtf8SourceFile(QStringLiteral("docs/src/app/models/editor/format/README.md"));
    const QString tagsReadme = readUtf8SourceFile(QStringLiteral("docs/src/app/models/editor/tags/README.md"));
    QVERIFY(!formatReadme.contains(QStringLiteral("ContentsRawInlineStyleMutationSupport")));
    QVERIFY(tagsReadme.contains(QStringLiteral("ContentsEditorTagInsertionController")));
}

void WhatSonCppRegressionTests::editorTagInsertionController_buildsShortcutSourceWrapMutations()
{
    ContentsEditorTagInsertionController controller;

    QCOMPARE(controller.tagNameForShortcutKey(Qt::Key_B), QStringLiteral("bold"));
    QCOMPARE(controller.tagNameForShortcutKey(Qt::Key_I), QStringLiteral("italic"));
    QCOMPARE(controller.tagNameForShortcutKey(Qt::Key_U), QStringLiteral("underline"));
    QCOMPARE(controller.tagNameForShortcutKey(Qt::Key_E), QStringLiteral("highlight"));
    QVERIFY(controller.tagNameForShortcutKey(Qt::Key_H).isEmpty());
    QVERIFY(controller.tagNameForShortcutKey(Qt::Key_C).isEmpty());
    QCOMPARE(controller.normalizedTagName(QStringLiteral("callout")), QStringLiteral("callout"));
    QCOMPARE(controller.normalizedTagName(QStringLiteral("agenda")), QStringLiteral("agenda"));

    const QVariantMap boldPayload = controller.buildWrappedTagInsertionPayload(
        QStringLiteral("Alpha beta"),
        0,
        5,
        QStringLiteral("bold"));
    QVERIFY(boldPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        boldPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Alpha</bold> beta"));
    QCOMPARE(boldPayload.value(QStringLiteral("selectionStart")).toInt(), QStringLiteral("<bold>").size());
    QCOMPARE(boldPayload.value(QStringLiteral("selectionEnd")).toInt(), QStringLiteral("<bold>Alpha").size());
    QCOMPARE(boldPayload.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("<bold>Alpha").size());
    QCOMPARE(boldPayload.value(QStringLiteral("tagName")).toString(), QStringLiteral("bold"));
    QCOMPARE(boldPayload.value(QStringLiteral("wrappedSourceText")).toString(), QStringLiteral("Alpha"));

    const QVariantMap collapsedPayload = controller.buildWrappedTagInsertionPayload(
        QStringLiteral("Alpha beta"),
        5,
        5,
        QStringLiteral("italic"));
    QVERIFY(collapsedPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        collapsedPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha<italic></italic> beta"));
    QCOMPARE(
        collapsedPayload.value(QStringLiteral("cursorPosition")).toInt(),
        QStringLiteral("Alpha<italic>").size());
    QCOMPARE(
        collapsedPayload.value(QStringLiteral("selectionStart")).toInt(),
        collapsedPayload.value(QStringLiteral("selectionEnd")).toInt());
}

void WhatSonCppRegressionTests::editorTagInsertionController_preservesInlineTagBoundariesWhenReformatting()
{
    ContentsEditorTagInsertionController controller;

    const QString highlightedSource = QStringLiteral("<highlight>STAR structure</highlight>");
    const int highlightOpeningEnd = QStringLiteral("<highlight>").size();
    const int highlightClosingStart = highlightedSource.indexOf(QStringLiteral("</highlight>"));

    const QVariantMap startInsideOpeningPayload = controller.buildTagInsertionPayload(
        highlightedSource,
        QStringLiteral("<highligh").size(),
        highlightClosingStart,
        QStringLiteral("bold"));
    QVERIFY(startInsideOpeningPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        startInsideOpeningPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<highlight><bold>STAR structure</bold></highlight>"));
    QVERIFY(!startInsideOpeningPayload.value(QStringLiteral("nextSourceText")).toString().contains(QStringLiteral("<highligh<")));

    const QVariantMap endInsideClosingPayload = controller.buildTagInsertionPayload(
        highlightedSource,
        highlightOpeningEnd,
        highlightClosingStart + 3,
        QStringLiteral("italic"));
    QVERIFY(endInsideClosingPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        endInsideClosingPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<highlight><italic>STAR structure</italic></highlight>"));

    const QVariantMap partialPrefixPayload = controller.buildTagInsertionPayload(
        highlightedSource,
        0,
        highlightOpeningEnd + QStringLiteral("STAR").size(),
        QStringLiteral("underline"));
    QVERIFY(partialPrefixPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        partialPrefixPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<highlight><underline>STAR</underline> structure</highlight>"));

    const QVariantMap balancedWholeTagPayload = controller.buildTagInsertionPayload(
        highlightedSource,
        0,
        highlightedSource.size(),
        QStringLiteral("bold"));
    QVERIFY(balancedWholeTagPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        balancedWholeTagPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold><highlight>STAR structure</highlight></bold>"));

    const QString crossingSource = QStringLiteral("<bold>Alpha</bold> Beta");
    const QVariantMap crossingPayload = controller.buildTagInsertionPayload(
        crossingSource,
        QStringLiteral("<bold>Al").size(),
        crossingSource.size(),
        QStringLiteral("italic"));
    QVERIFY(crossingPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        crossingPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"));
}
