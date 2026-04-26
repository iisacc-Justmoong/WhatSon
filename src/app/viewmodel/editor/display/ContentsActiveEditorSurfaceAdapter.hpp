#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>

class ContentsActiveEditorSurfaceAdapter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* structuredDocumentFlow READ structuredDocumentFlow WRITE setStructuredDocumentFlow NOTIFY structuredDocumentFlowChanged)
    Q_PROPERTY(QObject* contentEditor READ contentEditor WRITE setContentEditor NOTIFY contentEditorChanged)
    Q_PROPERTY(bool structuredSurfaceActive READ structuredSurfaceActive WRITE setStructuredSurfaceActive NOTIFY structuredSurfaceActiveChanged)
    Q_PROPERTY(bool inlineSurfaceActive READ inlineSurfaceActive WRITE setInlineSurfaceActive NOTIFY inlineSurfaceActiveChanged)
    Q_PROPERTY(QString activeSurfaceKind READ activeSurfaceKind NOTIFY activeSurfaceChanged)

public:
    explicit ContentsActiveEditorSurfaceAdapter(QObject* parent = nullptr);
    ~ContentsActiveEditorSurfaceAdapter() override;

    [[nodiscard]] QObject* structuredDocumentFlow() const noexcept;
    [[nodiscard]] QObject* contentEditor() const noexcept;
    [[nodiscard]] bool structuredSurfaceActive() const noexcept;
    [[nodiscard]] bool inlineSurfaceActive() const noexcept;
    [[nodiscard]] QString activeSurfaceKind() const;

public slots:
    void setStructuredDocumentFlow(QObject* surface);
    void setContentEditor(QObject* surface);
    void setStructuredSurfaceActive(bool active);
    void setInlineSurfaceActive(bool active);

    [[nodiscard]] bool requestFocus(const QVariant& request);
    [[nodiscard]] bool requestFocusAtSourceOffset(int sourceOffset);
    [[nodiscard]] bool setLogicalCursorPosition(int cursorPosition);
    [[nodiscard]] int currentLogicalCursorPosition() const;
    [[nodiscard]] bool isInputFocused() const;
    [[nodiscard]] bool supportsNativeInput() const;

signals:
    void structuredDocumentFlowChanged();
    void contentEditorChanged();
    void structuredSurfaceActiveChanged();
    void inlineSurfaceActiveChanged();
    void activeSurfaceChanged();

private:
    [[nodiscard]] bool structuredSurfaceUsable() const;
    [[nodiscard]] bool inlineSurfaceUsable() const;
    [[nodiscard]] bool requestStructuredFocus(const QVariant& request);
    [[nodiscard]] bool requestInlineFocus(const QVariant& request);
    [[nodiscard]] bool setInlineCursorPosition(int cursorPosition);
    [[nodiscard]] int cursorPositionFromRequest(const QVariant& request) const;

    QPointer<QObject> m_structuredDocumentFlow;
    QPointer<QObject> m_contentEditor;
    bool m_structuredSurfaceActive = false;
    bool m_inlineSurfaceActive = false;
};
