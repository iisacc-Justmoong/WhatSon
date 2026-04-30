#include "app/models/editor/display/ContentsDisplayPresentationState.hpp"

ContentsDisplayPresentationState::ContentsDisplayPresentationState(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayPresentationState::~ContentsDisplayPresentationState() = default;

int ContentsDisplayPresentationState::documentPresentationRefreshIntervalMs() const noexcept
{
    return m_documentPresentationRefreshIntervalMs;
}

QString ContentsDisplayPresentationState::renderedEditorHtml() const
{
    return m_renderedEditorHtml;
}

void ContentsDisplayPresentationState::setDocumentPresentationRefreshIntervalMs(const int value)
{
    if (m_documentPresentationRefreshIntervalMs == value)
        return;
    m_documentPresentationRefreshIntervalMs = value;
    emit stateChanged();
}

void ContentsDisplayPresentationState::setRenderedEditorHtml(const QString& value)
{
    if (m_renderedEditorHtml == value)
        return;
    m_renderedEditorHtml = value;
    emit stateChanged();
}
