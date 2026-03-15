#include "ContentsGutterMarkerBridge.hpp"

#include <QVariantMap>

#include <algorithm>

ContentsGutterMarkerBridge::ContentsGutterMarkerBridge(QObject* parent)
    : QObject(parent)
{
    refreshGutterMarkerState();
}

ContentsGutterMarkerBridge::~ContentsGutterMarkerBridge() = default;

QVariantList ContentsGutterMarkerBridge::gutterMarkers() const
{
    return m_gutterMarkers;
}

void ContentsGutterMarkerBridge::setGutterMarkers(const QVariantList& markers)
{
    if (m_gutterMarkers == markers)
    {
        return;
    }

    m_gutterMarkers = markers;
    emit gutterMarkersChanged();
    refreshGutterMarkerState();
}

QVariantList ContentsGutterMarkerBridge::normalizedExternalGutterMarkers() const
{
    return m_normalizedExternalGutterMarkers;
}

QVariantList ContentsGutterMarkerBridge::normalizeExternalMarkers(const QVariantList& markers)
{
    QVariantList normalized;
    normalized.reserve(markers.size());

    for (const QVariant& markerValue : markers)
    {
        const QVariantMap marker = markerValue.toMap();
        if (marker.isEmpty())
        {
            continue;
        }

        const QVariant rawStartLine = marker.value(QStringLiteral("startLine"), marker.value(QStringLiteral("line")));
        const int startLine = std::max(1, rawStartLine.toInt());
        const bool hasExplicitSpan = marker.contains(QStringLiteral("lineSpan"))
            && marker.value(QStringLiteral("lineSpan")).isValid();
        const int explicitSpan = hasExplicitSpan ? std::max(1, marker.value(QStringLiteral("lineSpan")).toInt()) : 0;
        const int endLine = std::max(startLine, marker.value(QStringLiteral("endLine"), startLine).toInt());
        const int lineSpan = explicitSpan > 0 ? explicitSpan : std::max(1, endLine - startLine + 1);
        const QString markerType = marker.value(QStringLiteral("type")).toString().trimmed().toLower();
        if (markerType != QStringLiteral("changed")
            && markerType != QStringLiteral("conflict")
            && markerType != QStringLiteral("current"))
        {
            continue;
        }

        QVariantMap normalizedMarker;
        normalizedMarker.insert(QStringLiteral("lineSpan"), lineSpan);
        normalizedMarker.insert(QStringLiteral("startLine"), startLine);
        normalizedMarker.insert(QStringLiteral("type"), markerType);
        normalized.push_back(normalizedMarker);
    }

    return normalized;
}

void ContentsGutterMarkerBridge::refreshGutterMarkerState()
{
    const QVariantList nextMarkers = normalizeExternalMarkers(m_gutterMarkers);
    if (m_normalizedExternalGutterMarkers == nextMarkers)
    {
        return;
    }

    m_normalizedExternalGutterMarkers = nextMarkers;
    emit normalizedExternalGutterMarkersChanged();
}
