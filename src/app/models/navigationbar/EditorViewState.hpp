#pragma once

#include <QString>

namespace WhatSon::NavigationBar
{
    enum class EditorView
    {
        Plain = 0,
        Page,
        Print,
        Web,
        Presentation
    };

    bool isValidEditorViewValue(int value) noexcept;
    EditorView editorViewFromValue(int value) noexcept;
    int editorViewValue(EditorView view) noexcept;
    QString editorViewName(EditorView view);
    QString editorViewNameFromValue(int value);
    EditorView nextEditorView(EditorView view) noexcept;
} // namespace WhatSon::NavigationBar
