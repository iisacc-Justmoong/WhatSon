#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcesHierarchyViewModel_defaultsSelectionToImageAndFiltersList()
{
    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({
        QStringLiteral("images/Cover.PNG"),
        QStringLiteral("documents/Report.pdf")
    });

    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Image"));
    QVERIFY(viewModel.noteListModel() != nullptr);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = viewModel.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        viewModel.noteListModel()->data(imageIndex, ResourcesListModel::TypeRole).toString(),
        QStringLiteral("image"));

    viewModel.setSelectedIndex(-1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);
}
