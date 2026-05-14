#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::breakComponent_projectsStandaloneBreakAsLogicalEditorLine()
{
    QVERIFY(WhatSon::EditorComponent::Break::isSourceLine(QStringLiteral("</break>")));
    QVERIFY(WhatSon::EditorComponent::Break::isSourceLine(QStringLiteral("<break />")));
    QVERIFY(WhatSon::EditorComponent::Break::isSourceLine(QStringLiteral("<hr/>")));
    QVERIFY(!WhatSon::EditorComponent::Break::isSourceLine(QStringLiteral("Alpha </break>")));
    QCOMPARE(WhatSon::EditorComponent::Break::sourceToken(), QStringLiteral("</break>"));
    QCOMPARE(WhatSon::EditorComponent::Break::renderHtml(), QString());

    const QString source = QStringLiteral("Alpha\n</break>\nBeta");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("break-note"),
        source);

    QCOMPARE(editorHtml, QStringLiteral("Alpha<br/><br/>Beta"));
    QVERIFY(!editorHtml.contains(QStringLiteral("</break>")));
    QVERIFY(!editorHtml.contains(QStringLiteral("&lt;/break&gt;")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(
            WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("break-note"), source)),
        source);
}
