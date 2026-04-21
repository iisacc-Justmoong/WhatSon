#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::iosBundleIconPackaging_declaresPrimaryAndIpadFallbackIconsInInfoPlist()
{
    const QString infoPlistSource = readUtf8SourceFile(QStringLiteral("platform/Apple/iOS/Info.plist"));

    QVERIFY(!infoPlistSource.isEmpty());
    QVERIFY(infoPlistSource.contains(QStringLiteral("<key>CFBundleIcons</key>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<key>CFBundleIcons~ipad</key>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<key>CFBundlePrimaryIcon</key>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<key>CFBundleIconName</key>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon</string>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon-60</string>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon-Notification</string>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon-Spotlight-40</string>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon-76~ipad</string>")));
    QVERIFY(infoPlistSource.contains(QStringLiteral("<string>AppIcon-83.5~ipad</string>")));
}

void WhatSonCppRegressionTests::iosBundleIconPackaging_stagesBundleRootPngsEvenWithAssetCatalogsEnabled()
{
    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    const QString resourceCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/cmake/resources/CMakeLists.txt"));

    QVERIFY(!appCmakeSource.isEmpty());
    QVERIFY(!resourceCmakeSource.isEmpty());
    QVERIFY(appCmakeSource.contains(
        QStringLiteral("Some generated Xcode projects still miss the asset catalog Resources-phase entry")));
    QVERIFY(appCmakeSource.contains(
        QStringLiteral("Xcode does not reliably attach MACOSX_PACKAGE_LOCATION \".\" image sources")));
    QVERIFY(appCmakeSource.contains(QStringLiteral("WHATSON_IOS_POST_BUILD_BUNDLE_ICON_FILES")));
    QVERIFY(appCmakeSource.contains(QStringLiteral("add_custom_command(TARGET WhatSon POST_BUILD")));
    QVERIFY(appCmakeSource.contains(QStringLiteral("copy_if_different")));
    QVERIFY(appCmakeSource.contains(
        QStringLiteral("$<TARGET_BUNDLE_DIR:WhatSon>/${_whatson_ios_bundle_icon_name}")));
    QVERIFY(resourceCmakeSource.contains(
        QStringLiteral("foreach (_whatson_ios_bundle_icon_file IN LISTS WHATSON_IOS_BUNDLE_ICON_FILES)")));
    QVERIFY(resourceCmakeSource.contains(
        QStringLiteral("set(WHATSON_IOS_POST_BUILD_BUNDLE_ICON_FILES")));
    QVERIFY(resourceCmakeSource.contains(
        QStringLiteral("XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME \"AppIcon\"")));
}
