#pragma once

#include <QString>

namespace WhatSon::EditorComponent
{
    class Break final
    {
    public:
        static QString sourceToken();
        static bool isSourceLine(const QString& sourceLine);
        static QString renderHtml();
    };
} // namespace WhatSon::EditorComponent
