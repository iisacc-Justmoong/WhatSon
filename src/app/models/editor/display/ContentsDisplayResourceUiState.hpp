#pragma once

#include <QColor>
#include <QObject>

class ContentsDisplayResourceUiState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool resourceDropActive READ resourceDropActive WRITE setResourceDropActive NOTIFY stateChanged)
    Q_PROPERTY(int resourceImportConflictPolicyAbort READ resourceImportConflictPolicyAbort CONSTANT)
    Q_PROPERTY(int resourceImportConflictPolicyOverwrite READ resourceImportConflictPolicyOverwrite CONSTANT)
    Q_PROPERTY(int resourceImportConflictPolicyKeepBoth READ resourceImportConflictPolicyKeepBoth CONSTANT)
    Q_PROPERTY(int resourceImportModeNone READ resourceImportModeNone CONSTANT)
    Q_PROPERTY(int resourceImportModeUrls READ resourceImportModeUrls CONSTANT)
    Q_PROPERTY(int resourceImportModeClipboard READ resourceImportModeClipboard CONSTANT)
    Q_PROPERTY(QColor resourceRenderBorderColor READ resourceRenderBorderColor CONSTANT)
    Q_PROPERTY(QColor resourceRenderCardColor READ resourceRenderCardColor CONSTANT)
    Q_PROPERTY(int resourceRenderDisplayLimit READ resourceRenderDisplayLimit CONSTANT)
    Q_PROPERTY(int resourceEditorPlaceholderLineCount READ resourceEditorPlaceholderLineCount CONSTANT)

public:
    explicit ContentsDisplayResourceUiState(QObject* parent = nullptr);
    ~ContentsDisplayResourceUiState() override;

    bool resourceDropActive() const noexcept;
    int resourceImportConflictPolicyAbort() const noexcept;
    int resourceImportConflictPolicyOverwrite() const noexcept;
    int resourceImportConflictPolicyKeepBoth() const noexcept;
    int resourceImportModeNone() const noexcept;
    int resourceImportModeUrls() const noexcept;
    int resourceImportModeClipboard() const noexcept;
    QColor resourceRenderBorderColor() const;
    QColor resourceRenderCardColor() const;
    int resourceRenderDisplayLimit() const noexcept;
    int resourceEditorPlaceholderLineCount() const noexcept;

public slots:
    void setResourceDropActive(bool value);

signals:
    void stateChanged();

private:
    bool m_resourceDropActive = false;
};
