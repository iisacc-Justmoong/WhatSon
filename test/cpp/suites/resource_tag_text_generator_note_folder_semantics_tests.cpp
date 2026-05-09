#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteFolderSemantics_normalizeDescriptorsAndXml()
{
    QCOMPARE(
        WhatSon::NoteFolders::normalizeFolderPath(QStringLiteral(" /Research//Today/ ")),
        QStringLiteral("Research/Today"));
    QCOMPARE(
        WhatSon::NoteFolders::normalizeFolderPath(QStringLiteral(" Marketing\\/Sales ")),
        QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::leafFolderName(QStringLiteral(" /Research//Today/ ")),
        QStringLiteral("Today"));
    QCOMPARE(
        WhatSon::NoteFolders::leafFolderName(QStringLiteral("Marketing\\/Sales")),
        QStringLiteral("Marketing/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::displayFolderPath(QStringLiteral("Marketing\\/Sales/Pipeline")),
        QStringLiteral("Marketing/Sales/Pipeline"));
    QCOMPARE(
        WhatSon::NoteFolders::folderPathSegments(QStringLiteral("Marketing\\/Sales/Pipeline")).size(),
        2);
    QVERIFY(!WhatSon::NoteFolders::isHierarchicalFolderPath(QStringLiteral("Marketing\\/Sales")));
    QVERIFY(WhatSon::NoteFolders::isHierarchicalFolderPath(QStringLiteral("Marketing\\/Sales/Pipeline")));
    QCOMPARE(
        WhatSon::NoteFolders::appendFolderPathSegment(QString(), QStringLiteral("Marketing/Sales")),
        QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::appendFolderPathSegment(
            QStringLiteral("Marketing\\/Sales"),
            QStringLiteral("Pipeline")),
        QStringLiteral("Marketing\\/Sales/Pipeline"));
    QVERIFY(WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/today")));
    QVERIFY(!WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/Tomorrow")));
    QVERIFY(!WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research\\/today")));

    const WhatSon::NoteFolders::RawFoldersBlockState missingFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(QStringLiteral("<header />"));
    QVERIFY(!missingFoldersBlock.blockPresent);
    QVERIFY(!missingFoldersBlock.hasConcreteEntry);

    const WhatSon::NoteFolders::RawFoldersBlockState emptyFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(
            QStringLiteral("<folders>\n  <folder>   </folder>\n</folders>"));
    QVERIFY(emptyFoldersBlock.blockPresent);
    QVERIFY(!emptyFoldersBlock.hasConcreteEntry);

    const WhatSon::NoteFolders::RawFoldersBlockState concreteFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(
            QStringLiteral("<folders><folder>Research</folder></folders>"));
    QVERIFY(concreteFoldersBlock.blockPresent);
    QVERIFY(concreteFoldersBlock.hasConcreteEntry);
}
