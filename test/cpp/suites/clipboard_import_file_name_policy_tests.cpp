#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::clipboardImportFileNamePolicy_generatesRandom32CharacterAlphaNumericPngNames()
{
    const QString firstFileName = WhatSon::Resources::generateClipboardImportAssetFileName();
    const QString secondFileName = WhatSon::Resources::generateClipboardImportAssetFileName();

    const QRegularExpression expectedPattern(QStringLiteral("^[A-Za-z0-9]{32}\\.png$"));
    QVERIFY(expectedPattern.match(firstFileName).hasMatch());
    QVERIFY(expectedPattern.match(secondFileName).hasMatch());
    QVERIFY(firstFileName != secondFileName);
}
