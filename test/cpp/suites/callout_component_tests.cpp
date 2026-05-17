#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QPainter>
#include <QTextDocument>

namespace
{
    bool colorNear(const QColor& actual, const QColor& expected, const int tolerance)
    {
        return std::abs(actual.red() - expected.red()) <= tolerance
            && std::abs(actual.green() - expected.green()) <= tolerance
            && std::abs(actual.blue() - expected.blue()) <= tolerance
            && actual.alpha() > 0;
    }

    int renderedPixelCountNear(const QString& html, const QColor& expected)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(WhatSon::EditorComponent::Callout::designWidth());
        editorDocument.setHtml(html);

        const QSize imageSize(
            WhatSon::EditorComponent::Callout::designWidth() + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        int count = 0;
        for (int y = 0; y < rendered.height(); ++y)
        {
            for (int x = 0; x < rendered.width(); ++x)
            {
                if (colorNear(rendered.pixelColor(x, y), expected, 3))
                {
                    ++count;
                }
            }
        }
        return count;
    }

    int renderedRightEdgePixelCountNear(const QString& html, const QColor& expected)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(WhatSon::EditorComponent::Callout::designWidth());
        editorDocument.setHtml(html);

        const QSize imageSize(
            WhatSon::EditorComponent::Callout::designWidth() + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        int count = 0;
        const int rightStripStart = qMax(0, WhatSon::EditorComponent::Callout::designWidth() - 24);
        for (int y = 0; y < rendered.height(); ++y)
        {
            for (int x = rightStripStart; x < WhatSon::EditorComponent::Callout::designWidth(); ++x)
            {
                if (colorNear(rendered.pixelColor(x, y), expected, 3))
                {
                    ++count;
                }
            }
        }
        return count;
    }

    int renderedPixelCountNearInRect(const QString& html, const QColor& expected, const QRect& rect)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(WhatSon::EditorComponent::Callout::designWidth());
        editorDocument.setHtml(html);

        const QSize imageSize(
            WhatSon::EditorComponent::Callout::designWidth() + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        int count = 0;
        const QRect boundedRect = rect.intersected(rendered.rect());
        for (int y = boundedRect.top(); y <= boundedRect.bottom(); ++y)
        {
            for (int x = boundedRect.left(); x <= boundedRect.right(); ++x)
            {
                if (colorNear(rendered.pixelColor(x, y), expected, 3))
                {
                    ++count;
                }
            }
        }
        return count;
    }

    int renderedDocumentHeight(const QString& html)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(WhatSon::EditorComponent::Callout::designWidth());
        editorDocument.setHtml(html);
        return static_cast<int>(std::ceil(editorDocument.size().height()));
    }

    int renderedLongestHorizontalRunNear(const QString& html, const QColor& expected)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(WhatSon::EditorComponent::Callout::designWidth());
        editorDocument.setHtml(html);

        const QSize imageSize(
            WhatSon::EditorComponent::Callout::designWidth() + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        int longestRun = 0;
        for (int y = 0; y < rendered.height(); ++y)
        {
            int currentRun = 0;
            for (int x = 0; x < rendered.width(); ++x)
            {
                if (colorNear(rendered.pixelColor(x, y), expected, 3))
                {
                    ++currentRun;
                    longestRun = qMax(longestRun, currentRun);
                }
                else
                {
                    currentRun = 0;
                }
            }
        }
        return longestRun;
    }

    QString plainTextWithoutCalloutChrome(QString plainText)
    {
        plainText.remove(QChar::ObjectReplacementCharacter);
        return plainText;
    }
} // namespace

void WhatSonCppRegressionTests::calloutComponent_rendersFigmaCalloutBlock()
{
    WhatSon::EditorComponent::CalloutDescriptor descriptor;
    descriptor.sourceText = QStringLiteral("<callout>This is callout text</callout>");
    descriptor.contentHtml = QStringLiteral("This is callout text");

    const QString html = WhatSon::EditorComponent::Callout::renderHtml(descriptor);

    QVERIFY(html.contains(QStringLiteral("<!--whatson-callout-source:")));
    QVERIFY(html.contains(QStringLiteral("<div class=\"whatson-callout\"")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(!html.contains(QStringLiteral("<span class=\"whatson-callout\"")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"280:7897\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-design-width=\"295\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-width-mode=\"fill\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-height-mode=\"hug-contents\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-padding-top=\"4\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-padding-right=\"4\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-padding-bottom=\"4\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-padding-left=\"4\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-width=\"3\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-height=\"14\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-radius=\"3\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-content-gap=\"12\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-frame-chrome-height=\"22\"")));
    QVERIFY(html.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(html.contains(QStringLiteral("height=\"22\"")));
    QVERIFY(!html.contains(QStringLiteral("<div class=\"whatson-callout\" height=\"")));
    QVERIFY(html.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(html.contains(QStringLiteral("padding:4px 4px")));
    QVERIFY(html.contains(QStringLiteral("height:auto")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(!html.contains(QStringLiteral("height:22px")));
    QVERIFY(!html.contains(QStringLiteral("<table")));
    QVERIFY(!html.contains(QStringLiteral("<td")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-callout-leading-bar\"")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-callout-content-gap\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-frame-chrome=\"true\"")));
    QVERIFY(!html.contains(QStringLiteral("<td width=\"3\"")));
    QVERIFY(!html.contains(QStringLiteral("<td width=\"12\"")));
    QVERIFY(!html.contains(QStringLiteral("&nbsp;")));
    QVERIFY(!html.contains(QStringLiteral("border-left:3px solid #d9d9d9")));
    QVERIFY(html.contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(html.contains(QStringLiteral("font-family:Pretendard")));
    QVERIFY(html.contains(QStringLiteral("font-size:12px")));
    QVERIFY(html.contains(QStringLiteral("line-height:12px")));
    QVERIFY(html.contains(QStringLiteral("This is callout text")));

    QTextDocument editorDocument;
    editorDocument.setHtml(html);
    QCOMPARE(
        plainTextWithoutCalloutChrome(editorDocument.toPlainText()).trimmed(),
        QStringLiteral("This is callout text"));
    QCOMPARE(editorDocument.toPlainText().split(QLatin1Char('\n'), Qt::KeepEmptyParts).size(), 1);
    QVERIFY(!editorDocument.toPlainText().contains(QChar::Nbsp));
    QVERIFY2(
        renderedDocumentHeight(html) >= 20,
        "The callout's HTML frame must render the requested 4px vertical padding as real row height.");
    QVERIFY2(
        renderedPixelCountNearInRect(html, QColor(QStringLiteral("#d9d9d9")), QRect(4, 3, 3, 18)) >= 30,
        "The callout's HTML frame must render the 3px leading bar after the 4px frame inset and 4px top padding.");
    QVERIFY2(
        renderedPixelCountNear(html, QColor(QStringLiteral("#262728"))) >= 100,
        "The callout HTML must render a surface without a QML overlay.");
    QVERIFY2(
        renderedRightEdgePixelCountNear(html, QColor(QStringLiteral("#262728"))) >= 48,
        "The callout surface must reach the editor frame's right edge instead of fitting only to text.");
    QVERIFY2(
        renderedLongestHorizontalRunNear(html, QColor(QStringLiteral("#262728")))
            >= WhatSon::EditorComponent::Callout::designWidth() - 1,
        "The callout surface must render as a full-width frame row.");
}
