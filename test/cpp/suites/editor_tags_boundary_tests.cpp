#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorTagsBoundary_groupsEditorTagInsertionResponsibilities()
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
             QStringLiteral("src/app/models/editor/tags/ContentsEditorTagInsertionController.hpp"),
             QStringLiteral("src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsEditorTagInsertionController.hpp.md"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp.md"),
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
             QStringLiteral("src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js"),
             QStringLiteral("docs/src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js.md"),
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
        QStringLiteral("src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp")));
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
        QStringLiteral("app/models/editor/tags/ContentsEditorTagInsertionController.hpp")));
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
    QVERIFY(tagsReadme.contains(QStringLiteral("Editor tag insertion")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<bold>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<italic>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<agenda>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<task>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<callout>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<break>")));
    QVERIFY(tagsReadme.contains(QStringLiteral("<resource")));
    QVERIFY(tagsReadme.contains(QStringLiteral("generated agenda/callout/break insertion")));
    QVERIFY(!tagsReadme.contains(QStringLiteral("ContentsRawBodyTagMutationSupport.js")));
    QVERIFY(!tagsReadme.contains(QStringLiteral("ContentsEditorBodyTagInsertionPlanner")));

    const QString validatorReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/file/validator/README.md"));
    QVERIFY(!validatorReadme.contains(QStringLiteral("WhatSonStructuredTagLinter")));
    QVERIFY(!validatorReadme.contains(QStringLiteral("ContentsStructuredTagValidator")));

    const QString rootGitIgnore = readUtf8SourceFile(QStringLiteral(".gitignore"));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!src/app/models/editor/tags/**")));
    QVERIFY(rootGitIgnore.contains(QStringLiteral("!docs/src/app/models/editor/tags/**")));
}

void WhatSonCppRegressionTests::editorTagInsertionController_buildsBodyTagInsertionPayloads()
{
    ContentsEditorTagInsertionController controller;

    QCOMPARE(controller.tagNameForBodyShortcutKey(Qt::Key_A), QStringLiteral("agenda"));
    QCOMPARE(controller.tagNameForBodyShortcutKey(Qt::Key_C), QStringLiteral("callout"));
    QCOMPARE(controller.tagNameForBodyShortcutKey(Qt::Key_Return), QStringLiteral("break"));
    QCOMPARE(controller.tagNameForBodyShortcutKey(Qt::Key_Enter), QStringLiteral("break"));
    QVERIFY(controller.tagNameForBodyShortcutKey(Qt::Key_X).isEmpty());

    const QVariantMap calloutPayload = controller.buildTagInsertionPayload(
        QStringLiteral("Intro"),
        5,
        5,
        QStringLiteral("callout"));
    QVERIFY(calloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        calloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Intro\n<callout> </callout>"));
    QVERIFY(calloutPayload.value(QStringLiteral("sourceOffset")).toInt() > 5);
    QCOMPARE(calloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));

    const QVariantMap wrappedCalloutPayload = controller.buildTagInsertionPayload(
        QStringLiteral("Alpha\nBeta\nGamma"),
        6,
        10,
        QStringLiteral("callout"));
    QVERIFY(wrappedCalloutPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("Alpha\n<callout>Beta</callout>\nGamma"));
    QCOMPARE(wrappedCalloutPayload.value(QStringLiteral("tagKind")).toString(), QStringLiteral("callout"));
    QCOMPARE(
        wrappedCalloutPayload.value(QStringLiteral("wrappedSourceText")).toString(),
        QStringLiteral("Beta"));

    const QVariantMap nestedCalloutWrapPayload = controller.buildTagInsertionPayload(
        QStringLiteral("<callout>inside</callout>\nTail"),
        9,
        15,
        QStringLiteral("callout"));
    QVERIFY(!nestedCalloutWrapPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        nestedCalloutWrapPayload.value(QStringLiteral("reason")).toString(),
        QStringLiteral("body-tag-range-overlaps-structured-block"));

    const QVariantMap agendaPayload = controller.buildTagInsertionPayload(
        QStringLiteral("Intro"),
        5,
        5,
        QStringLiteral("agenda"));
    QVERIFY(agendaPayload.value(QStringLiteral("applied")).toBool());
    QVERIFY(agendaPayload.value(QStringLiteral("rawSourceText")).toString().startsWith(QStringLiteral("<agenda")));
    QVERIFY(agendaPayload.value(QStringLiteral("rawSourceText")).toString().contains(QStringLiteral("<task done=\"false\">")));
    QVERIFY(agendaPayload.value(QStringLiteral("nextSourceText")).toString().startsWith(QStringLiteral("Intro\n<agenda")));

    const QString structuredSource = QStringLiteral("<callout>inside</callout>\nTail");
    const QVariantMap nestedBreakPayload = controller.buildTagInsertionPayload(
        structuredSource,
        10,
        10,
        QStringLiteral("break"));
    QVERIFY(nestedBreakPayload.value(QStringLiteral("applied")).toBool());
    QVERIFY(nestedBreakPayload.value(QStringLiteral("resolvedInsertionOffset")).toInt() > 10);
    QCOMPARE(
        nestedBreakPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<callout>inside</callout>\n</break>\nTail"));

    const QVariantMap rejectedCallout = controller.buildTagInsertionPayload(
        QStringLiteral("Alpha"),
        5,
        5,
        QStringLiteral("unsupported"));
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
