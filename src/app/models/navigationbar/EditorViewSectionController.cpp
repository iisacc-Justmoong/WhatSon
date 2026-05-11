#include "app/models/navigationbar/EditorViewSectionController.hpp"

EditorViewSectionController::EditorViewSectionController(
    EditorView editorView,
    QObject* parent)
    : QObject(parent)
      , m_editorView(editorView)
{
}

bool EditorViewSectionController::active() const noexcept
{
    return m_active;
}

int EditorViewSectionController::editorViewValue() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(m_editorView);
}

QString EditorViewSectionController::editorViewName() const
{
    return WhatSon::NavigationBar::editorViewName(m_editorView);
}

void EditorViewSectionController::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    emit activeChanged();
}
