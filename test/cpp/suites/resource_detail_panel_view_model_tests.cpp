#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourceDetailPanelViewModel_tracksCurrentResourceSelection()
{
    ResourceDetailPanelViewModel viewModel;
    ResourcesListModel resourceListModel;

    QSignalSpy resourceListModelChangedSpy(
        &viewModel,
        &ResourceDetailPanelViewModel::resourceListModelChanged);
    QSignalSpy currentResourceEntryChangedSpy(
        &viewModel,
        &ResourceDetailPanelViewModel::currentResourceEntryChanged);

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

    viewModel.setCurrentResourceListModel(&resourceListModel);

    QCOMPARE(resourceListModelChangedSpy.count(), 1);
    QVERIFY(viewModel.resourceContextLinked());
    QCOMPARE(viewModel.resourceListModel(), static_cast<QObject*>(&resourceListModel));
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        secondItem.resourcePath);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        secondItem.displayName);

    resourceListModel.setCurrentIndex(0);

    QVERIFY(currentResourceEntryChangedSpy.count() >= 2);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        firstItem.resourcePath);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        firstItem.displayName);

    viewModel.setCurrentResourceListModel(nullptr);

    QCOMPARE(resourceListModelChangedSpy.count(), 2);
    QVERIFY(!viewModel.resourceContextLinked());
    QVERIFY(viewModel.currentResourceEntry().isEmpty());
}
