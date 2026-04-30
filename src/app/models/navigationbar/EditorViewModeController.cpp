#include "app/models/navigationbar/EditorViewModeController.hpp"

EditorViewModeController::EditorViewModeController(QObject* parent)
    : QObject(parent)
      , m_plainViewModeController(EditorView::Plain, this)
      , m_pageViewModeController(EditorView::Page, this)
      , m_printViewModeController(EditorView::Print, this)
      , m_webViewModeController(EditorView::Web, this)
      , m_presentationViewModeController(EditorView::Presentation, this)
{
    applyActiveViewModeController(m_activeViewMode);
}

EditorViewModeController::~EditorViewModeController() = default;

int EditorViewModeController::activeViewMode() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(m_activeViewMode);
}

QString EditorViewModeController::activeViewModeName() const
{
    return WhatSon::NavigationBar::editorViewName(m_activeViewMode);
}

QObject* EditorViewModeController::activeViewModeController() const noexcept
{
    return m_activeViewModeController;
}

QObject* EditorViewModeController::plainViewModeController() const noexcept
{
    return const_cast<EditorViewSectionController*>(&m_plainViewModeController);
}

QObject* EditorViewModeController::pageViewModeController() const noexcept
{
    return const_cast<EditorViewSectionController*>(&m_pageViewModeController);
}

QObject* EditorViewModeController::printViewModeController() const noexcept
{
    return const_cast<EditorViewSectionController*>(&m_printViewModeController);
}

QObject* EditorViewModeController::webViewModeController() const noexcept
{
    return const_cast<EditorViewSectionController*>(&m_webViewModeController);
}

QObject* EditorViewModeController::presentationViewModeController() const noexcept
{
    return const_cast<EditorViewSectionController*>(&m_presentationViewModeController);
}

QObject* EditorViewModeController::viewModeControllerForState(int viewModeValue) const noexcept
{
    if (!WhatSon::NavigationBar::isValidEditorViewValue(viewModeValue))
    {
        return nullptr;
    }

    switch (WhatSon::NavigationBar::editorViewFromValue(viewModeValue))
    {
    case EditorView::Plain:
        return plainViewModeController();
    case EditorView::Page:
        return pageViewModeController();
    case EditorView::Print:
        return printViewModeController();
    case EditorView::Web:
        return webViewModeController();
    case EditorView::Presentation:
        return presentationViewModeController();
    }

    return nullptr;
}

void EditorViewModeController::setActiveViewMode(int viewModeValue)
{
    if (!WhatSon::NavigationBar::isValidEditorViewValue(viewModeValue))
    {
        return;
    }

    const EditorView nextViewMode = WhatSon::NavigationBar::editorViewFromValue(viewModeValue);
    if (nextViewMode == m_activeViewMode)
    {
        return;
    }

    m_activeViewMode = nextViewMode;
    applyActiveViewModeController(m_activeViewMode);
    emit activeViewModeChanged();
}

void EditorViewModeController::requestViewModeChange(int viewModeValue)
{
    setActiveViewMode(viewModeValue);
}

void EditorViewModeController::requestNextViewMode()
{
    setActiveViewMode(
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::nextEditorView(m_activeViewMode)));
}

void EditorViewModeController::applyActiveViewModeController(EditorView activeViewMode)
{
    m_plainViewModeController.setActive(activeViewMode == EditorView::Plain);
    m_pageViewModeController.setActive(activeViewMode == EditorView::Page);
    m_printViewModeController.setActive(activeViewMode == EditorView::Print);
    m_webViewModeController.setActive(activeViewMode == EditorView::Web);
    m_presentationViewModeController.setActive(activeViewMode == EditorView::Presentation);
    m_activeViewModeController = viewModeControllerForState(WhatSon::NavigationBar::editorViewValue(activeViewMode));
}
