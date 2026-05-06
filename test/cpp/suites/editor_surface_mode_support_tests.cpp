#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsEditorDisplayBackend.hpp"
#include "app/models/editor/display/ContentsEditorSurfaceModeSupport.hpp"

void WhatSonCppRegressionTests::editorSurfaceModeSupport_switchesToResourceEditorForResourceListModels()
{
    ContentsEditorSurfaceModeSupport support;

    QObject resourceListModel;
    QVariantMap resourceEntry;
    resourceEntry.insert(QStringLiteral("displayName"), QStringLiteral("Cover.PNG"));
    resourceEntry.insert(QStringLiteral("renderMode"), QStringLiteral("image"));
    resourceListModel.setProperty("noteBacked", false);
    resourceListModel.setProperty("currentResourceEntry", resourceEntry);

    QObject noteListModel;
    noteListModel.setProperty("noteBacked", true);
    noteListModel.setProperty("currentResourceEntry", resourceEntry);

    QVERIFY(support.resourceEditorVisibleFor(&resourceListModel));
    QVERIFY(!support.resourceEditorVisibleFor(&noteListModel));
    QVERIFY(!support.resourceEditorVisibleFor(nullptr));

    const QVariantMap resolvedResourceEntry = support.currentResourceEntryFor(&resourceListModel);
    QCOMPARE(
        resolvedResourceEntry.value(QStringLiteral("displayName")).toString(),
        QStringLiteral("Cover.PNG"));
    QVERIFY(support.hasCurrentResourceEntry(&resourceListModel));

    const QVariantMap emptyResourceEntry = support.currentResourceEntryFor(&noteListModel);
    QVERIFY(!support.hasCurrentResourceEntry(&noteListModel));
    QVERIFY(emptyResourceEntry.isEmpty());

    support.setNoteListModel(&resourceListModel);
    QVERIFY(support.resourceEditorVisible());
    QCOMPARE(support.currentResourceEntry().value(QStringLiteral("displayName")).toString(), QStringLiteral("Cover.PNG"));
}

void WhatSonCppRegressionTests::editorDisplayBackend_mountsNoteSessionAndCommitsRawBody()
{
    ContentsEditorDisplayBackend backend;
    FakeSelectionNoteListModel noteListModel;
    FakeContentPersistenceController contentController;

    noteListModel.setCurrentNoteId(QStringLiteral("note-alpha"));
    noteListModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/whatson/note-alpha.wsnote"));
    noteListModel.setCurrentBodyText(QStringLiteral("<paragraph>Alpha</paragraph>"));

    backend.setContentController(&contentController);
    backend.setNoteListModel(&noteListModel);

    QVERIFY(backend.noteDocumentParseMounted());
    QCOMPARE(backend.currentNoteId(), QStringLiteral("note-alpha"));
    QCOMPARE(backend.currentNoteDirectoryPath(), QStringLiteral("/tmp/whatson/note-alpha.wsnote"));
    QCOMPARE(backend.editorSession()->property("editorText").toString(), QStringLiteral("<paragraph>Alpha</paragraph>"));

    QVERIFY(backend.commitEditedSourceText(QStringLiteral("<paragraph>Beta</paragraph>")));
    QCOMPARE(backend.editorSession()->property("editorText").toString(), QStringLiteral("<paragraph>Beta</paragraph>"));
    QVERIFY(backend.editorSession()->property("pendingBodySave").toBool());

    QVariantMap mutationPayload;
    mutationPayload.insert(QStringLiteral("nextSourceText"), QStringLiteral("<paragraph>Gamma</paragraph>"));
    QVERIFY(backend.applyDocumentSourceMutation(mutationPayload).toBool());
    QCOMPARE(backend.editorSession()->property("editorText").toString(), QStringLiteral("<paragraph>Gamma</paragraph>"));
    QVERIFY(backend.editorSession()->property("pendingBodySave").toBool());

    QVERIFY(!backend.applyDocumentSourceMutation(QVariantMap()).toBool());
    QCOMPARE(backend.editorSession()->property("editorText").toString(), QStringLiteral("<paragraph>Gamma</paragraph>"));

    backend.setEditorCursorPosition(7);
    QCOMPARE(backend.currentEditorCursorPosition().toInt(), 7);
    QCOMPARE(backend.terminalBodyClickSourceOffset().toInt(), QStringLiteral("<paragraph>Gamma</paragraph>").size());
    QCOMPARE(
        backend.encodeXmlAttributeValue(QStringLiteral("\"<&>'")).toString(),
        QStringLiteral("&quot;&lt;&amp;&gt;&apos;"));
}
