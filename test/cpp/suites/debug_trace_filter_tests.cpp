#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

void WhatSonCppRegressionTests::debugTraceFilter_suppressesIiXmlDebugSpamByDefault()
{
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));
    const QString debugTraceHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/WhatSonDebugTrace.hpp"));

    QVERIFY(WhatSon::Debug::shouldSuppressThirdPartyTraceMessage(
        QtDebugMsg,
        QStringLiteral("iiXml::Parser::TagParser::ParseAllDocumentResult begin"),
        false));
    QVERIFY(!WhatSon::Debug::shouldSuppressThirdPartyTraceMessage(
        QtDebugMsg,
        QStringLiteral("iiXml::Parser::TagParser::ParseAllDocumentResult begin"),
        true));
    QVERIFY(!WhatSon::Debug::shouldSuppressThirdPartyTraceMessage(
        QtWarningMsg,
        QStringLiteral("iiXml::Parser::TagParser::ParseAllDocumentResult failed"),
        false));
    QVERIFY(!WhatSon::Debug::shouldSuppressThirdPartyTraceMessage(
        QtDebugMsg,
        QStringLiteral("[whatson:debug][main][startup]"),
        false));

    QVERIFY(mainSource.contains(QStringLiteral("WhatSon::Debug::installThirdPartyTraceMessageFilter();")));
    QVERIFY(debugTraceHeader.contains(QStringLiteral("WHATSON_IIXML_TRACE_MODE")));
    QVERIFY(debugTraceHeader.contains(QStringLiteral("qInstallMessageHandler(filteredThirdPartyTraceMessageHandler)")));
}

void WhatSonCppRegressionTests::debugTrace_summarizesLargeTextFromPreviewOnly()
{
    const QString debugTraceHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/WhatSonDebugTrace.hpp"));
    const QString largeText = QStringLiteral("Alpha\n") + QString(128 * 1024, QLatin1Char('x'));
    const QString summary = WhatSon::Debug::summarizeText(largeText, 8);

    QVERIFY(summary.startsWith(QStringLiteral("len=%1 ").arg(largeText.size())));
    QVERIFY(summary.contains(QStringLiteral("preview=\"Alpha\\nxx\"")));
    QVERIFY(summary.endsWith(QStringLiteral("...(truncated)")));
    QVERIFY(!summary.contains(QString(256, QLatin1Char('x'))));
    QVERIFY(debugTraceHeader.contains(QStringLiteral("text.left(safePreviewLength)")));
    QVERIFY(!debugTraceHeader.contains(QStringLiteral("const QString normalized = text.normalized")));
}
