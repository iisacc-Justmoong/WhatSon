#include "EditorViewState.hpp"

namespace WhatSon::NavigationBar
{
    bool isValidEditorViewValue(int value) noexcept
    {
        return value >= editorViewValue(EditorView::Plain)
            && value <= editorViewValue(EditorView::Presentation);
    }

    EditorView editorViewFromValue(int value) noexcept
    {
        switch (value)
        {
        case 0:
            return EditorView::Plain;
        case 1:
            return EditorView::Page;
        case 2:
            return EditorView::Print;
        case 3:
            return EditorView::Web;
        case 4:
            return EditorView::Presentation;
        default:
            return EditorView::Plain;
        }
    }

    int editorViewValue(EditorView view) noexcept
    {
        return static_cast<int>(view);
    }

    QString editorViewName(EditorView view)
    {
        switch (view)
        {
        case EditorView::Plain:
            return QStringLiteral("Plain");
        case EditorView::Page:
            return QStringLiteral("Page");
        case EditorView::Print:
            return QStringLiteral("Print");
        case EditorView::Web:
            return QStringLiteral("Web");
        case EditorView::Presentation:
            return QStringLiteral("Presentation");
        }

        return QString();
    }

    QString editorViewNameFromValue(int value)
    {
        if (!isValidEditorViewValue(value))
        {
            return QString();
        }

        return editorViewName(editorViewFromValue(value));
    }

    EditorView nextEditorView(EditorView view) noexcept
    {
        switch (view)
        {
        case EditorView::Plain:
            return EditorView::Page;
        case EditorView::Page:
            return EditorView::Print;
        case EditorView::Print:
            return EditorView::Web;
        case EditorView::Web:
            return EditorView::Presentation;
        case EditorView::Presentation:
            return EditorView::Plain;
        }

        return EditorView::Plain;
    }
} // namespace WhatSon::NavigationBar
