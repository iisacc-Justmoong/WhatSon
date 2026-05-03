#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteActiveStateTracker_tracksCurrentNoteAcrossActiveHierarchyChanges()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeSelectionNoteListModel libraryModel;
    libraryModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/library-note.wsnote"));

    FakeSelectionNoteListModel tagsModel;
    tagsModel.setCurrentNoteId(QStringLiteral("tags-note"));
    tagsModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/tags-note.wsnote"));

    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));
    libraryController.setNoteListModelObject(&libraryModel);
    tagsController.setNoteListModelObject(&tagsModel);

    HierarchyControllerProvider provider;
    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
    });

    SidebarHierarchyController sidebarController;
    sidebarController.setControllerProvider(&provider);

    NoteActiveStateTracker tracker;
    QSignalSpy activeNoteIdChangedSpy(&tracker, &NoteActiveStateTracker::activeNoteIdChanged);
    QSignalSpy activeStateChangedSpy(&tracker, &NoteActiveStateTracker::activeNoteStateChanged);
    tracker.setHierarchyContextSource(&sidebarController);

    QCOMPARE(tracker.hierarchyContextSource(), static_cast<QObject*>(&sidebarController));
    QCOMPARE(tracker.activeNoteListModel(), static_cast<QObject*>(&libraryModel));
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("library-note"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/library-note.wsnote"));
    QVERIFY(tracker.hasActiveNote());
    QCOMPARE(
        tracker.activeNoteEntry().value(QStringLiteral("noteId")).toString(),
        QStringLiteral("library-note"));
    QCOMPARE(tracker.activeNoteBodyText(), QString());

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(tracker.activeHierarchyIndex(), static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(tracker.activeHierarchyController(), static_cast<QObject*>(&tagsController));
    QCOMPARE(tracker.activeNoteListModel(), static_cast<QObject*>(&tagsModel));
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("tags-note"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/tags-note.wsnote"));

    libraryModel.setCurrentNoteId(QStringLiteral("library-note-2"));
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("tags-note"));

    tagsModel.setCurrentNoteId(QStringLiteral("tags-note-2"));
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("tags-note-2"));
    QVERIFY(activeNoteIdChangedSpy.count() >= 3);
    QVERIFY(activeStateChangedSpy.count() >= 3);

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}

void WhatSonCppRegressionTests::noteActiveStateTracker_clearsReadableEmptyAndNonNoteBackedSelections()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeCurrentNoteEntryOnlyListModel libraryModel;
    libraryModel.setCurrentNoteEntry({
        {QStringLiteral("noteId"), QStringLiteral("entry-note")},
        {QStringLiteral("noteDirectoryPath"), QStringLiteral("/tmp/entry-note.wsnote")},
        {QStringLiteral("bodyText"), QStringLiteral("Entry body")},
    });

    FakeSelectionNoteListModel resourcesModel;
    resourcesModel.setCurrentNoteId(QStringLiteral("resource-selection"));
    resourcesModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/resource-selection.wsnote"));
    resourcesModel.setNoteBacked(false);

    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController resourcesController(QStringLiteral("resources"));
    libraryController.setNoteListModelObject(&libraryModel);
    resourcesController.setNoteListModelObject(&resourcesModel);

    HierarchyControllerProvider provider;
    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesController},
    });

    SidebarHierarchyController sidebarController;
    sidebarController.setControllerProvider(&provider);

    NoteActiveStateTracker tracker;
    QSignalSpy hasActiveNoteChangedSpy(&tracker, &NoteActiveStateTracker::hasActiveNoteChanged);
    tracker.setHierarchyContextSource(&sidebarController);

    QCOMPARE(tracker.activeNoteId(), QStringLiteral("entry-note"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/entry-note.wsnote"));
    QCOMPARE(tracker.activeNoteBodyText(), QStringLiteral("Entry body"));
    QVERIFY(!tracker.activeNoteEntry().contains(QStringLiteral("bodyText")));
    QVERIFY(tracker.hasActiveNote());

    libraryModel.setCurrentNoteEntry({});
    QVERIFY(tracker.activeNoteId().isEmpty());
    QVERIFY(tracker.activeNoteDirectoryPath().isEmpty());
    QVERIFY(tracker.activeNoteBodyText().isEmpty());
    QVERIFY(!tracker.hasActiveNote());

    libraryModel.setCurrentNoteEntry({{QStringLiteral("id"), QStringLiteral("entry-note-2")}});
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("entry-note-2"));
    QVERIFY(tracker.hasActiveNote());

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));
    QVERIFY(tracker.activeNoteId().isEmpty());
    QVERIFY(!tracker.hasActiveNote());

    resourcesModel.setNoteBacked(true);
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("resource-selection"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/resource-selection.wsnote"));
    QVERIFY(tracker.hasActiveNote());
    QVERIFY(hasActiveNoteChangedSpy.count() >= 4);

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}

void WhatSonCppRegressionTests::noteActiveStateTracker_syncsAttachedEditorSessionFromActiveNote()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeSelectionNoteListModel libraryModel;
    libraryModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/library-note.wsnote"));
    libraryModel.setCurrentBodyText(QStringLiteral("Library body"));

    FakeSelectionNoteListModel tagsModel;
    tagsModel.setCurrentNoteId(QStringLiteral("tags-note"));
    tagsModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/tags-note.wsnote"));
    tagsModel.setCurrentBodyText(QStringLiteral("Tags body"));

    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));
    libraryController.setNoteListModelObject(&libraryModel);
    tagsController.setNoteListModelObject(&tagsModel);

    HierarchyControllerProvider provider;
    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
    });

    SidebarHierarchyController sidebarController;
    sidebarController.setControllerProvider(&provider);

    NoteActiveStateTracker tracker;
    ContentsEditorSessionController editorSession;
    QSignalSpy sessionSynchronizedSpy(
        &editorSession,
        &ContentsEditorSessionController::editorTextSynchronized);

    tracker.setHierarchyContextSource(&sidebarController);
    tracker.setEditorSession(&editorSession);

    QCOMPARE(tracker.editorSession(), static_cast<QObject*>(&editorSession));
    QCOMPARE(tracker.activeNoteBodyText(), QStringLiteral("Library body"));
    QCOMPARE(editorSession.editorBoundNoteId(), QStringLiteral("library-note"));
    QCOMPARE(editorSession.editorBoundNoteDirectoryPath(), QStringLiteral("/tmp/library-note.wsnote"));
    QCOMPARE(editorSession.editorText(), QStringLiteral("Library body"));

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(tracker.activeNoteId(), QStringLiteral("tags-note"));
    QCOMPARE(tracker.activeNoteBodyText(), QStringLiteral("Tags body"));
    QCOMPARE(editorSession.editorBoundNoteId(), QStringLiteral("tags-note"));
    QCOMPARE(editorSession.editorBoundNoteDirectoryPath(), QStringLiteral("/tmp/tags-note.wsnote"));
    QCOMPARE(editorSession.editorText(), QStringLiteral("Tags body"));

    tagsModel.setCurrentBodyText(QStringLiteral("Tags body updated"));
    QCOMPARE(tracker.activeNoteBodyText(), QStringLiteral("Tags body updated"));
    QCOMPARE(editorSession.editorBoundNoteId(), QStringLiteral("tags-note"));
    QCOMPARE(editorSession.editorText(), QStringLiteral("Tags body updated"));
    QVERIFY(sessionSynchronizedSpy.count() >= 3);

    tracker.setEditorSession(nullptr);
    QCOMPARE(tracker.editorSession(), nullptr);

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}
