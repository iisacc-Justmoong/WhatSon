#pragma once

#include "app/models/display/paper/ContentsPaperSelection.hpp"

#include <QColor>
#include <QObject>
#include <QSizeF>
#include <QString>

class ContentsA4PaperBackground : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ContentsPaperSelection::PaperKind paperKind READ paperKind CONSTANT)
    Q_PROPERTY(QString paperStandard READ paperStandard CONSTANT)
    Q_PROPERTY(qreal widthMillimeters READ widthMillimeters CONSTANT)
    Q_PROPERTY(qreal heightMillimeters READ heightMillimeters CONSTANT)
    Q_PROPERTY(QSizeF sizeMillimeters READ sizeMillimeters CONSTANT)
    Q_PROPERTY(qreal aspectRatio READ aspectRatio CONSTANT)

    Q_PROPERTY(QColor canvasColor READ canvasColor CONSTANT)
    Q_PROPERTY(QColor paperBorderColor READ paperBorderColor CONSTANT)
    Q_PROPERTY(QColor paperColor READ paperColor CONSTANT)
    Q_PROPERTY(QColor paperHighlightColor READ paperHighlightColor CONSTANT)
    Q_PROPERTY(QColor paperShadeColor READ paperShadeColor CONSTANT)
    Q_PROPERTY(QColor paperSeparatorColor READ paperSeparatorColor CONSTANT)
    Q_PROPERTY(QColor paperShadowColor READ paperShadowColor CONSTANT)
    Q_PROPERTY(QColor paperTextColor READ paperTextColor CONSTANT)

public:
    explicit ContentsA4PaperBackground(QObject* parent = nullptr);
    ~ContentsA4PaperBackground() override;

    static constexpr ContentsPaperSelection::PaperKind paperKindValue() noexcept
    {
        return ContentsPaperSelection::A4;
    }

    static constexpr qreal widthMillimetersValue() noexcept
    {
        return 210.0;
    }

    static constexpr qreal heightMillimetersValue() noexcept
    {
        return 297.0;
    }

    static constexpr qreal aspectRatioValue() noexcept
    {
        return widthMillimetersValue() / heightMillimetersValue();
    }

    static QString paperStandardValue();
    static QSizeF sizeMillimetersValue();
    static QColor canvasColorValue();
    static QColor paperBorderColorValue();
    static QColor paperColorValue();
    static QColor paperHighlightColorValue();
    static QColor paperShadeColorValue();
    static QColor paperSeparatorColorValue();
    static QColor paperShadowColorValue();
    static QColor paperTextColorValue();

    ContentsPaperSelection::PaperKind paperKind() const noexcept;
    QString paperStandard() const;
    qreal widthMillimeters() const noexcept;
    qreal heightMillimeters() const noexcept;
    QSizeF sizeMillimeters() const;
    qreal aspectRatio() const noexcept;

    QColor canvasColor() const;
    QColor paperBorderColor() const;
    QColor paperColor() const;
    QColor paperHighlightColor() const;
    QColor paperShadeColor() const;
    QColor paperSeparatorColor() const;
    QColor paperShadowColor() const;
    QColor paperTextColor() const;

public slots:
    void requestRefresh();

signals:
    void backgroundChanged();
};
