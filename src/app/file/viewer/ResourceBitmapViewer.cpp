#include "ResourceBitmapViewer.hpp"

#include "ImageFormatCompatibilityLayer.hpp"

#include <QDir>
#include <QUrl>
#include <QVariantMap>

namespace
{
    QString normalizedRenderMode(const QVariantMap& resourceEntry)
    {
        return resourceEntry.value(QStringLiteral("renderMode")).toString().trimmed().toCaseFolded();
    }
} // namespace

ResourceBitmapViewer::ResourceBitmapViewer(QObject* parent)
    : QObject(parent)
{
}

ResourceBitmapViewer::~ResourceBitmapViewer() = default;

QVariant ResourceBitmapViewer::resourceEntry() const
{
    return m_resourceEntry;
}

void ResourceBitmapViewer::setResourceEntry(const QVariant& resourceEntry)
{
    if (m_resourceEntry == resourceEntry)
    {
        return;
    }

    m_resourceEntry = resourceEntry;
    emit resourceEntryChanged();
    refreshState();
}

QString ResourceBitmapViewer::openTarget() const
{
    return m_openTarget;
}

QString ResourceBitmapViewer::viewerSource() const
{
    return m_viewerSource;
}

QString ResourceBitmapViewer::normalizedFormat() const
{
    return m_normalizedFormat;
}

bool ResourceBitmapViewer::bitmapFormatCompatible() const noexcept
{
    return m_bitmapFormatCompatible;
}

bool ResourceBitmapViewer::bitmapRenderable() const noexcept
{
    return m_bitmapRenderable;
}

QString ResourceBitmapViewer::incompatibilityReason() const
{
    return m_incompatibilityReason;
}

void ResourceBitmapViewer::requestRefresh()
{
    refreshState();
}

QString ResourceBitmapViewer::toViewerTarget(const QString& pathOrUrl)
{
    const QString value = pathOrUrl.trimmed();
    if (value.isEmpty())
    {
        return {};
    }

    if (value.contains(QStringLiteral("://")) || value.startsWith(QStringLiteral("qrc:/")))
    {
        return value;
    }

    if (value.startsWith(QLatin1Char('/')))
    {
        return QUrl::fromLocalFile(QDir::cleanPath(value)).toString();
    }

    return value;
}

QString ResourceBitmapViewer::stringValue(const QVariantMap& map, const QString& key)
{
    return map.value(key).toString().trimmed();
}

void ResourceBitmapViewer::refreshState()
{
    const QVariantMap entry = m_resourceEntry.toMap();
    const QString renderMode = normalizedRenderMode(entry);
    const QString sourceValue = stringValue(entry, QStringLiteral("source"));
    const QString resolvedPathValue = stringValue(entry, QStringLiteral("resolvedPath"));

    QString nextOpenTarget = sourceValue;
    if (nextOpenTarget.isEmpty())
    {
        nextOpenTarget = toViewerTarget(resolvedPathValue);
    }

    QString formatProbe = stringValue(entry, QStringLiteral("format"));
    if (formatProbe.isEmpty())
    {
        formatProbe = stringValue(entry, QStringLiteral("assetPath"));
    }
    if (formatProbe.isEmpty())
    {
        formatProbe = stringValue(entry, QStringLiteral("resourcePath"));
    }
    if (formatProbe.isEmpty())
    {
        formatProbe = resolvedPathValue;
    }
    if (formatProbe.isEmpty())
    {
        formatProbe = nextOpenTarget;
    }

    const QString nextNormalizedFormat =
        WhatSon::Viewer::ImageFormatCompatibilityLayer::normalizedBitmapFormat(formatProbe);
    const bool nextBitmapFormatCompatible =
        WhatSon::Viewer::ImageFormatCompatibilityLayer::isBitmapFormatCompatible(nextNormalizedFormat);
    const bool nextBitmapRenderable =
        renderMode == QStringLiteral("image")
        && !nextOpenTarget.isEmpty()
        && nextBitmapFormatCompatible;
    const QString nextViewerSource = nextBitmapRenderable ? nextOpenTarget : QString();

    QString nextIncompatibilityReason;
    if (renderMode == QStringLiteral("image"))
    {
        if (nextOpenTarget.isEmpty())
        {
            nextIncompatibilityReason = QStringLiteral("Missing resource source path for bitmap preview.");
        }
        else if (!nextBitmapFormatCompatible)
        {
            nextIncompatibilityReason =
                WhatSon::Viewer::ImageFormatCompatibilityLayer::unsupportedBitmapFormatMessage(formatProbe);
        }
    }

    if (m_openTarget == nextOpenTarget
        && m_viewerSource == nextViewerSource
        && m_normalizedFormat == nextNormalizedFormat
        && m_bitmapFormatCompatible == nextBitmapFormatCompatible
        && m_bitmapRenderable == nextBitmapRenderable
        && m_incompatibilityReason == nextIncompatibilityReason)
    {
        return;
    }

    m_openTarget = nextOpenTarget;
    m_viewerSource = nextViewerSource;
    m_normalizedFormat = nextNormalizedFormat;
    m_bitmapFormatCompatible = nextBitmapFormatCompatible;
    m_bitmapRenderable = nextBitmapRenderable;
    m_incompatibilityReason = nextIncompatibilityReason;
    emit viewerStateChanged();
}

