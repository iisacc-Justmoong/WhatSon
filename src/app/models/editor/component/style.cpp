#include "app/models/editor/component/style.h"

#include <QColor>
#include <QFont>
#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <cmath>

namespace
{
    QString decodeXmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString escapeHtmlAttribute(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    QString sanitizedCssValue(QString value)
    {
        value = value.trimmed();
        value.remove(QLatin1Char(';'));
        value.remove(QLatin1Char('<'));
        value.remove(QLatin1Char('>'));
        value.remove(QLatin1Char('{'));
        value.remove(QLatin1Char('}'));
        return value.simplified();
    }

    QString integerCssValue(const QString& value)
    {
        bool ok = false;
        const int number = value.trimmed().toInt(&ok);
        return ok && number > 0 ? QString::number(number) : QString();
    }

    QString floatingCssValue(const QString& value)
    {
        bool ok = false;
        const double number = value.trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(number) || number <= 0.0)
        {
            return {};
        }
        return QString::number(number, 'g', 16);
    }

    QColor colorFromCssValue(QString value)
    {
        value = value.trimmed();
        const QColor namedColor(value);
        if (namedColor.isValid())
        {
            return namedColor;
        }

        static const QRegularExpression rgbaPattern(
            QStringLiteral(
                R"(^rgba?\s*\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})(?:\s*,\s*([0-9]*\.?[0-9]+))?\s*\)$)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = rgbaPattern.match(value);
        if (!match.hasMatch())
        {
            return {};
        }

        const int red = std::clamp(match.captured(1).toInt(), 0, 255);
        const int green = std::clamp(match.captured(2).toInt(), 0, 255);
        const int blue = std::clamp(match.captured(3).toInt(), 0, 255);
        QColor color(red, green, blue);
        if (match.capturedStart(4) >= 0)
        {
            bool ok = false;
            const double alpha = match.captured(4).toDouble(&ok);
            if (ok)
            {
                color.setAlphaF(std::clamp(alpha, 0.0, 1.0));
            }
        }
        return color;
    }

    QString normalizedColorName(const QString& value)
    {
        const QColor color = colorFromCssValue(value);
        if (!color.isValid())
        {
            return value.trimmed().toCaseFolded();
        }
        return color.alpha() < 255
            ? color.name(QColor::HexArgb).toCaseFolded()
            : color.name(QColor::HexRgb).toCaseFolded();
    }

    QString normalizedCssIdentifier(QString value)
    {
        value = value.trimmed();
        if ((value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))
            || (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))))
        {
            value = value.mid(1, value.size() - 2);
        }
        return value.simplified().toCaseFolded();
    }

    QString cssDeclarationValue(const QString& declaration, const QString& propertyName)
    {
        QString resolvedValue;
        const QStringList parts = declaration.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const QString& part : parts)
        {
            const int separator = part.indexOf(QLatin1Char(':'));
            if (separator < 0)
            {
                continue;
            }

            const QString name = part.left(separator).trimmed();
            if (name.compare(propertyName, Qt::CaseInsensitive) == 0)
            {
                resolvedValue = decodeXmlEntities(part.mid(separator + 1).trimmed());
            }
        }
        return resolvedValue;
    }

    double cssNumericValue(QString value, bool* ok)
    {
        value = value.trimmed().toCaseFolded();
        static const QRegularExpression numericPattern(
            QStringLiteral(R"(^\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+))\s*(?:px|pt|em|rem|%)?\s*$)"));
        const QRegularExpressionMatch match = numericPattern.match(value);
        if (!match.hasMatch())
        {
            if (ok != nullptr)
            {
                *ok = false;
            }
            return 0.0;
        }
        return match.captured(1).toDouble(ok);
    }

    bool cssNumericValuesMatch(const QString& left, const QString& right)
    {
        bool leftOk = false;
        bool rightOk = false;
        const double leftValue = cssNumericValue(left, &leftOk);
        const double rightValue = cssNumericValue(right, &rightOk);
        return leftOk && rightOk && std::abs(leftValue - rightValue) < 0.001;
    }
} // namespace

namespace WhatSon::EditorComponent
{
    QString Style::canonicalName()
    {
        return QStringLiteral("style");
    }

    QString Style::openingToken()
    {
        return QStringLiteral("<style>");
    }

    QString Style::closingToken()
    {
        return QStringLiteral("</style>");
    }

    QString Style::defaultEditorFontFamily()
    {
        return QStringLiteral("Pretendard");
    }

    StyleToken Style::lvrsTextStyleTokenFromName(QString tokenName)
    {
        tokenName = tokenName.trimmed().toCaseFolded();
        tokenName.remove(QLatin1Char('-'));
        tokenName.remove(QLatin1Char('_'));
        tokenName.remove(QLatin1Char(' '));

        if (tokenName == QStringLiteral("title"))
        {
            return {true, QStringLiteral("title"), 26, QFont::Bold, QStringLiteral("Bold"), 26, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("title2"))
        {
            return {true, QStringLiteral("title2"), 22, QFont::Bold, QStringLiteral("Bold"), 22, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("header"))
        {
            return {true, QStringLiteral("header"), 17, QFont::DemiBold, QStringLiteral("SemiBold"), 17, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("header2"))
        {
            return {true, QStringLiteral("header2"), 15, QFont::DemiBold, QStringLiteral("SemiBold"), 15, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("body"))
        {
            return {true, QStringLiteral("body"), 12, QFont::Medium, QStringLiteral("Medium"), 12, QStringLiteral("#CCFFFFFF")};
        }
        if (tokenName == QStringLiteral("description"))
        {
            return {true, QStringLiteral("description"), 12, QFont::DemiBold, QStringLiteral("SemiBold"), 12, QStringLiteral("#99FFFFFF")};
        }
        if (tokenName == QStringLiteral("caption"))
        {
            return {true, QStringLiteral("caption"), 11, QFont::Normal, QStringLiteral("Regular"), 11, QStringLiteral("#80FFFFFF")};
        }
        if (tokenName == QStringLiteral("disabled"))
        {
            return {true, QStringLiteral("disabled"), 11, QFont::Normal, QStringLiteral("Regular"), 11, QStringLiteral("#4DFFFFFF")};
        }
        return {};
    }

    QString Style::bodyEditorCssDeclaration()
    {
        const StyleToken bodyToken = lvrsTextStyleTokenFromName(QStringLiteral("body"));
        return QStringLiteral("font-family:%1;font-size:%2px;font-weight:%3;line-height:%4px;color:%5;")
            .arg(
                defaultEditorFontFamily(),
                QString::number(bodyToken.pixelSize),
                QString::number(bodyToken.weight),
                QString::number(bodyToken.lineHeight),
                bodyToken.color);
    }

    QString Style::attributeValueFromRawToken(const QString& rawTagText, const QString& attributeName)
    {
        const QRegularExpression attributePattern(
            QStringLiteral("\\b%1\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>/]+))")
                .arg(QRegularExpression::escape(attributeName)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawTagText);
        if (!match.hasMatch())
        {
            return {};
        }

        for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
        {
            if (match.capturedStart(captureIndex) >= 0)
            {
                return decodeXmlEntities(match.captured(captureIndex)).trimmed();
            }
        }
        return {};
    }

    QString Style::cssDeclarationFromRawToken(const QString& rawTagText)
    {
        QStringList declarations;
        const StyleToken styleToken =
            lvrsTextStyleTokenFromName(attributeValueFromRawToken(rawTagText, QStringLiteral("style")));
        if (styleToken.valid)
        {
            declarations.push_back(QStringLiteral("font-family:%1;").arg(defaultEditorFontFamily()));
            declarations.push_back(QStringLiteral("font-size:%1px;").arg(styleToken.pixelSize));
            declarations.push_back(QStringLiteral("font-weight:%1;").arg(styleToken.weight));
            declarations.push_back(QStringLiteral("line-height:%1px;").arg(styleToken.lineHeight));
            declarations.push_back(QStringLiteral("color:%1;").arg(styleToken.color));
        }

        const auto appendStringDeclaration =
            [&](const QString& attributeName, const QString& cssPropertyName)
        {
            const QString value = sanitizedCssValue(attributeValueFromRawToken(rawTagText, attributeName));
            if (!value.isEmpty())
            {
                declarations.push_back(QStringLiteral("%1:%2;").arg(cssPropertyName, value));
            }
        };

        appendStringDeclaration(QStringLiteral("font"), QStringLiteral("font-family"));
        appendStringDeclaration(QStringLiteral("weight"), QStringLiteral("font-weight"));

        const QString size = integerCssValue(attributeValueFromRawToken(rawTagText, QStringLiteral("size")));
        if (!size.isEmpty())
        {
            declarations.push_back(QStringLiteral("font-size:%1px;").arg(size));
        }

        appendStringDeclaration(QStringLiteral("color"), QStringLiteral("color"));
        appendStringDeclaration(QStringLiteral("background"), QStringLiteral("background-color"));
        appendStringDeclaration(QStringLiteral("align"), QStringLiteral("text-align"));

        const QString height = floatingCssValue(attributeValueFromRawToken(rawTagText, QStringLiteral("height")));
        if (!height.isEmpty())
        {
            declarations.push_back(QStringLiteral("line-height:%1;").arg(height));
        }

        return declarations.join(QString());
    }

    QString Style::openingHtmlFromRawToken(const QString& rawTagText)
    {
        const QString encodedSource = QString::fromLatin1(rawTagText.toUtf8().toHex());
        const QString marker = QStringLiteral("<!--whatson-style-source:%1-->"
                                              "<a name=\"whatson-style-source:%1\"></a>")
            .arg(encodedSource);
        const QString cssDeclaration = cssDeclarationFromRawToken(rawTagText);
        if (cssDeclaration.isEmpty())
        {
            return marker + QStringLiteral("<span>");
        }
        return marker
            + QStringLiteral("<span style=\"%1\">").arg(escapeHtmlAttribute(cssDeclaration));
    }

    QString Style::closingHtml()
    {
        return QStringLiteral("</span><a name=\"whatson-style-source-end\"></a><!--/whatson-style-source-->");
    }

    QString Style::markerContentHtml(QString markerBody)
    {
        static const QRegularExpression startAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source:[0-9a-fA-F]*["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression endAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source-end["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        markerBody.remove(startAnchorPattern);
        markerBody.remove(endAnchorPattern);
        markerBody.remove(QChar(0x200B));
        return markerBody;
    }

    int Style::weightValue(const QString& value)
    {
        bool ok = false;
        const int numericWeight = value.trimmed().toInt(&ok);
        if (ok)
        {
            return numericWeight;
        }

        const QString normalized = value.trimmed().toCaseFolded();
        if (normalized == QStringLiteral("bold"))
        {
            return QFont::Black;
        }
        if (normalized == QStringLiteral("normal") || normalized.isEmpty())
        {
            return QFont::Normal;
        }
        return QFont::Normal;
    }

    bool Style::spanMatchesOpeningToken(const QString& spanOpeningTag, const QString& openingToken)
    {
        const QString cssDeclaration =
            attributeValueFromRawToken(spanOpeningTag, QStringLiteral("style"));
        if (cssDeclaration.isEmpty())
        {
            return false;
        }

        const QString expectedDeclaration = cssDeclarationFromRawToken(openingToken);
        if (expectedDeclaration.isEmpty())
        {
            return false;
        }

        int matchedProperties = 0;
        const auto matchIdentifier =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            return normalizedCssIdentifier(actual) == normalizedCssIdentifier(expected);
        };
        const auto matchColor =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            const QColor actualColor = colorFromCssValue(actual);
            const QColor expectedColor = colorFromCssValue(expected);
            if (actualColor.isValid() && expectedColor.isValid())
            {
                return actualColor.rgb() == expectedColor.rgb()
                    && (actualColor.alpha() == expectedColor.alpha()
                        || actualColor.alpha() == 255
                        || expectedColor.alpha() == 255);
            }
            return normalizedColorName(actual) == normalizedColorName(expected);
        };
        const auto matchNumber =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            return cssNumericValuesMatch(actual, expected);
        };
        const auto matchWeight =
            [&](bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, QStringLiteral("font-weight"));
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actualWeight = cssDeclarationValue(cssDeclaration, QStringLiteral("font-weight"));
            if (actualWeight.isEmpty())
            {
                return !requiredWhenMissing || weightValue(expected) == QFont::Normal;
            }

            ++matchedProperties;
            return weightValue(actualWeight) >= weightValue(expected);
        };

        return matchIdentifier(QStringLiteral("font-family"), true)
            && matchNumber(QStringLiteral("font-size"), true)
            && matchWeight(true)
            && matchColor(QStringLiteral("color"), true)
            && matchColor(QStringLiteral("background-color"), true)
            && matchIdentifier(QStringLiteral("text-align"), false)
            && matchNumber(QStringLiteral("line-height"), false)
            && matchedProperties > 0;
    }

    StyleSourceBaseline Style::sourceBaselineFromOpeningToken(const QString& openingToken)
    {
        const QString styleValue = attributeValueFromRawToken(openingToken, QStringLiteral("style"));
        const StyleToken styleToken = lvrsTextStyleTokenFromName(styleValue);
        const QString explicitWeight = attributeValueFromRawToken(openingToken, QStringLiteral("weight"));
        return {
            false,
            explicitWeight.isEmpty()
                ? (styleToken.valid ? styleToken.weight : QFont::Normal)
                : weightValue(explicitWeight),
            normalizedColorName(attributeValueFromRawToken(openingToken, QStringLiteral("background")))
        };
    }
} // namespace WhatSon::EditorComponent
