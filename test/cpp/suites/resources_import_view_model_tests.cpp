#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcesImportViewModel_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importViewModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/import/ResourcesImportViewModel.cpp"));

    QVERIFY(!importViewModelSource.isEmpty());
    QVERIFY(importViewModelSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importViewModelSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
}
