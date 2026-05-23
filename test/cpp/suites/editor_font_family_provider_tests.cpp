#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorFontFamilyProvider_buildsMenuItemsFromSystemFamilies()
{
    const QStringList normalizedFamilies = EditorFontFamilyProvider::normalizedFontFamiliesForMenu({
        QStringLiteral(" Beta "),
        QString(),
        QStringLiteral("Alpha"),
        QStringLiteral("Alpha"),
        QStringLiteral("Gamma")
    });
    QCOMPARE(normalizedFamilies, QStringList({
        QStringLiteral("Alpha"),
        QStringLiteral("Beta"),
        QStringLiteral("Gamma")
    }));

    const QVariantList menuItems = EditorFontFamilyProvider::fontFamilyMenuItemsForFamilies({
        QStringLiteral(" Beta "),
        QStringLiteral("Alpha"),
        QStringLiteral("Gamma"),
        QStringLiteral("Alpha")
    });
    QCOMPARE(menuItems.size(), 3);

    const QVariantMap firstItem = menuItems.at(0).toMap();
    QCOMPARE(firstItem.value(QStringLiteral("label")).toString(), QStringLiteral("Alpha"));
    QCOMPARE(firstItem.value(QStringLiteral("fontFamily")).toString(), QStringLiteral("Alpha"));
    QCOMPARE(firstItem.value(QStringLiteral("eventName")).toString(), QStringLiteral("editor.toolbar.font"));
    QVERIFY(firstItem.contains(QStringLiteral("showIconSlot")));
    QCOMPARE(firstItem.value(QStringLiteral("showIconSlot")).toBool(), false);
    QCOMPARE(
        firstItem.value(QStringLiteral("eventPayload")).toMap().value(QStringLiteral("fontFamily")).toString(),
        QStringLiteral("Alpha"));

    EditorFontFamilyProvider provider;
    QSignalSpy hookSpy(&provider, &EditorFontFamilyProvider::providerHookRequested);
    provider.requestProviderHook(QStringLiteral("font-menu"));
    QCOMPARE(hookSpy.size(), 1);
    QCOMPARE(hookSpy.takeFirst().at(0).toString(), QStringLiteral("font-menu"));
    QCOMPARE(provider.fontFamilyMenuItems().size(), provider.fontFamilies().size());
    QCOMPARE(provider.fontFamilyCount(), provider.fontFamilies().size());
}
