#pragma once

#include <QSize>
#include <QString>

namespace WhatSon::EditorComponent
{
    struct ResourceFrameDescriptor final
    {
        QString sourceTag;
        QString resourcePath;
        QString resourceId;
        QString type;
        QString format;
        QString resolvedAssetPath;
        int editorViewportWidth = 0;
    };

    class ResourceFrame final
    {
    public:
        static QSize previewViewportSize();
        static QSize imageDisplaySize(const QSize& sourceSize);
        static QString sourceMarker(const QString& sourceTag);
        static QString renderHtml(const ResourceFrameDescriptor& descriptor);
    };
} // namespace WhatSon::EditorComponent
