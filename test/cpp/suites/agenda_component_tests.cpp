#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

void WhatSonCppRegressionTests::agendaComponent_providesStaticTagTemplates()
{
    const QStringList names = WhatSon::EditorComponent::Agenda::staticTagNames();
    QCOMPARE(names, QStringList({QStringLiteral("agenda"), QStringLiteral("task")}));

    const QString dateBefore = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    const QString timeBefore = QTime::currentTime().toString(QStringLiteral("HH-mm"));
    const WhatSon::EditorComponent::AgendaStaticTag agendaTag =
        WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("Agenda"));
    const QString dateAfter = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    const QString timeAfter = QTime::currentTime().toString(QStringLiteral("HH-mm"));
    QVERIFY(agendaTag.isValid());
    QCOMPARE(agendaTag.canonicalName, QStringLiteral("agenda"));
    const QRegularExpression agendaOpeningPattern(
        QStringLiteral(R"rx(^<agenda date="(\d{4}-\d{2}-\d{2})" time="(\d{2}-\d{2})"><task done=false>$)rx"));
    const QRegularExpressionMatch agendaOpeningMatch = agendaOpeningPattern.match(agendaTag.openingToken);
    QVERIFY(agendaOpeningMatch.hasMatch());
    QVERIFY(agendaOpeningMatch.captured(1) == dateBefore || agendaOpeningMatch.captured(1) == dateAfter);
    QVERIFY(agendaOpeningMatch.captured(2) == timeBefore || agendaOpeningMatch.captured(2) == timeAfter);
    QVERIFY(!agendaTag.openingToken.contains(QStringLiteral("yyyy-mm-dd")));
    QVERIFY(!agendaTag.openingToken.contains(QStringLiteral("hh-mm")));
    QCOMPARE(agendaTag.closingToken, QStringLiteral("</task></agenda>"));

    const WhatSon::EditorComponent::AgendaStaticTag taskTag =
        WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("task"));
    QVERIFY(taskTag.isValid());
    QCOMPARE(taskTag.canonicalName, QStringLiteral("task"));
    QCOMPARE(taskTag.openingToken, QStringLiteral("<task done=false>"));
    QCOMPARE(taskTag.closingToken, QStringLiteral("</task>"));

    QVERIFY(!WhatSon::EditorComponent::Agenda::staticTagFor(QStringLiteral("callout")).isValid());
}

void WhatSonCppRegressionTests::agendaComponent_rendersFigmaAgendaFrame()
{
    WhatSon::EditorComponent::AgendaDescriptor descriptor;
    descriptor.sourceText = QStringLiteral(
        "<agenda date=\"2026-05-18\" time=\"09-30\">"
        "<task done=false>Draft <bold>outline</bold></task>"
        "<task done=true>Ship patch</task>"
        "</agenda>");
    descriptor.dateText = QStringLiteral("2026-05-18");
    descriptor.timeText = QStringLiteral("09-30");
    descriptor.tasks = {
        {
            QStringLiteral("<task done=false>Draft <bold>outline</bold></task>"),
            QStringLiteral("Draft <strong style=\"font-weight:900;\">outline</strong>"),
            false
        },
        {
            QStringLiteral("<task done=true>Ship patch</task>"),
            QStringLiteral("Ship patch"),
            true
        }
    };

    const QString html = WhatSon::EditorComponent::Agenda::renderHtml(descriptor);

    QVERIFY(html.contains(QStringLiteral("<!--whatson-agenda-source:")));
    QVERIFY(html.contains(QStringLiteral("<div class=\"whatson-agenda\"")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"279:7854\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-width-mode=\"fill\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-height-mode=\"hug-contents\"")));
    QVERIFY(html.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(html.contains(QStringLiteral("width:100%")));
    QVERIFY(html.contains(QStringLiteral("max-width:100%")));
    QVERIFY(html.contains(QStringLiteral("height:auto")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-design-width")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(!html.contains(QStringLiteral("width=\"307\"")));
    QVERIFY(!html.contains(QStringLiteral("height=\"136\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-padding=\"8\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-header-gap=\"8\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-task-gap=\"4\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-task-inset=\"8\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-checkbox-size=\"17\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-checkbox-radius=\"3.5\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-checkbox-text-gap=\"6\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-task-component=\"LV.CheckBox\"")));
    QVERIFY(html.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(html.contains(QStringLiteral("border:1px solid #343536")));
    QVERIFY(html.contains(QStringLiteral("border-radius:12px")));
    QVERIFY(html.contains(QStringLiteral("padding:8px")));
    QVERIFY(html.contains(QStringLiteral("font-family:Pretendard")));
    QVERIFY(html.contains(QStringLiteral("font-size:11px")));
    QVERIFY(html.contains(QStringLiteral("font-size:12px")));
    QVERIFY(html.contains(QStringLiteral("Agenda")));
    QVERIFY(html.contains(QStringLiteral("2026-05-18")));
    QVERIFY(html.contains(QStringLiteral("Draft <strong style=\"font-weight:900;\">outline</strong>")));
    QVERIFY(html.contains(QStringLiteral("Ship patch")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-task-done=\"false\"")));
    QVERIFY(html.contains(QStringLiteral("data-agenda-task-done=\"true\"")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-agenda-checkbox\"")));
    QVERIFY(html.contains(QStringLiteral("data:image/png;base64,")));
    QVERIFY(!html.contains(QStringLiteral("<agenda")));
    QVERIFY(!html.contains(QStringLiteral("<task done")));

    QTextDocument editorDocument;
    editorDocument.setHtml(html);
    QString editorPlainText = editorDocument.toPlainText();
    editorPlainText.remove(QChar::ObjectReplacementCharacter);
    QVERIFY(editorPlainText.contains(QStringLiteral("Agenda")));
    QVERIFY(editorPlainText.contains(QStringLiteral("2026-05-18")));
    QVERIFY(editorPlainText.contains(QStringLiteral("Draft outline")));
    QVERIFY(editorPlainText.contains(QStringLiteral("Ship patch")));
    QVERIFY(!editorPlainText.contains(QStringLiteral("<task")));

    WhatSon::EditorComponent::AgendaDescriptor singleTaskDescriptor = descriptor;
    singleTaskDescriptor.tasks = {descriptor.tasks.constFirst()};
    const QString singleTaskHtml = WhatSon::EditorComponent::Agenda::renderHtml(singleTaskDescriptor);
    QTextDocument singleTaskEditorDocument;
    singleTaskEditorDocument.setHtml(singleTaskHtml);
    QVERIFY(editorDocument.size().height() > singleTaskEditorDocument.size().height());
}
