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

void WhatSonCppRegressionTests::noteActiveStateTracker_publishesAtomicNoteSnapshotBeforeChangeSignals()
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
    QString bodyTextDuringTagsNoteIdSignal;
    QString noteDirectoryPathDuringTagsNoteIdSignal;
    QVariantMap noteEntryDuringTagsNoteIdSignal;
    QObject::connect(
        &tracker,
        &NoteActiveStateTracker::activeNoteIdChanged,
        &tracker,
        [
            &tracker,
            &bodyTextDuringTagsNoteIdSignal,
            &noteDirectoryPathDuringTagsNoteIdSignal,
            &noteEntryDuringTagsNoteIdSignal
        ]()
        {
            if (tracker.activeNoteId() != QStringLiteral("tags-note"))
            {
                return;
            }
            bodyTextDuringTagsNoteIdSignal = tracker.activeNoteBodyText();
            noteDirectoryPathDuringTagsNoteIdSignal = tracker.activeNoteDirectoryPath();
            noteEntryDuringTagsNoteIdSignal = tracker.activeNoteEntry();
        });

    tracker.setHierarchyContextSource(&sidebarController);
    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(bodyTextDuringTagsNoteIdSignal, QStringLiteral("Tags body"));
    QCOMPARE(noteDirectoryPathDuringTagsNoteIdSignal, QStringLiteral("/tmp/tags-note.wsnote"));
    QCOMPARE(
        noteEntryDuringTagsNoteIdSignal.value(QStringLiteral("noteId")).toString(),
        QStringLiteral("tags-note"));
    QVERIFY(!noteEntryDuringTagsNoteIdSignal.contains(QStringLiteral("bodyText")));
    QCOMPARE(tracker.activeNoteBodyText(), QStringLiteral("Tags body"));

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}

void WhatSonCppRegressionTests::noteActiveStateTracker_publishesBodyPathForLvrsTextEditorBinding()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString firstNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("first-note"),
        QStringLiteral("first body"),
        &createError);
    QVERIFY2(!firstNoteDirectoryPath.isEmpty(), qPrintable(createError));

    const QString secondNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("second-note"),
        QStringLiteral("second body"),
        &createError);
    QVERIFY2(!secondNoteDirectoryPath.isEmpty(), qPrintable(createError));

    LibraryNoteListItem firstItem;
    firstItem.id = QStringLiteral("first-note");
    firstItem.noteDirectoryPath = firstNoteDirectoryPath;
    firstItem.primaryText = QStringLiteral("First note");
    firstItem.bodyText = QStringLiteral("first body");

    LibraryNoteListItem secondItem;
    secondItem.id = QStringLiteral("second-note");
    secondItem.noteDirectoryPath = secondNoteDirectoryPath;
    secondItem.primaryText = QStringLiteral("Second note");
    secondItem.bodyText = QStringLiteral("second body");

    LibraryNoteListModel noteListModel;
    noteListModel.setItems({firstItem, secondItem});
    noteListModel.setCurrentIndex(0);

    FakeHierarchyController libraryController(QStringLiteral("library"));
    libraryController.setNoteListModelObject(&noteListModel);

    HierarchyControllerProvider provider;
    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
    });

    SidebarHierarchyController sidebarController;
    sidebarController.setControllerProvider(&provider);

    NoteActiveStateTracker tracker;
    QSignalSpy activeNoteBodyPathChangedSpy(&tracker, &NoteActiveStateTracker::activeNoteBodyPathChanged);

    QString bodyPathDuringSecondNoteSignal;
    QObject::connect(
        &tracker,
        &NoteActiveStateTracker::activeNoteIdChanged,
        &tracker,
        [&tracker, &bodyPathDuringSecondNoteSignal]()
        {
            if (tracker.activeNoteId() == QStringLiteral("second-note"))
            {
                bodyPathDuringSecondNoteSignal = tracker.activeNoteBodyPath();
            }
        });

    tracker.setHierarchyContextSource(&sidebarController);

    QCOMPARE(
        tracker.activeNoteBodyPath(),
        WhatSon::NoteBodyPersistence::resolveBodyPath(firstNoteDirectoryPath));
    QVERIFY(QFileInfo(tracker.activeNoteBodyPath()).isFile());

    noteListModel.setCurrentIndex(1);

    QCOMPARE(
        tracker.activeNoteBodyPath(),
        WhatSon::NoteBodyPersistence::resolveBodyPath(secondNoteDirectoryPath));
    QCOMPARE(bodyPathDuringSecondNoteSignal, tracker.activeNoteBodyPath());
    QVERIFY(activeNoteBodyPathChangedSpy.count() >= 2);

    noteListModel.setCurrentIndex(-1);
    QVERIFY(tracker.activeNoteBodyPath().isEmpty());

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}
