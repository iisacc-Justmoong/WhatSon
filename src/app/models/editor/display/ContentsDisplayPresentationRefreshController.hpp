#pragma once

#include <QObject>
#include <QVariantMap>

class ContentsDisplayPresentationRefreshController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool projectionEnabled READ projectionEnabled WRITE setProjectionEnabled NOTIFY projectionEnabledChanged)
    Q_PROPERTY(bool renderDirty READ renderDirty WRITE setRenderDirty NOTIFY renderDirtyChanged)
    Q_PROPERTY(bool editorInputFocused READ editorInputFocused WRITE setEditorInputFocused NOTIFY editorInputFocusedChanged)
    Q_PROPERTY(
        bool
            typingSessionSyncProtected READ typingSessionSyncProtected WRITE setTypingSessionSyncProtected
                NOTIFY typingSessionSyncProtectedChanged)
    Q_PROPERTY(bool pendingWhileFocused READ pendingWhileFocused NOTIFY pendingWhileFocusedChanged)

public:
    explicit ContentsDisplayPresentationRefreshController(QObject* parent = nullptr);
    ~ContentsDisplayPresentationRefreshController() override;

    bool projectionEnabled() const noexcept;
    void setProjectionEnabled(bool enabled);

    bool renderDirty() const noexcept;
    void setRenderDirty(bool dirty);

    bool editorInputFocused() const noexcept;
    void setEditorInputFocused(bool focused);

    bool typingSessionSyncProtected() const noexcept;
    void setTypingSessionSyncProtected(bool protectedSync);

    bool pendingWhileFocused() const noexcept;

    Q_INVOKABLE QVariantMap planDeferredRequest();
    Q_INVOKABLE QVariantMap planRefreshRequest(bool immediate);
    Q_INVOKABLE QVariantMap planDeferredTrigger();
    Q_INVOKABLE void clearPendingWhileFocused();

signals:
    void projectionEnabledChanged();
    void renderDirtyChanged();
    void editorInputFocusedChanged();
    void typingSessionSyncProtectedChanged();
    void pendingWhileFocusedChanged();

private:
    QVariantMap buildPlan(
        bool clearPresentation,
        bool stopTimer,
        bool startTimer,
        bool commitRefresh,
        bool requestMinimapRefresh,
        bool requestMinimapRepaint,
        const QString& reason) const;
    void setPendingWhileFocused(bool pending);

    bool m_projectionEnabled = true;
    bool m_renderDirty = false;
    bool m_editorInputFocused = false;
    bool m_typingSessionSyncProtected = false;
    bool m_pendingWhileFocused = false;
};
