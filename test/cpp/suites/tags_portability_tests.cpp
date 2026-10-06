#include "test/cpp/whatson_cpp_regression_tests.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyCreator.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "app/models/hierarchy/tags/WhatSonHubTagsStateStore.hpp"

void WhatSonCppRegressionTests::tagsHierarchy_roundTripAndHubIsolation()
{
    QVERIFY(!WhatSon::HubPath::isNonLocalUrl(QStringLiteral("D:/Workspace/Example.wshub")));
    QVERIFY(!WhatSon::HubPath::isNonLocalUrl(QStringLiteral("D:\\Workspace\\Example.wshub")));
    QVERIFY(!WhatSon::HubPath::isNonLocalUrl(QStringLiteral("\\\\server\\share\\Example.wshub")));
    QVERIFY(!WhatSon::HubPath::isNonLocalUrl(QStringLiteral("file:///D:/Example.wshub")));
    QVERIFY(WhatSon::HubPath::isNonLocalUrl(QStringLiteral("https://example.invalid/Example.wshub")));
    WhatSonTagsHierarchyStore source;
    source.setTagEntries({{QStringLiteral("Work"), QString::fromUtf8("자료"), 0, {}},
                          {QStringLiteral("Work/Next"), QStringLiteral("Next"), 1, {}}});
    const auto expected = source.tagEntries();
    WhatSonTagsHierarchyCreator creator;
    QCOMPARE(creator.targetRelativePath(), QStringLiteral("Tags.wstags"));
    const auto json = creator.createText(source);
    QVERIFY(json.contains(QStringLiteral("whatson.tags.list")));
    WhatSonTagsHierarchyParser parser;
    WhatSonTagsHierarchyStore parsed;
    QString error;
    QVERIFY2(parser.parse(json, &parsed, &error), qPrintable(error));
    QCOMPARE(parsed.tagEntries().size(), expected.size());
    for (qsizetype i = 0; i < expected.size(); ++i) {
        QCOMPARE(parsed.tagEntries()[i].id, expected[i].id);
        QCOMPARE(parsed.tagEntries()[i].label, expected[i].label);
        QCOMPARE(parsed.tagEntries()[i].depth, expected[i].depth);
    }
    QVERIFY(!parser.parse(QStringLiteral("{broken"), &parsed, &error));
    QVERIFY(!error.isEmpty());
    WhatSonHubTagsStateStore state;
    state.setEntries(QStringLiteral("hub-a.wshub"), expected);
    state.setEntries(QStringLiteral("hub-b.wshub"), {});
    auto copy = state;
    copy.remove(QStringLiteral("hub-a.wshub"));
    QVERIFY(state.contains(QStringLiteral("hub-a.wshub")));
    QVERIFY(!copy.contains(QStringLiteral("hub-a.wshub")));
    QVERIFY(copy.contains(QStringLiteral("hub-b.wshub")));
    QCOMPARE(state.entries(QStringLiteral("hub-a.wshub")).size(), expected.size());
}
