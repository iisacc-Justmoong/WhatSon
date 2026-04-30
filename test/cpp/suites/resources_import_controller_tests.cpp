#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcesImportController_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/import/ResourcesImportController.cpp"));

    QVERIFY(!importControllerSource.isEmpty());
    QVERIFY(importControllerSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importControllerSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
}
