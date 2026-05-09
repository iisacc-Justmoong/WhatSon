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
    QVERIFY(rootCmakeSource.contains(QStringLiteral("add_custom_target(whatson_build_all)")));
    QVERIFY(rootCmakeSource.contains(QStringLiteral("add_dependencies(whatson_build_all ${WHATSON_BUILD_TARGETS})")));
    QVERIFY(!rootCmakeSource.contains(QStringLiteral("add_custom_target(whatson_build_all DEPENDS ${WHATSON_BUILD_TARGETS})")));

    QVERIFY(appRuntimeCmakeSource.contains(QStringLiteral("iiXml::iiXml")));
    QVERIFY(appRuntimeCmakeSource.contains(QStringLiteral("iiHtmlBlock::iiHtmlBlock")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("iiXml::iiXml")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("iiHtmlBlock::iiHtmlBlock")));
}

void WhatSonCppRegressionTests::cmakeBuildTargets_cleanTransientBuildDiagnostics()
{
    const QString buildCmakeSource = readUtf8SourceFile(QStringLiteral("cmake/root/build/CMakeLists.txt"));
    const QString devCmakeSource = readUtf8SourceFile(QStringLiteral("cmake/root/dev/CMakeLists.txt"));
    const QString cleanupScriptSource = readUtf8SourceFile(QStringLiteral("cmake/CleanWhatSonBuildExtras.cmake"));

    QVERIFY(!buildCmakeSource.isEmpty());
    QVERIFY(!devCmakeSource.isEmpty());
    QVERIFY(!cleanupScriptSource.isEmpty());

    QVERIFY(buildCmakeSource.contains(QStringLiteral("add_custom_target(whatson_clean_build_extras")));
    QVERIFY(buildCmakeSource.contains(QStringLiteral("CleanWhatSonBuildExtras.cmake")));
    QVERIFY(buildCmakeSource.contains(QStringLiteral("add_dependencies(whatson_regression whatson_clean_build_extras)")));
    QVERIFY(buildCmakeSource.contains(QStringLiteral("COMMAND \"${CMAKE_CTEST_COMMAND}\" --output-on-failure -L cpp_regression")));

    QVERIFY(devCmakeSource.contains(QStringLiteral("set(WHATSON_DEV_FILELIST_DIR \"${CMAKE_BINARY_DIR}/CMakeFiles/whatson_dev\")")));
    QVERIFY(devCmakeSource.contains(QStringLiteral("${WHATSON_DEV_FILELIST_DIR}/whatson_qml_files.txt")));
    QVERIFY(devCmakeSource.contains(QStringLiteral("${WHATSON_DEV_FILELIST_DIR}/whatson_cpp_files.txt")));
    QVERIFY(!devCmakeSource.contains(QStringLiteral("${CMAKE_BINARY_DIR}/whatson_qml_files.txt")));
    QVERIFY(!devCmakeSource.contains(QStringLiteral("${CMAKE_BINARY_DIR}/whatson_cpp_files.txt")));

    QVERIFY(cleanupScriptSource.contains(QStringLiteral("whatson-*.png")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("*.wsnbody.backup.xml")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("*.wsnbody.pre-*-backup.xml")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral(".DS_Store")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("whatson_qmllint.latest.log")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("whatson_qmllint_diagnostic.log")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("whatson_cpp_files.txt")));
    QVERIFY(cleanupScriptSource.contains(QStringLiteral("whatson_qml_files.txt")));
}
