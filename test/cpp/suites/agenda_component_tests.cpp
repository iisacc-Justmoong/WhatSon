#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::agendaComponent_providesStaticTagTemplates()
{
    const QStringList names = WhatSon::EditorComponent::Agenda::staticTagNames();
    QCOMPARE(names, QStringList({QStringLiteral("agenda"), QStringLiteral("task")}));

    const WhatSon::EditorComponent::AgendaStaticTag agendaTag =
        WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("Agenda"));
    QVERIFY(agendaTag.isValid());
    QCOMPARE(agendaTag.canonicalName, QStringLiteral("agenda"));
    QCOMPARE(agendaTag.openingToken, QStringLiteral("<agenda><task>"));
    QCOMPARE(agendaTag.closingToken, QStringLiteral("</task></agenda>"));

    const WhatSon::EditorComponent::AgendaStaticTag taskTag =
        WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("task"));
    QVERIFY(taskTag.isValid());
    QCOMPARE(taskTag.canonicalName, QStringLiteral("task"));
    QCOMPARE(taskTag.openingToken, QStringLiteral("<task>"));
    QCOMPARE(taskTag.closingToken, QStringLiteral("</task>"));

    QVERIFY(!WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("callout")).isValid());
}
