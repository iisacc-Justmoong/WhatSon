#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::progressHierarchySupport_defaultsFirstVisibleItemToFirstDraft()
{
    const QVector<ProgressHierarchyItem> items =
        WhatSon::Hierarchy::ProgressSupport::buildSupportedTypeItems({});

    QVERIFY(!items.isEmpty());
    QCOMPARE(items.constFirst().label, QStringLiteral("First draft"));
    QCOMPARE(
        WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, items.size()),
        0);
}
