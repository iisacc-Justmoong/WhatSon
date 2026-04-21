#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::a4PaperBackground_exposesCanonicalMetricsAndAnchorsPrintRendererDefaults()
{
    ContentsA4PaperBackground background;
    ContentsPagePrintLayoutRenderer layoutRenderer;

    QCOMPARE(background.paperKind(), ContentsPaperSelection::A4);
    QCOMPARE(background.paperStandard(), QStringLiteral("A4"));
    QCOMPARE(background.widthMillimeters(), 210.0);
    QCOMPARE(background.heightMillimeters(), 297.0);
    QCOMPARE(background.sizeMillimeters(), QSizeF(210.0, 297.0));
    QVERIFY(std::abs(background.aspectRatio() - (210.0 / 297.0)) < 0.0001);

    QCOMPARE(background.canvasColor(), QColor(QStringLiteral("#F1F3F6")));
    QCOMPARE(background.paperBorderColor(), QColor(QStringLiteral("#19000000")));
    QCOMPARE(background.paperColor(), QColor(QStringLiteral("#FFFCF5")));
    QCOMPARE(background.paperHighlightColor(), QColor(QStringLiteral("#FFFDF9")));
    QCOMPARE(background.paperShadeColor(), QColor(QStringLiteral("#F6EEE0")));
    QCOMPARE(background.paperSeparatorColor(), QColor(QStringLiteral("#24000000")));
    QCOMPARE(background.paperShadowColor(), QColor(QStringLiteral("#14000000")));
    QCOMPARE(background.paperTextColor(), QColor(QStringLiteral("#000000")));

    layoutRenderer.setPaperAspectRatio(0.0);
    QCOMPARE(layoutRenderer.paperAspectRatio(), background.aspectRatio());
    QCOMPARE(layoutRenderer.canvasColor(), background.canvasColor());
    QCOMPARE(layoutRenderer.paperBorderColor(), background.paperBorderColor());
    QCOMPARE(layoutRenderer.paperColor(), background.paperColor());
    QCOMPARE(layoutRenderer.paperHighlightColor(), background.paperHighlightColor());
    QCOMPARE(layoutRenderer.paperShadeColor(), background.paperShadeColor());
    QCOMPARE(layoutRenderer.paperSeparatorColor(), background.paperSeparatorColor());
    QCOMPARE(layoutRenderer.paperShadowColor(), background.paperShadowColor());
    QCOMPARE(layoutRenderer.paperTextColor(), background.paperTextColor());
}
