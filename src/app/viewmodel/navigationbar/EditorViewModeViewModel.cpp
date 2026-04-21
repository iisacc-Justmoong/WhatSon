#include "app/viewmodel/navigationbar/EditorViewModeViewModel.hpp"

EditorViewModeViewModel::EditorViewModeViewModel(QObject* parent)
    : QObject(parent)
      , m_plainViewModeViewModel(EditorView::Plain, this)
      , m_pageViewModeViewModel(EditorView::Page, this)
      , m_printViewModeViewModel(EditorView::Print, this)
      , m_webViewModeViewModel(EditorView::Web, this)
      , m_presentationViewModeViewModel(EditorView::Presentation, this)
{
    applyActiveViewModeViewModel(m_activeViewMode);
}

EditorViewModeViewModel::~EditorViewModeViewModel() = default;

int EditorViewModeViewModel::activeViewMode() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(m_activeViewMode);
}

QString EditorViewModeViewModel::activeViewModeName() const
{
    return WhatSon::NavigationBar::editorViewName(m_activeViewMode);
}

QObject* EditorViewModeViewModel::activeViewModeViewModel() const noexcept
{
    return m_activeViewModeViewModel;
}

QObject* EditorViewModeViewModel::plainViewModeViewModel() const noexcept
{
    return const_cast<EditorViewSectionViewModel*>(&m_plainViewModeViewModel);
}

QObject* EditorViewModeViewModel::pageViewModeViewModel() const noexcept
{
    return const_cast<EditorViewSectionViewModel*>(&m_pageViewModeViewModel);
}

QObject* EditorViewModeViewModel::printViewModeViewModel() const noexcept
{
    return const_cast<EditorViewSectionViewModel*>(&m_printViewModeViewModel);
}

QObject* EditorViewModeViewModel::webViewModeViewModel() const noexcept
{
    return const_cast<EditorViewSectionViewModel*>(&m_webViewModeViewModel);
}

QObject* EditorViewModeViewModel::presentationViewModeViewModel() const noexcept
{
    return const_cast<EditorViewSectionViewModel*>(&m_presentationViewModeViewModel);
}

QObject* EditorViewModeViewModel::viewModeViewModelForState(int viewModeValue) const noexcept
{
    if (!WhatSon::NavigationBar::isValidEditorViewValue(viewModeValue))
    {
        return nullptr;
    }

    switch (WhatSon::NavigationBar::editorViewFromValue(viewModeValue))
    {
    case EditorView::Plain:
        return plainViewModeViewModel();
    case EditorView::Page:
        return pageViewModeViewModel();
    case EditorView::Print:
        return printViewModeViewModel();
    case EditorView::Web:
        return webViewModeViewModel();
    case EditorView::Presentation:
        return presentationViewModeViewModel();
    }

    return nullptr;
}

void EditorViewModeViewModel::setActiveViewMode(int viewModeValue)
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
    applyActiveViewModeViewModel(m_activeViewMode);
    emit activeViewModeChanged();
}

void EditorViewModeViewModel::requestViewModeChange(int viewModeValue)
{
    setActiveViewMode(viewModeValue);
}

void EditorViewModeViewModel::requestNextViewMode()
{
    setActiveViewMode(
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::nextEditorView(m_activeViewMode)));
}

void EditorViewModeViewModel::applyActiveViewModeViewModel(EditorView activeViewMode)
{
    m_plainViewModeViewModel.setActive(activeViewMode == EditorView::Plain);
    m_pageViewModeViewModel.setActive(activeViewMode == EditorView::Page);
    m_printViewModeViewModel.setActive(activeViewMode == EditorView::Print);
    m_webViewModeViewModel.setActive(activeViewMode == EditorView::Web);
    m_presentationViewModeViewModel.setActive(activeViewMode == EditorView::Presentation);
    m_activeViewModeViewModel = viewModeViewModelForState(WhatSon::NavigationBar::editorViewValue(activeViewMode));
}
