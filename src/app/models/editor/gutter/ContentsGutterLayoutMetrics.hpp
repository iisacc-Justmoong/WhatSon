#pragma once

#include <QObject>

class ContentsGutterLayoutMetrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int gapNone READ gapNone WRITE setGapNone NOTIFY gapNoneChanged)
    Q_PROPERTY(int gap2 READ gap2 WRITE setGap2 NOTIFY gap2Changed)
    Q_PROPERTY(int gap3 READ gap3 WRITE setGap3 NOTIFY gap3Changed)
    Q_PROPERTY(int gap5 READ gap5 WRITE setGap5 NOTIFY gap5Changed)
    Q_PROPERTY(int gap7 READ gap7 WRITE setGap7 NOTIFY gap7Changed)
    Q_PROPERTY(int gap14 READ gap14 WRITE setGap14 NOTIFY gap14Changed)
    Q_PROPERTY(int gap20 READ gap20 WRITE setGap20 NOTIFY gap20Changed)
    Q_PROPERTY(int gap24 READ gap24 WRITE setGap24 NOTIFY gap24Changed)
    Q_PROPERTY(int strokeThin READ strokeThin WRITE setStrokeThin NOTIFY strokeThinChanged)
    Q_PROPERTY(int controlHeightMd READ controlHeightMd WRITE setControlHeightMd NOTIFY controlHeightMdChanged)
    Q_PROPERTY(int controlHeightSm READ controlHeightSm WRITE setControlHeightSm NOTIFY controlHeightSmChanged)
    Q_PROPERTY(int dialogMaxWidth READ dialogMaxWidth WRITE setDialogMaxWidth NOTIFY dialogMaxWidthChanged)
    Q_PROPERTY(int gutterWidthOverride READ gutterWidthOverride WRITE setGutterWidthOverride NOTIFY gutterWidthOverrideChanged)
    Q_PROPERTY(int inputWidthMd READ inputWidthMd WRITE setInputWidthMd NOTIFY inputWidthMdChanged)
    Q_PROPERTY(int lineNumberColumnLeftOverride READ lineNumberColumnLeftOverride WRITE setLineNumberColumnLeftOverride NOTIFY lineNumberColumnLeftOverrideChanged)
    Q_PROPERTY(int lineNumberColumnTextWidthOverride READ lineNumberColumnTextWidthOverride WRITE setLineNumberColumnTextWidthOverride NOTIFY lineNumberColumnTextWidthOverrideChanged)
    Q_PROPERTY(int changedMarkerHeight READ changedMarkerHeight NOTIFY metricsChanged)
    Q_PROPERTY(int changedMarkerY READ changedMarkerY NOTIFY metricsChanged)
    Q_PROPERTY(int conflictMarkerHeight READ conflictMarkerHeight NOTIFY metricsChanged)
    Q_PROPERTY(int conflictMarkerY READ conflictMarkerY NOTIFY metricsChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(int defaultActiveLineNumber READ defaultActiveLineNumber NOTIFY metricsChanged)
    Q_PROPERTY(int defaultGutterWidth READ defaultGutterWidth NOTIFY metricsChanged)
    Q_PROPERTY(int designLineNumberCount READ designLineNumberCount NOTIFY metricsChanged)
    Q_PROPERTY(int effectiveGutterWidth READ effectiveGutterWidth NOTIFY metricsChanged)
    Q_PROPERTY(int effectiveLineNumberCount READ effectiveLineNumberCount NOTIFY metricsChanged)
    Q_PROPERTY(int iconRailX READ iconRailX NOTIFY metricsChanged)
    Q_PROPERTY(int inactiveLineNumber READ inactiveLineNumber NOTIFY metricsChanged)
    Q_PROPERTY(int lineNumberBaseOffset READ lineNumberBaseOffset NOTIFY metricsChanged)
    Q_PROPERTY(int lineNumberColumnLeft READ lineNumberColumnLeft NOTIFY metricsChanged)
    Q_PROPERTY(int lineNumberColumnTextWidth READ lineNumberColumnTextWidth NOTIFY metricsChanged)

public:
    explicit ContentsGutterLayoutMetrics(QObject* parent = nullptr);
    ~ContentsGutterLayoutMetrics() override;

    int gapNone() const noexcept;
    void setGapNone(int value);
    int gap2() const noexcept;
    void setGap2(int value);
    int gap3() const noexcept;
    void setGap3(int value);
    int gap5() const noexcept;
    void setGap5(int value);
    int gap7() const noexcept;
    void setGap7(int value);
    int gap14() const noexcept;
    void setGap14(int value);
    int gap20() const noexcept;
    void setGap20(int value);
    int gap24() const noexcept;
    void setGap24(int value);
    int strokeThin() const noexcept;
    void setStrokeThin(int value);
    int controlHeightMd() const noexcept;
    void setControlHeightMd(int value);
    int controlHeightSm() const noexcept;
    void setControlHeightSm(int value);
    int dialogMaxWidth() const noexcept;
    void setDialogMaxWidth(int value);
    int gutterWidthOverride() const noexcept;
    void setGutterWidthOverride(int value);
    int inputWidthMd() const noexcept;
    void setInputWidthMd(int value);
    int lineNumberColumnLeftOverride() const noexcept;
    void setLineNumberColumnLeftOverride(int value);
    int lineNumberColumnTextWidthOverride() const noexcept;
    void setLineNumberColumnTextWidthOverride(int value);
    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);

    int changedMarkerHeight() const noexcept;
    int changedMarkerY() const noexcept;
    int conflictMarkerHeight() const noexcept;
    int conflictMarkerY() const noexcept;
    int defaultActiveLineNumber() const noexcept;
    int defaultGutterWidth() const noexcept;
    int designLineNumberCount() const noexcept;
    int effectiveGutterWidth() const noexcept;
    int effectiveLineNumberCount() const noexcept;
    int iconRailX() const noexcept;
    int inactiveLineNumber() const noexcept;
    int lineNumberBaseOffset() const noexcept;
    int lineNumberColumnLeft() const noexcept;
    int lineNumberColumnTextWidth() const noexcept;

public slots:
    void requestMetricsRefresh();

signals:
    void gapNoneChanged();
    void gap2Changed();
    void gap3Changed();
    void gap5Changed();
    void gap7Changed();
    void gap14Changed();
    void gap20Changed();
    void gap24Changed();
    void strokeThinChanged();
    void controlHeightMdChanged();
    void controlHeightSmChanged();
    void dialogMaxWidthChanged();
    void gutterWidthOverrideChanged();
    void inputWidthMdChanged();
    void lineNumberColumnLeftOverrideChanged();
    void lineNumberColumnTextWidthOverrideChanged();
    void logicalLineCountChanged();
    void metricsChanged();

private:
    int minimumMetricUnit() const noexcept;
    void emitChanged(void (ContentsGutterLayoutMetrics::*signal)());

    int m_gapNone = 0;
    int m_gap2 = 0;
    int m_gap3 = 0;
    int m_gap5 = 0;
    int m_gap7 = 0;
    int m_gap14 = 0;
    int m_gap20 = 0;
    int m_gap24 = 0;
    int m_strokeThin = 1;
    int m_controlHeightMd = 1;
    int m_controlHeightSm = 1;
    int m_dialogMaxWidth = 0;
    int m_gutterWidthOverride = -1;
    int m_inputWidthMd = 0;
    int m_lineNumberColumnLeftOverride = -1;
    int m_lineNumberColumnTextWidthOverride = -1;
    int m_logicalLineCount = 1;
};
