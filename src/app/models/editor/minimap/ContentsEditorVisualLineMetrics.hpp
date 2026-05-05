#pragma once

#include <QObject>
#include <QVariantList>

class ContentsEditorVisualLineMetrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int measuredVisualLineCount READ measuredVisualLineCount WRITE setMeasuredVisualLineCount NOTIFY measuredVisualLineCountChanged)
    Q_PROPERTY(QVariantList measuredLineWidthRatios READ measuredLineWidthRatios WRITE setMeasuredLineWidthRatios NOTIFY measuredLineWidthRatiosChanged)
    Q_PROPERTY(int visualLineCount READ visualLineCount NOTIFY metricsChanged)
    Q_PROPERTY(QVariantList visualLineWidthRatios READ visualLineWidthRatios NOTIFY metricsChanged)

public:
    explicit ContentsEditorVisualLineMetrics(QObject* parent = nullptr);
    ~ContentsEditorVisualLineMetrics() override;

    int measuredVisualLineCount() const noexcept;
    void setMeasuredVisualLineCount(int value);
    QVariantList measuredLineWidthRatios() const;
    void setMeasuredLineWidthRatios(const QVariantList& value);

    int visualLineCount() const;
    QVariantList visualLineWidthRatios() const;

public slots:
    void requestMetricsRefresh();

signals:
    void measuredVisualLineCountChanged();
    void measuredLineWidthRatiosChanged();
    void metricsChanged();

private:
    void emitInputChanged(void (ContentsEditorVisualLineMetrics::*signal)());

    int m_measuredVisualLineCount = 1;
    QVariantList m_measuredLineWidthRatios;
};
