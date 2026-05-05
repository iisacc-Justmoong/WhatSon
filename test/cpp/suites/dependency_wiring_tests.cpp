#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <iiHtmlBlock.h>
#include <iiXml.h>

void WhatSonCppRegressionTests::cmakeDependencyWiring_declaresLocalXmlAndHtmlBlockPackages()
{
    const QString rootCmakeSource = readUtf8SourceFile(QStringLiteral("CMakeLists.txt"));
    const QString appRuntimeCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/cmake/runtime/CMakeLists.txt"));
    const QString testCmakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));

    QVERIFY(!rootCmakeSource.isEmpty());
    QVERIFY(!appRuntimeCmakeSource.isEmpty());
    QVERIFY(!testCmakeSource.isEmpty());

    QVERIFY(rootCmakeSource.contains(QStringLiteral("set(IIXML_PREFIX")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("set(IIHTMLBLOCK_PREFIX")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("WHATSON_IIXML_IOS_PREFIX")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("WHATSON_IIHTMLBLOCK_IOS_PREFIX")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("whatson_require_ios_local_package(iiXml")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("whatson_require_ios_local_package(iiHtmlBlock")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("find_package(iiXml 0.1.0 CONFIG REQUIRED)")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("find_package(iiHtmlBlock 0.1.0 CONFIG REQUIRED)")));

    QVERIFY(appRuntimeCmakeSource.contains(QStringLiteral("iiXml::iiXml")));
    QVERIFY(appRuntimeCmakeSource.contains(QStringLiteral("iiHtmlBlock::iiHtmlBlock")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("iiXml::iiXml")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("iiHtmlBlock::iiHtmlBlock")));
}
