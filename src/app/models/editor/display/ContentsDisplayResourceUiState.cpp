#include "app/models/editor/display/ContentsDisplayResourceUiState.hpp"

ContentsDisplayResourceUiState::ContentsDisplayResourceUiState(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayResourceUiState::~ContentsDisplayResourceUiState() = default;

bool ContentsDisplayResourceUiState::resourceDropActive() const noexcept { return m_resourceDropActive; }
int ContentsDisplayResourceUiState::resourceImportConflictPolicyAbort() const noexcept { return 0; }
int ContentsDisplayResourceUiState::resourceImportConflictPolicyOverwrite() const noexcept { return 1; }
int ContentsDisplayResourceUiState::resourceImportConflictPolicyKeepBoth() const noexcept { return 2; }
int ContentsDisplayResourceUiState::resourceImportModeNone() const noexcept { return 0; }
int ContentsDisplayResourceUiState::resourceImportModeUrls() const noexcept { return 1; }
int ContentsDisplayResourceUiState::resourceImportModeClipboard() const noexcept { return 2; }
QColor ContentsDisplayResourceUiState::resourceRenderBorderColor() const { return QColor(QStringLiteral("#334E5157")); }
QColor ContentsDisplayResourceUiState::resourceRenderCardColor() const { return QColor(QStringLiteral("#E61A1D22")); }
int ContentsDisplayResourceUiState::resourceRenderDisplayLimit() const noexcept { return 0; }
int ContentsDisplayResourceUiState::resourceEditorPlaceholderLineCount() const noexcept { return 1; }

void ContentsDisplayResourceUiState::setResourceDropActive(const bool value)
{
    if (m_resourceDropActive == value)
        return;
    m_resourceDropActive = value;
    emit stateChanged();
}
