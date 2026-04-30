#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourceDetailPanelController_tracksCurrentResourceSelection()
{
    ResourceDetailPanelController controller;
    ResourcesListModel resourceListModel;

    QSignalSpy resourceListModelChangedSpy(
        &controller,
        &ResourceDetailPanelController::resourceListModelChanged);
    QSignalSpy currentResourceEntryChangedSpy(
        &controller,
        &ResourceDetailPanelController::currentResourceEntryChanged);

    ResourcesListItem firstItem;
    firstItem.id = QStringLiteral("resource-1");
    firstItem.displayName = QStringLiteral("Cover");
    firstItem.resourcePath = QStringLiteral("resources/cover.png");
    firstItem.resolvedPath = QStringLiteral("/tmp/resources/cover.png");
    firstItem.type = QStringLiteral("image");
    firstItem.renderMode = QStringLiteral("image");

    ResourcesListItem secondItem;
    secondItem.id = QStringLiteral("resource-2");
    secondItem.displayName = QStringLiteral("Palette");
    secondItem.resourcePath = QStringLiteral("resources/palette.png");
    secondItem.resolvedPath = QStringLiteral("/tmp/resources/palette.png");
    secondItem.type = QStringLiteral("image");
    secondItem.renderMode = QStringLiteral("image");

    resourceListModel.setItems({firstItem, secondItem});
    resourceListModel.setCurrentIndex(1);

    controller.setCurrentResourceListModel(&resourceListModel);

    QCOMPARE(resourceListModelChangedSpy.count(), 1);
    QVERIFY(controller.resourceContextLinked());
    QCOMPARE(controller.resourceListModel(), static_cast<QObject*>(&resourceListModel));
    QCOMPARE(
        controller.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        secondItem.resourcePath);
    QCOMPARE(
        controller.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        secondItem.displayName);

    resourceListModel.setCurrentIndex(0);

    QVERIFY(currentResourceEntryChangedSpy.count() >= 2);
    QCOMPARE(
        controller.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        firstItem.resourcePath);
    QCOMPARE(
        controller.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        firstItem.displayName);

    controller.setCurrentResourceListModel(nullptr);

    QCOMPARE(resourceListModelChangedSpy.count(), 2);
    QVERIFY(!controller.resourceContextLinked());
    QVERIFY(controller.currentResourceEntry().isEmpty());
}
