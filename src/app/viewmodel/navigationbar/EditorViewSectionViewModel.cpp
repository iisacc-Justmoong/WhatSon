#include "EditorViewSectionViewModel.hpp"

EditorViewSectionViewModel::EditorViewSectionViewModel(
    EditorView editorView,
    QObject* parent)
    : QObject(parent)
      , m_editorView(editorView)
{
}

bool EditorViewSectionViewModel::active() const noexcept
{
    return m_active;
}

int EditorViewSectionViewModel::editorViewValue() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(m_editorView);
}

QString EditorViewSectionViewModel::editorViewName() const
{
    return WhatSon::NavigationBar::editorViewName(m_editorView);
}

void EditorViewSectionViewModel::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    emit activeChanged();
}
