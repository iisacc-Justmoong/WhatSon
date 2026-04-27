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
             QStringLiteral("src/app/models/editor/tags/ContentsStructuredTagValidator.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsStructuredTagValidator.cpp"),
             QStringLiteral("src/app/models/editor/tags/WhatSonStructuredTagLinter.hpp"),
             QStringLiteral("src/app/models/editor/tags/WhatSonStructuredTagLinter.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagTextGenerator.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagTextGenerator.cpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsResourceTagController.qml") })
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
             QStringLiteral("src/app/models/editor/resource/ContentsResourceTagController.qml") })
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
    QVERIFY(testCmakeSource.contains(
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
    QVERIFY(registrarSource.contains(
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
        QStringLiteral("app/models/editor/tags/WhatSonStructuredTagLinter.hpp")));

    const QString resourceImportController = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.qml"));
    QVERIFY(resourceImportController.contains(QStringLiteral("import \"../tags\" as EditorTagsModel")));
    QVERIFY(resourceImportController.contains(
        QStringLiteral("EditorTagsModel.ContentsResourceTagController")));

    const QString tagsReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/editor/tags/README.md"));
    QVERIFY(tagsReadme.contains(QStringLiteral("Non-format editor body tags")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<agenda>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<task>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<callout>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<break>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<resource")));
    QVERIFY(tagsReadme.contains(QStringLiteral("ContentsEditorBodyTagInsertionPlanner")));

    const QString validatorReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/file/validator/README.md"));
    QVERIFY(!validatorReadme.contains(QStringLiteral("WhatSonStructuredTagLinter")));
    QVERIFY(!validatorReadme.contains(QStringLiteral("ContentsStructuredTagValidator")));

    const QString rootGitIgnore = readUtf8SourceFile(QStringLiteral(".gitignore"));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!src/app/models/editor/tags/**")));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!docs/src/app/models/editor/tags/**")));
}

void WhatSonCppRegressionTests::editorBodyTagInsertionPlanner_buildsRawTagInsertionPayloads()
{
    ContentsAgendaBackend agendaBackend;
    ContentsCalloutBackend calloutBackend;
    ContentsEditorBodyTagInsertionPlanner planner;
    planner.setAgendaBackend(&agendaBackend);
    planner.setCalloutBackend(&calloutBackend);

    const QVariantMap calloutPayload = planner.buildStructuredShortcutInsertionPayload(
        QStringLiteral("Intro"),
        5,
        QStringLiteral("callout"));
    QVERIFY(calloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        calloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Intro\n<callout> </callout>"));
    QVERIFY(calloutPayload.value(QStringLiteral("sourceOffset")).toInt() > 5);
    QCOMPARE(calloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));

    const QVariantMap wrappedCalloutPayload = planner.buildCalloutRangeWrappingPayload(
        QStringLiteral("Alpha\nBeta\nGamma"),
        6,
        10);
    QVERIFY(wrappedCalloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha\n<callout>Beta</callout>\nGamma"));
    QCOMPARE(wrappedCalloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("wrappedSourceText")).toString(),
        QStringLiteral("Beta"));

    const QVariantMap nestedCalloutWrapPayload = planner.buildCalloutRangeWrappingPayload(
        QStringLiteral("<callout>inside</callout>\nTail"),
        9,
        15);
    QVERIFY(!nestedCalloutWrapPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        nestedCalloutWrapPayload.value(QStringLiteral("reason")).toString(),
        QStringLiteral("callout-range-overlaps-structured-block"));

    const QVariantMap agendaSpec = planner.structuredShortcutInsertionSpec(QStringLiteral("agenda"));
    QVERIFY(agendaSpec.value(QStringLiteral("applied")).toBool());
    QVERIFY(agendaSpec.value(QStringLiteral("insertionSourceText")).toString().startsWith(QStringLiteral("<agenda")));
    QVERIFY(agendaSpec.value(QStringLiteral("insertionSourceText")).toString().contains(QStringLiteral("<task done=\"false\">")));

    const QString structuredSource = QStringLiteral("<callout>inside</callout>\nTail");
    const QVariantMap nestedBreakPayload = planner.buildStructuredShortcutInsertionPayload(
        structuredSource,
        10,
        QStringLiteral("break"));
    QVERIFY(nestedBreakPayload.value(QStringLiteral("applied")).toBool());
    QVERIFY(nestedBreakPayload.value(QStringLiteral("resolvedInsertionOffset")).toInt() > 10);
    QCOMPARE(
        nestedBreakPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<callout>inside</callout>\n</break>\nTail"));

    const QVariantMap rawResourcePayload = planner.buildRawSourceInsertionPayload(
        QStringLiteral("Alpha"),
        5,
        QStringLiteral("<resource type=\"image\" />"),
        QStringLiteral("<resource type=\"image\" />").size());
    QVERIFY(rawResourcePayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        rawResourcePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha\n<resource type=\"image\" />"));

    ContentsEditorBodyTagInsertionPlanner missingBackendPlanner;
    const QVariantMap rejectedCallout = missingBackendPlanner.structuredShortcutInsertionSpec(QStringLiteral("callout"));
    QVERIFY(!rejectedCallout.value(QStringLiteral("applied")).toBool());
    QCOMPARE(rejectedCallout.value(QStringLiteral("reason")).toString(), QStringLiteral("missing-callout-backend"));
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
