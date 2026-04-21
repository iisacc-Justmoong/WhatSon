#pragma once

#include <QObject>
#include <QVariantList>

class ContentsDisplayGutterCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive WRITE setStructuredHostGeometryActive NOTIFY structuredHostGeometryActiveChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(double gutterViewportHeight READ gutterViewportHeight WRITE setGutterViewportHeight NOTIFY gutterViewportHeightChanged)

public:
    explicit ContentsDisplayGutterCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayGutterCoordinator() override;

    bool structuredHostGeometryActive() const noexcept;
    void setStructuredHostGeometryActive(bool value);

    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);

    double gutterViewportHeight() const noexcept;
    void setGutterViewportHeight(double value);

    Q_INVOKABLE QVariantList buildVisiblePlainGutterLineEntries(
        int firstVisibleLine,
        const QVariantList& gutterLineYs,
        const QVariantList& gutterLineHeights) const;

signals:
    void structuredHostGeometryActiveChanged();
    void logicalLineCountChanged();
    void gutterViewportHeightChanged();

private:
    bool m_structuredHostGeometryActive = false;
    int m_logicalLineCount = 1;
    double m_gutterViewportHeight = 0.0;
};
