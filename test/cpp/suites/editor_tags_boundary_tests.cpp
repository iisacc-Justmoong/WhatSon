#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorTagsBoundary_groupsNonFormatBodyTagResponsibilities()
{
    QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();

    const auto fileExists = [&repositoryRoot](const QString& relativePath) {
        return QFileInfo::exists(repositoryRoot.filePath(relativePath));
    };

    for (const QString& relativePath : {
             QStringLiteral("src/app/models/editor/tags/ContentsAgendaBackend.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsAgendaBackend.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsCalloutBackend.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsCalloutBackend.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js.md"),
             QStringLiteral("src/app/models/editor/tags/ContentsStructuredTagValidator.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsStructuredTagValidator.cpp"),
             QStringLiteral("src/app/models/editor/tags/WhatSonStructuredTagLinter.hpp"),
             QStringLiteral("src/app/models/editor/tags/WhatSonStructuredTagLinter.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagTextGenerator.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagTextGenerator.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagController.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagController.cpp") })
    {
        QVERIFY2(fileExists(relativePath), qPrintable(relativePath));
    }

    for (const QString& relativePath : {
             QStringLiteral("src/app/models/agenda/CMakeLists.txt"),
             QStringLiteral("src/app/models/agenda/ContentsAgendaBackend.hpp"),
             QStringLiteral("src/app/models/agenda/ContentsAgendaBackend.cpp"),
             QStringLiteral("src/app/models/callout/CMakeLists.txt"),
             QStringLiteral("src/app/models/callout/ContentsCalloutBackend.hpp"),
             QStringLiteral("src/app/models/callout/ContentsCalloutBackend.cpp"),
             QStringLiteral("src/app/models/file/validator/ContentsStructuredTagValidator.hpp"),
             QStringLiteral("src/app/models/file/validator/ContentsStructuredTagValidator.cpp"),
             QStringLiteral("src/app/models/file/validator/WhatSonStructuredTagLinter.hpp"),
             QStringLiteral("src/app/models/file/validator/WhatSonStructuredTagLinter.cpp"),
             QStringLiteral("src/app/models/editor/resource/ContentsResourceTagTextGenerator.hpp"),
             QStringLiteral("src/app/models/editor/resource/ContentsResourceTagTextGenerator.cpp"),
             QStringLiteral("src/app/models/editor/resource/ContentsResourceTagController.qml"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagController.qml"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.hpp.md"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.cpp.md") })
    {
        QVERIFY2(!fileExists(relativePath), qPrintable(relativePath));
    }

    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("add_subdirectory(models/agenda)")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("add_subdirectory(models/callout)")));

    const QString testCmakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/tags/ContentsAgendaBackend.cpp")));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/tags/ContentsCalloutBackend.cpp")));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/tags/WhatSonStructuredTagLinter.cpp")));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/tags/ContentsResourceTagTextGenerator.cpp")));
    QVERIFY(!testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.cpp")));
    QVERIFY(!testCmakeSource.contains(QStringLiteral("src/app/models/agenda/ContentsAgendaBackend.cpp")));
    QVERIFY(!testCmakeSource.contains(QStringLiteral("src/app/models/file/validator/WhatSonStructuredTagLinter.cpp")));
    QVERIFY(!testCmakeSource.contains(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceTagTextGenerator.cpp")));

    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));
    QVERIFY(registrarSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsAgendaBackend.hpp")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsCalloutBackend.hpp")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsStructuredTagValidator.hpp")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsResourceTagTextGenerator.hpp")));
    QVERIFY(!registrarSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.hpp")));
    QVERIFY(!registrarSource.contains(QStringLiteral("app/models/agenda/ContentsAgendaBackend.hpp")));
    QVERIFY(!registrarSource.contains(QStringLiteral("app/models/callout/ContentsCalloutBackend.hpp")));
    QVERIFY(!registrarSource.contains(
        QStringLiteral("app/models/file/validator/ContentsStructuredTagValidator.hpp")));

    const QString sessionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    QVERIFY(sessionControllerSource.contains(
        QStringLiteral("app/models/editor/tags/ContentsAgendaBackend.hpp")));

    const QString parserSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/parser/ContentsWsnBodyBlockParser.cpp"));
    const QString rendererSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/renderer/ContentsStructuredBlockRenderer.cpp"));
    QVERIFY(parserSource.contains(
        QStringLiteral("app/models/editor/tags/WhatSonStructuredTagLinter.hpp")));
    QVERIFY(rendererSource.contains(
        QStringLiteral("app/models/editor/parser/ContentsWsnBodyBlockParser.hpp")));
    QVERIFY(!rendererSource.contains(
        QStringLiteral("app/models/editor/tags/WhatSonStructuredTagLinter.hpp")));

    const QString resourceImportControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.hpp"));
    const QString resourceImportControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.cpp"));
    QVERIFY(resourceImportControllerHeader.contains(QStringLiteral("class ContentsResourceTagController;")));
    QVERIFY(resourceImportControllerSource.contains(
        QStringLiteral("#include \"app/models/editor/tags/ContentsResourceTagController.hpp\"")));
    QVERIFY(resourceImportControllerSource.contains(
        QStringLiteral("m_resourceTagController(new ContentsResourceTagController(this))")));

    const QString tagsReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/editor/tags/README.md"));
    QVERIFY(tagsReadme.contains(QStringLiteral("Non-format editor body tags")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<agenda>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<task>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<callout>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<break>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<resource")));
    QVERIFY(tagsReadme.contains(QStringLiteral("ContentsRawBodyTagMutationSupport.js")));
    QVERIFY(!tagsReadme.contains(QStringLiteral("ContentsEditorBodyTagInsertionPlanner")));

    const QString validatorReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/file/validator/README.md"));
    QVERIFY(!validatorReadme.contains(QStringLiteral("WhatSonStructuredTagLinter")));
    QVERIFY(!validatorReadme.contains(QStringLiteral("ContentsStructuredTagValidator")));

    const QString rootGitIgnore = readUtf8SourceFile(QStringLiteral(".gitignore"));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!src/app/models/editor/tags/**")));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!docs/src/app/models/editor/tags/**")));
}

void WhatSonCppRegressionTests::editorBodyTagMutationSupport_buildsRawTagInsertionPayloads()
{
    const QString supportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js"));
    QVERIFY(!supportSource.isEmpty());

    QJSEngine engine;
    QJSValue evaluation = engine.evaluate(
        supportSource,
        QStringLiteral("ContentsRawBodyTagMutationSupport.js"));
    QVERIFY2(
        !evaluation.isError(),
        qPrintable(evaluation.toString()));

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

    const QVariantMap calloutPayload = callHelper(
        QStringLiteral("buildStructuredShortcutInsertionPayload"),
        {
            QJSValue(QStringLiteral("Intro")),
            QJSValue(5),
            QJSValue(QStringLiteral("callout"))
        });
    QVERIFY2(!calloutPayload.contains(QStringLiteral("__error")), qPrintable(calloutPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(calloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        calloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Intro\n<callout> </callout>"));
    QVERIFY(calloutPayload.value(QStringLiteral("sourceOffset")).toInt() > 5);
    QCOMPARE(calloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));

    const QVariantMap wrappedCalloutPayload = callHelper(
        QStringLiteral("buildCalloutRangeWrappingPayload"),
        {
            QJSValue(QStringLiteral("Alpha\nBeta\nGamma")),
            QJSValue(6),
            QJSValue(10)
        });
    QVERIFY2(!wrappedCalloutPayload.contains(QStringLiteral("__error")), qPrintable(wrappedCalloutPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(wrappedCalloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha\n<callout>Beta</callout>\nGamma"));
    QCOMPARE(wrappedCalloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("wrappedSourceText")).toString(),
        QStringLiteral("Beta"));

    const QVariantMap nestedCalloutWrapPayload = callHelper(
        QStringLiteral("buildCalloutRangeWrappingPayload"),
        {
            QJSValue(QStringLiteral("<callout>inside</callout>\nTail")),
            QJSValue(9),
            QJSValue(15)
        });
    QVERIFY2(!nestedCalloutWrapPayload.contains(QStringLiteral("__error")), qPrintable(nestedCalloutWrapPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(!nestedCalloutWrapPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        nestedCalloutWrapPayload.value(QStringLiteral("reason")).toString(),
        QStringLiteral("callout-range-overlaps-structured-block"));

    const QVariantMap agendaSpec = callHelper(
        QStringLiteral("structuredShortcutInsertionSpec"),
        {QJSValue(QStringLiteral("agenda"))});
    QVERIFY2(!agendaSpec.contains(QStringLiteral("__error")), qPrintable(agendaSpec.value(QStringLiteral("__error")).toString()));
    QVERIFY(agendaSpec.value(QStringLiteral("applied")).toBool());
    QVERIFY(agendaSpec.value(QStringLiteral("insertionSourceText")).toString().startsWith(QStringLiteral("<agenda")));
    QVERIFY(agendaSpec.value(QStringLiteral("insertionSourceText")).toString().contains(QStringLiteral("<task done=\"false\">")));

    const QString structuredSource = QStringLiteral("<callout>inside</callout>\nTail");
    const QVariantMap nestedBreakPayload = callHelper(
        QStringLiteral("buildStructuredShortcutInsertionPayload"),
        {
            QJSValue(structuredSource),
            QJSValue(10),
            QJSValue(QStringLiteral("break"))
        });
    QVERIFY2(!nestedBreakPayload.contains(QStringLiteral("__error")), qPrintable(nestedBreakPayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(nestedBreakPayload.value(QStringLiteral("applied")).toBool());
    QVERIFY(nestedBreakPayload.value(QStringLiteral("resolvedInsertionOffset")).toInt() > 10);
    QCOMPARE(
        nestedBreakPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<callout>inside</callout>\n</break>\nTail"));

    const QVariantMap rawResourcePayload = callHelper(
        QStringLiteral("buildRawSourceInsertionPayload"),
        {
            QJSValue(QStringLiteral("Alpha")),
            QJSValue(5),
            QJSValue(QStringLiteral("<resource type=\"image\" />")),
            QJSValue(static_cast<int>(QStringLiteral("<resource type=\"image\" />").size()))
        });
    QVERIFY2(!rawResourcePayload.contains(QStringLiteral("__error")), qPrintable(rawResourcePayload.value(QStringLiteral("__error")).toString()));
    QVERIFY(rawResourcePayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        rawResourcePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha\n<resource type=\"image\" />"));

    const QVariantMap rejectedCallout = callHelper(
        QStringLiteral("structuredShortcutInsertionSpec"),
        {QJSValue(QStringLiteral("unsupported"))});
    QVERIFY2(!rejectedCallout.contains(QStringLiteral("__error")), qPrintable(rejectedCallout.value(QStringLiteral("__error")).toString()));
    QVERIFY(!rejectedCallout.value(QStringLiteral("applied")).toBool());
    QCOMPARE(rejectedCallout.value(QStringLiteral("reason")).toString(), QStringLiteral("unsupported-tag-kind"));
}

void WhatSonCppRegressionTests::contentsCalloutBackend_exitsOnPlainEnterAndSplitsAtCursor()
{
    ContentsCalloutBackend backend;

    const QString sourceText = QStringLiteral("<callout>AlphaBeta</callout>");
    const int cursorSourceOffset = static_cast<int>(QStringLiteral("<callout>Alpha").size());
    const QVariantMap splitPayload = backend.detectCalloutEnterReplacement(
        sourceText,
        cursorSourceOffset,
        cursorSourceOffset,
        QStringLiteral("\n"));
    QVERIFY(splitPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(splitPayload.value(QStringLiteral("replacementSourceStart")).toInt(), cursorSourceOffset);
    QCOMPARE(
        splitPayload.value(QStringLiteral("replacementSourceEnd")).toInt(),
        static_cast<int>(sourceText.size()));
    QCOMPARE(
        splitPayload.value(QStringLiteral("replacementSourceText")).toString(),
        QStringLiteral("</callout>\nBeta"));
    QCOMPARE(
        splitPayload.value(QStringLiteral("cursorSourceOffsetFromReplacementStart")).toInt(),
        QStringLiteral("</callout>\n").size());

    const QVariantMap endPayload = backend.detectCalloutEnterReplacement(
        sourceText,
        static_cast<int>(QStringLiteral("<callout>AlphaBeta").size()),
        static_cast<int>(QStringLiteral("<callout>AlphaBeta").size()),
        QStringLiteral("\n"));
    QVERIFY(endPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        endPayload.value(QStringLiteral("replacementSourceText")).toString(),
        QStringLiteral("</callout>\n"));

    const QVariantMap shiftEnterEquivalentPayload = backend.detectCalloutEnterReplacement(
        sourceText,
        cursorSourceOffset,
        cursorSourceOffset,
        QStringLiteral("\n\n"));
    QVERIFY(!shiftEnterEquivalentPayload.value(QStringLiteral("applied")).toBool());
}
