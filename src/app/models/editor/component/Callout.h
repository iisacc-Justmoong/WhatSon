#pragma once

#include <QString>

namespace WhatSon::EditorComponent
{
    struct CalloutDescriptor final
    {
        QString sourceText;
        QString contentHtml;
        int editorViewportWidth = 0;
    };

    class Callout final
    {
    public:
        static int designWidth();
        static QString sourceMarker(const QString& sourceText);
        static QString renderHtml(const CalloutDescriptor& descriptor);
    };
} // namespace WhatSon::EditorComponent
