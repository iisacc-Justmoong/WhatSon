#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::appLaunchSupport_requiresMountedAndLoadedHubForStartupWorkspace()
{
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(false, false));
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(false, true));
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(true, false));
    QVERIFY(WhatSon::Runtime::Bootstrap::startupWorkspaceReady(true, true));
}
