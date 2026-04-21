#include "app/models/display/paper/ContentsA4PaperBackground.hpp"

ContentsA4PaperBackground::ContentsA4PaperBackground(QObject* parent)
    : QObject(parent)
{
}

ContentsA4PaperBackground::~ContentsA4PaperBackground() = default;

QString ContentsA4PaperBackground::paperStandardValue()
{
    return ContentsPaperSelection::paperStandardForKind(paperKindValue());
}

QSizeF ContentsA4PaperBackground::sizeMillimetersValue()
{
    return QSizeF(widthMillimetersValue(), heightMillimetersValue());
}

QColor ContentsA4PaperBackground::canvasColorValue()
{
    return QColor(QStringLiteral("#F1F3F6"));
}

QColor ContentsA4PaperBackground::paperBorderColorValue()
{
    return QColor(QStringLiteral("#19000000"));
}

QColor ContentsA4PaperBackground::paperColorValue()
{
    return QColor(QStringLiteral("#FFFCF5"));
}

QColor ContentsA4PaperBackground::paperHighlightColorValue()
{
    return QColor(QStringLiteral("#FFFDF9"));
}

QColor ContentsA4PaperBackground::paperShadeColorValue()
{
    return QColor(QStringLiteral("#F6EEE0"));
}

QColor ContentsA4PaperBackground::paperSeparatorColorValue()
{
    return QColor(QStringLiteral("#24000000"));
}

QColor ContentsA4PaperBackground::paperShadowColorValue()
{
    return QColor(QStringLiteral("#14000000"));
}

QColor ContentsA4PaperBackground::paperTextColorValue()
{
    return QColor(QStringLiteral("#000000"));
}

ContentsPaperSelection::PaperKind ContentsA4PaperBackground::paperKind() const noexcept
{
    return paperKindValue();
}

QString ContentsA4PaperBackground::paperStandard() const
{
    return paperStandardValue();
}

qreal ContentsA4PaperBackground::widthMillimeters() const noexcept
{
    return widthMillimetersValue();
}

qreal ContentsA4PaperBackground::heightMillimeters() const noexcept
{
    return heightMillimetersValue();
}

QSizeF ContentsA4PaperBackground::sizeMillimeters() const
{
    return sizeMillimetersValue();
}

qreal ContentsA4PaperBackground::aspectRatio() const noexcept
{
    return aspectRatioValue();
}

QColor ContentsA4PaperBackground::canvasColor() const
{
    return canvasColorValue();
}

QColor ContentsA4PaperBackground::paperBorderColor() const
{
    return paperBorderColorValue();
}

QColor ContentsA4PaperBackground::paperColor() const
{
    return paperColorValue();
}

QColor ContentsA4PaperBackground::paperHighlightColor() const
{
    return paperHighlightColorValue();
}

QColor ContentsA4PaperBackground::paperShadeColor() const
{
    return paperShadeColorValue();
}

QColor ContentsA4PaperBackground::paperSeparatorColor() const
{
    return paperSeparatorColorValue();
}

QColor ContentsA4PaperBackground::paperShadowColor() const
{
    return paperShadowColorValue();
}

QColor ContentsA4PaperBackground::paperTextColor() const
{
    return paperTextColorValue();
}

void ContentsA4PaperBackground::requestRefresh()
{
    emit backgroundChanged();
}
