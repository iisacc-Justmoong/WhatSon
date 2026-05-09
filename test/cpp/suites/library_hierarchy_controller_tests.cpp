#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::libraryHierarchyController_keepsInAppScaffoldIndependentFromHubSnapshots()
{
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/library/LibraryHierarchyController.cpp"));
    const QString controllerDoc = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/file/hierarchy/library/LibraryHierarchyController.cpp.md"));

    QVERIFY(controllerSource.contains(QStringLiteral("constexpr auto kLibraryDraftLabel = \"Drafts\";")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryAllLabel), LibraryHierarchyItem::SystemBucket::All)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryDraftLabel), LibraryHierarchyItem::SystemBucket::Draft)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryTodayLabel), LibraryHierarchyItem::SystemBucket::Today)")));
    QVERIFY(controllerSource.contains(QStringLiteral("QVector<LibraryHierarchyItem> prependInAppLibraryScaffold(QVector<LibraryHierarchyItem> items)")));
    QVERIFY(controllerSource.contains(QStringLiteral("bool hasCompleteInAppLibraryScaffold(const QVector<LibraryHierarchyItem>& items)")));
    QVERIFY(controllerSource.contains(QStringLiteral("void LibraryHierarchyController::applyInAppLibraryScaffold()")));
    QVERIFY(controllerSource.contains(QStringLiteral("m_items = prependInAppLibraryScaffold({});")));
    QVERIFY(controllerSource.contains(QStringLiteral("const bool hierarchySourceChanged =\n        !hasCompleteInAppLibraryScaffold(m_items) || !folderDepthEntriesEqual(currentFolderEntries, folderEntries);")));
    QVERIFY(controllerSource.contains(QStringLiteral("applyInAppLibraryScaffold();\n    setSelectedIndex(-1);")));
    QVERIFY(!controllerSource.contains(QStringLiteral("m_items.clear();")));

    QVERIFY(controllerDoc.contains(QStringLiteral("hub-independent in-app scaffold")));
    QVERIFY(controllerDoc.contains(QStringLiteral("`All Library`, `Drafts`, and `Today`")));
}
