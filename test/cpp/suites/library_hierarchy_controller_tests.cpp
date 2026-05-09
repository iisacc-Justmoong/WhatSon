#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::libraryHierarchyController_synthesizesSystemBucketsForEmptyRuntimeSnapshots()
{
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/library/LibraryHierarchyController.cpp"));
    const QString hubCreatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hub/WhatSonHubCreator.cpp"));
    const QString controllerDoc = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/file/hierarchy/library/LibraryHierarchyController.cpp.md"));

    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryAllLabel), LibraryHierarchyItem::SystemBucket::All)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryDraftLabel), LibraryHierarchyItem::SystemBucket::Draft)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryTodayLabel), LibraryHierarchyItem::SystemBucket::Today)")));
    QVERIFY(controllerSource.contains(QStringLiteral("bool hasRequiredSystemBuckets(const QVector<LibraryHierarchyItem>& items)")));
    QVERIFY(controllerSource.contains(QStringLiteral("const bool hierarchySourceChanged =\n        !hasRequiredSystemBuckets(m_items) || !folderDepthEntriesEqual(currentFolderEntries, folderEntries);")));
    QVERIFY(controllerSource.contains(QStringLiteral("applyIndexedBuckets();")));

    QVERIFY(hubCreatorSource.contains(QStringLiteral("\"folders\": []")));
    QVERIFY(controllerDoc.contains(QStringLiteral("empty-folder runtime snapshot")));
    QVERIFY(controllerDoc.contains(QStringLiteral("`All Library`, `Draft`, and `Today`")));
}
