#pragma once

#include <QObject>

class ContentsMinimapLayoutMetrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int gapNone READ gapNone WRITE setGapNone NOTIFY gapNoneChanged)
    Q_PROPERTY(int gap8 READ gap8 WRITE setGap8 NOTIFY gap8Changed)
    Q_PROPERTY(int gap12 READ gap12 WRITE setGap12 NOTIFY gap12Changed)
    Q_PROPERTY(int gap20 READ gap20 WRITE setGap20 NOTIFY gap20Changed)
    Q_PROPERTY(int gap24 READ gap24 WRITE setGap24 NOTIFY gap24Changed)
    Q_PROPERTY(int strokeThin READ strokeThin WRITE setStrokeThin NOTIFY strokeThinChanged)
    Q_PROPERTY(int buttonMinWidth READ buttonMinWidth WRITE setButtonMinWidth NOTIFY buttonMinWidthChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(bool minimapVisible READ minimapVisible WRITE setMinimapVisible NOTIFY minimapVisibleChanged)
    Q_PROPERTY(int defaultMinimapWidth READ defaultMinimapWidth NOTIFY metricsChanged)
    Q_PROPERTY(int designRowCount READ designRowCount NOTIFY metricsChanged)
    Q_PROPERTY(int effectiveMinimapWidth READ effectiveMinimapWidth NOTIFY metricsChanged)
    Q_PROPERTY(int effectiveRowCount READ effectiveRowCount NOTIFY metricsChanged)

public:
    explicit ContentsMinimapLayoutMetrics(QObject* parent = nullptr);
    ~ContentsMinimapLayoutMetrics() override;

    int gapNone() const noexcept;
    void setGapNone(int value);
    int gap8() const noexcept;
    void setGap8(int value);
    int gap12() const noexcept;
    void setGap12(int value);
    int gap20() const noexcept;
    void setGap20(int value);
    int gap24() const noexcept;
    void setGap24(int value);
    int strokeThin() const noexcept;
    void setStrokeThin(int value);
    int buttonMinWidth() const noexcept;
    void setButtonMinWidth(int value);
    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);
    bool minimapVisible() const noexcept;
    void setMinimapVisible(bool value);

    int defaultMinimapWidth() const noexcept;
    int designRowCount() const noexcept;
    int effectiveMinimapWidth() const noexcept;
    int effectiveRowCount() const noexcept;

public slots:
    void requestMetricsRefresh();

signals:
    void gapNoneChanged();
    void gap8Changed();
    void gap12Changed();
    void gap20Changed();
    void gap24Changed();
    void strokeThinChanged();
    void buttonMinWidthChanged();
    void logicalLineCountChanged();
    void minimapVisibleChanged();
    void metricsChanged();

private:
    int minimumMetricUnit() const noexcept;
    void emitChanged(void (ContentsMinimapLayoutMetrics::*signal)());

    int m_gapNone = 0;
    int m_gap8 = 0;
    int m_gap12 = 0;
    int m_gap20 = 0;
    int m_gap24 = 0;
    int m_strokeThin = 1;
    int m_buttonMinWidth = 0;
    int m_logicalLineCount = 1;
    bool m_minimapVisible = true;
};
