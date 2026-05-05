#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::iosXcodeprojExport_surfacesSdkSigningAndPermissionPolicyOptionsInCmake()
{
    const QString rootCmakeSource = readUtf8SourceFile(QStringLiteral("CMakeLists.txt"));
    const QString runtimeCmakeSource = readUtf8SourceFile(QStringLiteral("cmake/root/runtime/CMakeLists.txt"));

    QVERIFY(!rootCmakeSource.isEmpty());
    QVERIFY(!runtimeCmakeSource.isEmpty());
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_SDK \"iphoneos\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_ARCHITECTURES \"\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_DEVELOPMENT_TEAM \"$ENV{WHATSON_IOS_DEVELOPMENT_TEAM}\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_CODE_SIGN_IDENTITY \"$ENV{WHATSON_IOS_CODE_SIGN_IDENTITY}\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_CODE_SIGN_STYLE \"Automatic\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY \"auto\" CACHE STRING")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IIXML_IOS_PREFIX \"$ENV{WHATSON_IIXML_IOS_PREFIX}\" CACHE PATH")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("set(WHATSON_IIHTMLBLOCK_IOS_PREFIX \"$ENV{WHATSON_IIHTMLBLOCK_IOS_PREFIX}\" CACHE PATH")));
    QVERIFY(rootCmakeSource.contains(
        QStringLiteral("WhatSon iOS build requires an iOS ${package_name} package")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("set(_whatson_ios_export_sdk \"${WHATSON_IOS_SDK}\")")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("set(_whatson_ios_export_architectures \"${WHATSON_IOS_ARCHITECTURES}\")")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DCMAKE_OSX_SYSROOT=${_whatson_ios_export_sdk}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DCMAKE_OSX_ARCHITECTURES=${_whatson_ios_export_architectures}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_SDK=${_whatson_ios_export_sdk}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_ARCHITECTURES=${_whatson_ios_export_architectures}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_DEVELOPMENT_TEAM=${WHATSON_IOS_DEVELOPMENT_TEAM}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_CODE_SIGN_IDENTITY=${WHATSON_IOS_CODE_SIGN_IDENTITY}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_CODE_SIGN_STYLE=${WHATSON_IOS_CODE_SIGN_STYLE}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY=${WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY}\"")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("-D${_whatson_ios_prefix_var}=${_whatson_ios_package_prefix}")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("-D${_whatson_ios_package_dir_var}=${_whatson_ios_package_config_dir}")));
    QVERIFY(runtimeCmakeSource.contains(
        QStringLiteral("if (_whatson_ios_export_sdk STREQUAL \"iphonesimulator\")")));
}

void WhatSonCppRegressionTests::iosXcodeprojExport_routesSimulatorPermissionFallbackThroughAppRuntimeCmake()
{
    const QString appRuntimeCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/cmake/runtime/CMakeLists.txt"));
    const QString exportRuntimeCmakeSource = readUtf8SourceFile(QStringLiteral("cmake/root/runtime/CMakeLists.txt"));
    const QString xcodeprojPatchSource = readUtf8SourceFile(QStringLiteral("cmake/patch_whatson_ios_xcodeproj.py"));
    const QString permissionBootstrapperSource =
        readUtf8SourceFile(QStringLiteral("src/app/permissions/WhatSonPermissionBootstrapper.cpp"));

    QVERIFY(!appRuntimeCmakeSource.isEmpty());
    QVERIFY(!exportRuntimeCmakeSource.isEmpty());
    QVERIFY(!xcodeprojPatchSource.isEmpty());
    QVERIFY(!permissionBootstrapperSource.isEmpty());
    QVERIFY(appRuntimeCmakeSource.contains(
        QStringLiteral("qt_import_plugins(WhatSon EXCLUDE_BY_TYPE permissions)")));
    QVERIFY(appRuntimeCmakeSource.contains(
        QStringLiteral("WHATSON_DISABLE_QT_PERMISSION_REQUESTS=1")));
    QVERIFY(appRuntimeCmakeSource.contains(
        QStringLiteral("WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY STREQUAL \"auto\"")));
    QVERIFY(appRuntimeCmakeSource.contains(
        QStringLiteral("Some Qt iOS kits ship device-only arm64 slices for permissions plugins")));
    QVERIFY(exportRuntimeCmakeSource.contains(
        QStringLiteral("--strip-qt-permissions")));
    QVERIFY(xcodeprojPatchSource.contains(
        QStringLiteral("strip_qt_permission_link_inputs")));
    QVERIFY(xcodeprojPatchSource.contains(
        QStringLiteral("--strip-qt-permissions")));
    QVERIFY(xcodeprojPatchSource.contains(
        QStringLiteral("QDarwinMicrophonePermissionRequest")));
    QVERIFY(xcodeprojPatchSource.contains(
        QStringLiteral("plugins/permissions/libqdarwin")));
    QVERIFY(permissionBootstrapperSource.contains(
        QStringLiteral("#if QT_CONFIG(permissions) && !defined(WHATSON_DISABLE_QT_PERMISSION_REQUESTS)")));
}

void WhatSonCppRegressionTests::iosXcodeprojExport_patchScriptStripsQtPermissionsEvenWhenIconPhaseAlreadyExists()
{
    const QString pythonExecutable = QStandardPaths::findExecutable(QStringLiteral("python3"));
    QVERIFY2(!pythonExecutable.isEmpty(), "python3 executable is required for Xcode project patch regression tests.");

    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString pbxprojPath = temporaryDirectory.filePath(QStringLiteral("project.pbxproj"));
    QFile pbxprojFile(pbxprojPath);
    QVERIFY(pbxprojFile.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray pbxprojSource = R"PBX(/* Begin PBXBuildFile section */
		2E0B1AA07BA14B0EBC55FB19 /* build/ios-xcode-artifact/src/app/cmake/resources/WhatSonIcons.xcassets */ = {isa = PBXBuildFile; fileRef = C9BAE897DD0A4DE6933ED668 /* WhatSonIcons.xcassets */; };
/* End PBXBuildFile section */

/* Begin PBXNativeTarget section */
		1234567890ABCDEF12345678 /* WhatSon */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = 111111111111111111111111 /* Build configuration list for PBXNativeTarget "WhatSon" */;
			buildPhases = (
				87654321FEDCBA0987654321 /* Resources */,
			);
			dependencies = (
			);
			name = WhatSon;
		};
/* End PBXNativeTarget section */

/* Begin PBXResourcesBuildPhase section */
		87654321FEDCBA0987654321 /* Resources */ = {
			isa = PBXResourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				2E0B1AA07BA14B0EBC55FB19 /* build/ios-xcode-artifact/src/app/cmake/resources/WhatSonIcons.xcassets */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXResourcesBuildPhase section */

		buildSettings = {
			OTHER_LDFLAGS = ("-Xlinker -no_warn_duplicate_libraries '-Wl,-e,_qt_main_wrapper' '-Wl,-u,_QDarwinMicrophonePermissionRequest' -Wl,-search_paths_first","/tmp/Qt/ios/plugins/permissions/objects-Release/QDarwinBluetoothPermissionPlugin_init/QDarwinBluetoothPermissionPlugin_init.cpp.o",/tmp/Qt/ios/plugins/permissions/libqdarwinbluetoothpermission.a,"$(inherited)");
		};
)PBX";
    QVERIFY(pbxprojFile.write(pbxprojSource) == pbxprojSource.size());
    pbxprojFile.close();

    QProcess patchProcess;
    patchProcess.setProgram(pythonExecutable);
    patchProcess.setArguments({
        QStringLiteral("cmake/patch_whatson_ios_xcodeproj.py"),
        QStringLiteral("--strip-qt-permissions"),
        pbxprojPath,
    });
    patchProcess.setWorkingDirectory(QDir::currentPath());
    patchProcess.start();
    QVERIFY2(patchProcess.waitForFinished(), "patch_whatson_ios_xcodeproj.py did not finish.");
    QVERIFY2(
        patchProcess.exitStatus() == QProcess::NormalExit && patchProcess.exitCode() == 0,
        qPrintable(QString::fromUtf8(patchProcess.readAllStandardError())));

    const QString patchedProject = readUtf8SourceFile(pbxprojPath);
    QVERIFY(patchedProject.contains(QStringLiteral("2E0B1AA07BA14B0EBC55FB19")));
    QVERIFY(patchedProject.contains(QStringLiteral("WhatSonIcons.xcassets")));
    QVERIFY(!patchedProject.contains(QStringLiteral("QDarwinMicrophonePermissionRequest")));
    QVERIFY(!patchedProject.contains(QStringLiteral("QDarwinBluetoothPermissionPlugin_init.cpp.o")));
    QVERIFY(!patchedProject.contains(QStringLiteral("libqdarwinbluetoothpermission.a")));
}

void WhatSonCppRegressionTests::iosXcodeprojExport_keepsBuildIosScriptOnHighLevelCmakeOptions()
{
    const QString buildPlatformRunnerSource =
        readUtf8SourceFile(QStringLiteral("scripts/build_platform_runner.py"));

    QVERIFY(!buildPlatformRunnerSource.isEmpty());
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_SDK=iphoneos\"")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_ARCHITECTURES=arm64\"")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("f\"-DWHATSON_APPLE_BUNDLE_ID={self.ios_bundle_id}\"")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("f\"-DWHATSON_IOS_DEVELOPMENT_TEAM={resolved_ios_development_team}\"")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("\"-DWHATSON_IOS_CODE_SIGN_STYLE=Automatic\"")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("ios_cmd.append(f\"-DWHATSON_IOS_CODE_SIGN_IDENTITY={self.ios_code_sign_identity}\")")));
    QVERIFY(buildPlatformRunnerSource.contains(
        QStringLiteral("patch_whatson_ios_xcodeproj.py")));
    QVERIFY(!buildPlatformRunnerSource.contains(
        QStringLiteral("f\"-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM={resolved_ios_development_team}\"")));
    QVERIFY(!buildPlatformRunnerSource.contains(
        QStringLiteral("f\"-DCMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER={self.ios_bundle_id}\"")));
    QVERIFY(!buildPlatformRunnerSource.contains(
        QStringLiteral("\"-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic\"")));
}
