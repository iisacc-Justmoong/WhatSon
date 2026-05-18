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

    int renderedPixelCountNear(
        const QString& html,
        const QColor& expected,
        const int documentWidth = WhatSon::EditorComponent::Callout::designWidth())
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(documentWidth);
        editorDocument.setHtml(html);

        const QSize imageSize(
            documentWidth + 16,
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

    int renderedPixelCountNearInRect(
        const QString& html,
        const QColor& expected,
        const QRect& rect,
        const int documentWidth = WhatSon::EditorComponent::Callout::designWidth())
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(documentWidth);
        editorDocument.setHtml(html);

        const QSize imageSize(
            documentWidth + 16,
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

    QRect renderedColorBoundsNearInRect(
        const QString& html,
        const QColor& expected,
        const QRect& rect,
        const int documentWidth = WhatSon::EditorComponent::Callout::designWidth(),
        const int tolerance = 3)
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(documentWidth);
        editorDocument.setHtml(html);

        const QSize imageSize(
            documentWidth + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        QRect bounds;
        const QRect boundedRect = rect.intersected(rendered.rect());
        for (int y = boundedRect.top(); y <= boundedRect.bottom(); ++y)
        {
            for (int x = boundedRect.left(); x <= boundedRect.right(); ++x)
            {
                if (!colorNear(rendered.pixelColor(x, y), expected, tolerance))
                {
                    continue;
                }

                const QPoint point(x, y);
                bounds = bounds.isValid() ? bounds.united(QRect(point, QSize(1, 1))) : QRect(point, QSize(1, 1));
            }
        }
        return bounds;
    }

    int renderedDocumentHeight(
        const QString& html,
        const int documentWidth = WhatSon::EditorComponent::Callout::designWidth())
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(documentWidth);
        editorDocument.setHtml(html);
        return static_cast<int>(std::ceil(editorDocument.size().height()));
    }

    int renderedLongestVerticalRunNearInRect(
        const QString& html,
        const QColor& expected,
        const QRect& rect,
        const int documentWidth = WhatSon::EditorComponent::Callout::designWidth())
    {
        QTextDocument editorDocument;
        editorDocument.setDocumentMargin(0);
        editorDocument.setTextWidth(documentWidth);
        editorDocument.setHtml(html);

        const QSize imageSize(
            documentWidth + 16,
            qMax(48, static_cast<int>(std::ceil(editorDocument.size().height())) + 16));
        QImage rendered(imageSize, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);

        QPainter painter(&rendered);
        editorDocument.drawContents(&painter);
        painter.end();

        int longestRun = 0;
        const QRect boundedRect = rect.intersected(rendered.rect());
        for (int x = boundedRect.left(); x <= boundedRect.right(); ++x)
        {
            int currentRun = 0;
            for (int y = boundedRect.top(); y <= boundedRect.bottom(); ++y)
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
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-design-height=\"14\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-height-mode=\"match-content\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-bar-radius=\"3\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-content-gap=\"12\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-frame-min-height=\"22\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-frame-chrome-height=\"")));
    QVERIFY(html.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(!html.contains(QStringLiteral("<div class=\"whatson-callout\" height=\"")));
    QVERIFY(html.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(html.contains(QStringLiteral("padding:4px 4px")));
    QVERIFY(html.contains(QStringLiteral("height:auto")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(!html.contains(QStringLiteral("height:22px")));
    QVERIFY(!html.contains(QStringLiteral("<table")));
    QVERIFY(!html.contains(QStringLiteral("<td")));
    QVERIFY(!html.contains(QStringLiteral("<td width=\"3\"")));
    QVERIFY(html.contains(QStringLiteral("<img class=\"whatson-callout-leading-bar\"")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-callout-leading-bar\"")));
    QVERIFY(!html.contains(QStringLiteral("class=\"whatson-callout-content-gap\"")));
    QVERIFY(html.contains(QStringLiteral("data-callout-frame-chrome=\"true\"")));
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
    QVERIFY(editorDocument.toPlainText().contains(QChar::ObjectReplacementCharacter));
    QVERIFY(!editorDocument.toPlainText().contains(QChar::Nbsp));
    QVERIFY2(
        renderedDocumentHeight(html) >= 14,
        "The callout's HTML frame must render the callout row without collapsing below the Figma bar height.");
    QVERIFY2(
        renderedPixelCountNearInRect(html, QColor(QStringLiteral("#d9d9d9")), QRect(0, 0, 16, 24)) >= 30,
        "The callout's HTML frame must render the 3px leading bar as frame chrome.");
    const int singleLineDocumentHeight = renderedDocumentHeight(html);
    const QRect barBounds = renderedColorBoundsNearInRect(
        html,
        QColor(QStringLiteral("#d9d9d9")),
        QRect(0, 0, 16, singleLineDocumentHeight));
    const QRect textBounds = renderedColorBoundsNearInRect(
        html,
        QColor(QStringLiteral("#ffffff")),
        QRect(8, 0, WhatSon::EditorComponent::Callout::designWidth() - 8, singleLineDocumentHeight),
        WhatSon::EditorComponent::Callout::designWidth(),
        64);
    QVERIFY2(barBounds.isValid(), "The callout leading bar must paint into the rendered frame.");
    QVERIFY2(textBounds.isValid(), "The callout text must paint into the rendered frame.");
    QVERIFY2(
        qAbs(barBounds.center().y() - textBounds.center().y()) <= 2,
        qPrintable(QStringLiteral("The callout leading bar must stay vertically aligned with the text. bar=%1,%2 text=%3,%4")
            .arg(barBounds.top())
            .arg(barBounds.bottom())
            .arg(textBounds.top())
            .arg(textBounds.bottom())));
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

    WhatSon::EditorComponent::CalloutDescriptor wrappedDescriptor;
    wrappedDescriptor.sourceText = QStringLiteral("<callout>Wrapped callout text</callout>");
    wrappedDescriptor.contentHtml = QStringLiteral(
        "Wrapped callout text should continue on the same visual row while the leading bar grows "
        "with every wrapped line instead of staying as a fixed icon before the text. "
        "A second sentence keeps this fixture long enough to wrap at the callout design width.");
    constexpr int kNarrowDocumentWidth = 142;
    wrappedDescriptor.editorViewportWidth = kNarrowDocumentWidth;
    const QString wrappedHtml = WhatSon::EditorComponent::Callout::renderHtml(wrappedDescriptor);
    const int wrappedDocumentHeight = renderedDocumentHeight(wrappedHtml, kNarrowDocumentWidth);
    QVERIFY2(
        wrappedDocumentHeight > 34,
        "The narrow callout fixture must wrap across multiple visual lines.");
    const int wrappedBarRun = renderedLongestVerticalRunNearInRect(
        wrappedHtml,
        QColor(QStringLiteral("#d9d9d9")),
        QRect(0, 0, 16, wrappedDocumentHeight),
        kNarrowDocumentWidth);
    QVERIFY2(
        wrappedBarRun >= wrappedDocumentHeight - 8,
        qPrintable(QStringLiteral("The leading bar must stretch with the wrapped callout content height. run=%1 height=%2")
            .arg(wrappedBarRun)
            .arg(wrappedDocumentHeight)));
}

void WhatSonCppRegressionTests::calloutComponent_plansBoundaryEditsAgainstDecoratedCursor()
{
    const auto sourceToVisible =
        [](const int sourcePosition) -> int
        {
            if (sourcePosition <= 5)
            {
                return sourcePosition;
            }
            if (sourcePosition <= 14)
            {
                return 5;
            }
            if (sourcePosition <= 20)
            {
                return 5 + (sourcePosition - 14);
            }
            if (sourcePosition <= 30)
            {
                return 11;
            }
            return sourcePosition - 19;
        };

    const QString sourceText = QStringLiteral("Alpha<callout>Inside</callout> Beta");
    const QVector<WhatSon::EditorComponent::CalloutSourceRange> ranges =
        WhatSon::EditorComponent::Callout::sourceRanges(sourceText);
    QCOMPARE(ranges.size(), 1);
    QCOMPARE(ranges.first().openingStart, 5);
    QCOMPARE(ranges.first().contentStart, 14);
    QCOMPARE(ranges.first().contentEnd, 20);
    QVERIFY(ranges.first().isValid());

    const std::optional<WhatSon::EditorComponent::CalloutBoundaryEdit> backspaceEdit =
        WhatSon::EditorComponent::Callout::backspaceAtVisibleContentStart(
            sourceText,
            5,
            sourceToVisible);
    QVERIFY(backspaceEdit.has_value());
    QCOMPARE(backspaceEdit->bodySourceText, QStringLiteral("AlphaInside Beta"));
    QCOMPARE(backspaceEdit->sourceCursorPosition, 5);
    QVERIFY(backspaceEdit->changed);

    const std::optional<WhatSon::EditorComponent::CalloutBoundaryEdit> ignoredBackspace =
        WhatSon::EditorComponent::Callout::backspaceAtVisibleContentStart(
            sourceText,
            6,
            sourceToVisible);
    QVERIFY(!ignoredBackspace.has_value());
    QVERIFY(!WhatSon::EditorComponent::Callout::backspaceAtVisibleContentStart(
        sourceText,
        5,
        WhatSon::EditorComponent::Callout::SourceToVisibleCursor()).has_value());

    const QString emptySourceText = QStringLiteral("Alpha<callout></callout> Beta");
    const auto emptySourceToVisible =
        [](const int sourcePosition) -> int
        {
            if (sourcePosition <= 5)
            {
                return sourcePosition;
            }
            if (sourcePosition <= 24)
            {
                return 5;
            }
            return sourcePosition - 19;
        };
    const std::optional<WhatSon::EditorComponent::CalloutBoundaryEdit> emptyBackspaceEdit =
        WhatSon::EditorComponent::Callout::backspaceAtVisibleContentStart(
            emptySourceText,
            5,
            emptySourceToVisible);
    QVERIFY(emptyBackspaceEdit.has_value());
    QCOMPARE(emptyBackspaceEdit->bodySourceText, QStringLiteral("Alpha Beta"));
    QCOMPARE(emptyBackspaceEdit->sourceCursorPosition, 5);

    WhatSon::EditorComponent::CalloutDescriptor descriptor;
    descriptor.sourceText = QStringLiteral("<callout>Inside</callout>");
    descriptor.contentHtml = QStringLiteral("Inside");
    const QString editorHtml = WhatSon::EditorComponent::Callout::renderHtml(descriptor);
    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    const QString editorPlainText = editorDocument.toPlainText();
    const int decoratedContentStart = editorPlainText.indexOf(QStringLiteral("Inside"));
    QVERIFY(decoratedContentStart > 0);
    QCOMPARE(
        WhatSon::EditorComponent::Callout::sourceVisibleCursorForDecoratedCursor(
            editorHtml,
            QStringLiteral("Inside"),
            decoratedContentStart),
        0);
    QCOMPARE(
        WhatSon::EditorComponent::Callout::decoratedContentStartForVisibleCursor(
            editorHtml,
            QStringLiteral("Inside"),
            0),
        decoratedContentStart);

    const std::optional<WhatSon::EditorComponent::CalloutBoundaryEdit> chromeEnterEdit =
        WhatSon::EditorComponent::Callout::enterBeforeContentChrome(
            QStringLiteral("<callout>Inside</callout>"),
            editorHtml,
            QStringLiteral("Inside"),
            0,
            [](const int sourcePosition) -> int
            {
                if (sourcePosition <= 9)
                {
                    return 0;
                }
                if (sourcePosition <= 15)
                {
                    return sourcePosition - 9;
                }
                return 6;
            });
    QVERIFY(chromeEnterEdit.has_value());
    QCOMPARE(chromeEnterEdit->bodySourceText, QStringLiteral("\n<callout>Inside</callout>"));
    QCOMPARE(chromeEnterEdit->sourceCursorPosition, 1);
    QVERIFY(!WhatSon::EditorComponent::Callout::enterBeforeContentChrome(
        QStringLiteral("<callout>Inside</callout>"),
        editorHtml,
        QStringLiteral("Inside"),
        0,
        WhatSon::EditorComponent::Callout::SourceToVisibleCursor()).has_value());

    const std::optional<WhatSon::EditorComponent::CalloutBoundaryEdit> enterEdit =
        WhatSon::EditorComponent::Callout::enterInsideVisibleCursor(
            QStringLiteral("<callout>Inside</callout>"),
            QStringLiteral("Inside").size(),
            [](const int sourcePosition) -> int
            {
                if (sourcePosition <= 9)
                {
                    return 0;
                }
                if (sourcePosition <= 15)
                {
                    return sourcePosition - 9;
                }
                return 6;
            });
    QVERIFY(enterEdit.has_value());
    QCOMPARE(enterEdit->bodySourceText, QStringLiteral("<callout>Inside</callout>\n"));
    QCOMPARE(enterEdit->sourceCursorPosition, QStringLiteral("<callout>Inside</callout>\n").size());
    QVERIFY(!WhatSon::EditorComponent::Callout::enterInsideVisibleCursor(
        QStringLiteral("<callout>Inside</callout>"),
        0,
        WhatSon::EditorComponent::Callout::SourceToVisibleCursor()).has_value());
}
