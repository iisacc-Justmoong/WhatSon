#pragma once

#include <QObject>
#include <QVariantList>

class ContentsGutterMarkerBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList gutterMarkers READ gutterMarkers WRITE setGutterMarkers NOTIFY gutterMarkersChanged)
    Q_PROPERTY(
        QVariantList
            normalizedExternalGutterMarkers READ normalizedExternalGutterMarkers NOTIFY normalizedExternalGutterMarkersChanged)


public:
    explicit ContentsGutterMarkerBridge(QObject* parent = nullptr);
    ~ContentsGutterMarkerBridge() override;

    QVariantList gutterMarkers() const;
    void setGutterMarkers(const QVariantList& markers);

    QVariantList normalizedExternalGutterMarkers() const;

    signals  :



    void gutterMarkersChanged();
    void normalizedExternalGutterMarkersChanged();

private:
    static QVariantList normalizeExternalMarkers(const QVariantList& markers);

    void refreshGutterMarkerState();

    QVariantList m_gutterMarkers;
    QVariantList m_normalizedExternalGutterMarkers;
};
