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

void WhatSonCppRegressionTests::editorInlineStyleMutationSupport_buildsDirectRawSelectionWrapMutations()
{
    const QString supportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js"));
    QVERIFY(!supportSource.isEmpty());

    QJSEngine engine;
    QJSValue evaluation = engine.evaluate(
        supportSource,
        QStringLiteral("ContentsRawInlineStyleMutationSupport.js"));
    QVERIFY2(!evaluation.isError(), qPrintable(evaluation.toString()));

    const auto callHelper = [&engine](const QString& functionName, const QJSValueList& arguments) {
        QVariantMap failurePayload;
        QJSValue function = engine.globalObject().property(functionName);
        if (!function.isCallable())
        {
            failurePayload.insert(QStringLiteral("__error"), functionName + QStringLiteral(":not-callable"));
            return failurePayload;
        }
        QJSValue result = function.call(arguments);
        if (result.isError())
        {
            failurePayload.insert(QStringLiteral("__error"), result.toString());
            return failurePayload;
        }
        return result.toVariant().toMap();
    };

    const QVariantMap boldPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("Alpha beta")),
            QJSValue(0),
            QJSValue(5),
            QJSValue(QStringLiteral("bold"))
        });
    QVERIFY2(!boldPayload.contains(QStringLiteral("__error")), qPrintable(boldPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(boldPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        boldPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Alpha</bold> beta"));

    const QVariantMap nestedBoldPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("<bold>Alpha</bold> beta")),
            QJSValue(0),
            QJSValue(5),
            QJSValue(QStringLiteral("bold"))
        });
    QVERIFY2(!nestedBoldPayload.contains(QStringLiteral("__error")), qPrintable(nestedBoldPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(nestedBoldPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        nestedBoldPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold><bold>Alpha</bold></bold> beta"));

    const QVariantMap plainPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("Alpha <italic>beta</italic>")),
            QJSValue(6),
            QJSValue(10),
            QJSValue(QStringLiteral("plain"))
        });
    QVERIFY2(!plainPayload.contains(QStringLiteral("__error")), qPrintable(plainPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(plainPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        plainPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha beta"));

    const QVariantMap markdownPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("# Title")),
            QJSValue(0),
            QJSValue(7),
            QJSValue(QStringLiteral("bold"))
        });
    QVERIFY2(!markdownPayload.contains(QStringLiteral("__error")), qPrintable(markdownPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(markdownPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        markdownPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold># Title</bold>"));

    const QVariantMap weblinkPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("<weblink href=\"https://example.com\">Doc</weblink>")),
            QJSValue(0),
            QJSValue(3),
            QJSValue(QStringLiteral("bold"))
        });
    QVERIFY2(!weblinkPayload.contains(QStringLiteral("__error")), qPrintable(weblinkPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(weblinkPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        weblinkPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<weblink href=\"https://example.com\"><bold>Doc</bold></weblink>"));

    const QVariantMap unsupportedPayload = callHelper(
        QStringLiteral("buildInlineStyleSelectionPayload"),
        {
            QJSValue(QStringLiteral("Alpha beta")),
            QJSValue(0),
            QJSValue(5),
            QJSValue(QStringLiteral("unsupported"))
        });
    QVERIFY2(!unsupportedPayload.contains(QStringLiteral("__error")), qPrintable(unsupportedPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(!unsupportedPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        unsupportedPayload.value(QStringLiteral("reason")).toString(),
        QStringLiteral("unsupported-style-tag"));

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
}
