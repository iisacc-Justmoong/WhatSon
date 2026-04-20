#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourceTagTextGenerator_and_noteFolderSemantics_normalizeDescriptorsAndXml()
{
    ContentsResourceTagTextGenerator generator;
    QSignalSpy generatedTagTextChangedSpy(
        &generator,
        &ContentsResourceTagTextGenerator::lastGeneratedTagTextChanged);
    QSignalSpy generatedDescriptorChangedSpy(
        &generator,
        &ContentsResourceTagTextGenerator::lastGeneratedDescriptorChanged);

    const QVariantMap importedResourceEntry{
        {QStringLiteral("resourcePath"), QStringLiteral("images/&lt;Cover&gt;.PNG")},
        {QStringLiteral("bucket"), QStringLiteral("Image")},
        {QStringLiteral("id"), QStringLiteral("res-1")},
    };

    const QVariantMap normalizedDescriptor = generator.normalizeImportedResourceEntry(importedResourceEntry);
    QVERIFY(normalizedDescriptor.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        normalizedDescriptor.value(QStringLiteral("resourcePath")).toString(),
        QStringLiteral("images/<Cover>.PNG"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("format")).toString(), QStringLiteral(".PNG"));

    const QString generatedTagText = generator.buildCanonicalResourceTag(importedResourceEntry);
    QCOMPARE(
        generatedTagText,
        QStringLiteral("<resource type=\"image\" format=\".PNG\" path=\"images/&lt;Cover&gt;.PNG\" id=\"res-1\" />"));
    QCOMPARE(generator.lastGeneratedTagText(), generatedTagText);
    QCOMPARE(
        generator.lastGeneratedDescriptor().value(QStringLiteral("resourcePath")).toString(),
        QStringLiteral("images/<Cover>.PNG"));
    QCOMPARE(generatedTagTextChangedSpy.count(), 1);
    QCOMPARE(generatedDescriptorChangedSpy.count(), 1);

    QCOMPARE(generator.buildCanonicalResourceTag(importedResourceEntry), generatedTagText);
    QCOMPARE(generatedTagTextChangedSpy.count(), 1);
    QCOMPARE(generatedDescriptorChangedSpy.count(), 1);

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
