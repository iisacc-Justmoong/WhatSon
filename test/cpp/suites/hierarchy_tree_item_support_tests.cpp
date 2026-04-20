#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::hierarchyTreeItemSupport_clampsNegativeSelectionToFirstVisibleRow()
{
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, 0), -1);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, 4), 0);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(0, 4), 0);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(3, 4), 3);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(99, 4), 3);
}
