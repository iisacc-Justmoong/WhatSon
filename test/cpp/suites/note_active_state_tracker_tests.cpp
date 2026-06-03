#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteActiveStateTracker_tracksCurrentNoteAcrossActiveHierarchyChanges()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeSelectionNoteListModel libraryModel;
    libraryModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/library-note.note"));

    FakeSelectionNoteListModel tagsModel;
    tagsModel.setCurrentNoteId(QStringLiteral("tags-note"));
    tagsModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/tags-note.note"));

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
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/library-note.note"));
    QVERIFY(tracker.hasActiveNote());
    QCOMPARE(
        tracker.activeNoteEntry().value(QStringLiteral("noteId")).toString(),
        QStringLiteral("library-note"));
    QVERIFY(!tracker.activeNoteEntry().contains(QStringLiteral("bodyText")));

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(tracker.activeHierarchyIndex(), static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(tracker.activeHierarchyController(), static_cast<QObject*>(&tagsController));
    QCOMPARE(tracker.activeNoteListModel(), static_cast<QObject*>(&tagsModel));
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("tags-note"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/tags-note.note"));

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
        {QStringLiteral("noteDirectoryPath"), QStringLiteral("/tmp/entry-note.note")},
        {QStringLiteral("bodyText"), QStringLiteral("Entry body")},
    });

    FakeSelectionNoteListModel resourcesModel;
    resourcesModel.setCurrentNoteId(QStringLiteral("resource-selection"));
    resourcesModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/resource-selection.note"));
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
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/entry-note.note"));
    QVERIFY(!tracker.activeNoteEntry().contains(QStringLiteral("bodyText")));
    QVERIFY(tracker.hasActiveNote());

    libraryModel.setCurrentNoteEntry({});
    QVERIFY(tracker.activeNoteId().isEmpty());
    QVERIFY(tracker.activeNoteDirectoryPath().isEmpty());
    QVERIFY(!tracker.hasActiveNote());

    libraryModel.setCurrentNoteEntry({{QStringLiteral("id"), QStringLiteral("entry-note-2")}});
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("entry-note-2"));
    QVERIFY(tracker.hasActiveNote());

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));
    QVERIFY(tracker.activeNoteId().isEmpty());
    QVERIFY(!tracker.hasActiveNote());

    resourcesModel.setNoteBacked(true);
    QCOMPARE(tracker.activeNoteId(), QStringLiteral("resource-selection"));
    QCOMPARE(tracker.activeNoteDirectoryPath(), QStringLiteral("/tmp/resource-selection.note"));
    QVERIFY(tracker.hasActiveNote());
    QVERIFY(hasActiveNoteChangedSpy.count() >= 4);

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}

void WhatSonCppRegressionTests::noteActiveStateTracker_publishesAtomicNoteSnapshotBeforeChangeSignals()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeSelectionNoteListModel libraryModel;
    libraryModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/library-note.note"));
    libraryModel.setCurrentBodyText(QStringLiteral("Library body"));

    FakeSelectionNoteListModel tagsModel;
    tagsModel.setCurrentNoteId(QStringLiteral("tags-note"));
    tagsModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/tags-note.note"));
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
    QString noteDirectoryPathDuringTagsNoteIdSignal;
    QVariantMap noteEntryDuringTagsNoteIdSignal;
    QObject::connect(
        &tracker,
        &NoteActiveStateTracker::activeNoteIdChanged,
        &tracker,
        [
            &tracker,
            &noteDirectoryPathDuringTagsNoteIdSignal,
            &noteEntryDuringTagsNoteIdSignal
        ]()
        {
            if (tracker.activeNoteId() != QStringLiteral("tags-note"))
            {
                return;
            }
            noteDirectoryPathDuringTagsNoteIdSignal = tracker.activeNoteDirectoryPath();
            noteEntryDuringTagsNoteIdSignal = tracker.activeNoteEntry();
        });

    tracker.setHierarchyContextSource(&sidebarController);
    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(noteDirectoryPathDuringTagsNoteIdSignal, QStringLiteral("/tmp/tags-note.note"));
    QCOMPARE(
        noteEntryDuringTagsNoteIdSignal.value(QStringLiteral("noteId")).toString(),
        QStringLiteral("tags-note"));
    QVERIFY(!noteEntryDuringTagsNoteIdSignal.contains(QStringLiteral("bodyText")));

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}
