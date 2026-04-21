#include "ContentsDisplayPresentationRefreshController.hpp"

#include "models/file/WhatSonDebugTrace.hpp"

ContentsDisplayPresentationRefreshController::ContentsDisplayPresentationRefreshController(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("presentationRefreshController"), QStringLiteral("ctor"));
}

ContentsDisplayPresentationRefreshController::~ContentsDisplayPresentationRefreshController()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("dtor"),
        QStringLiteral("projectionEnabled=%1 renderDirty=%2 focused=%3 typingProtected=%4 pendingWhileFocused=%5")
            .arg(m_projectionEnabled)
            .arg(m_renderDirty)
            .arg(m_editorInputFocused)
            .arg(m_typingSessionSyncProtected)
            .arg(m_pendingWhileFocused));
}

bool ContentsDisplayPresentationRefreshController::projectionEnabled() const noexcept
{
    return m_projectionEnabled;
}

void ContentsDisplayPresentationRefreshController::setProjectionEnabled(const bool enabled)
{
    if (m_projectionEnabled == enabled)
    {
        return;
    }

    m_projectionEnabled = enabled;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("setProjectionEnabled"),
        QStringLiteral("enabled=%1").arg(enabled));
    emit projectionEnabledChanged();
}

bool ContentsDisplayPresentationRefreshController::renderDirty() const noexcept
{
    return m_renderDirty;
}

void ContentsDisplayPresentationRefreshController::setRenderDirty(const bool dirty)
{
    if (m_renderDirty == dirty)
    {
        return;
    }

    m_renderDirty = dirty;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("setRenderDirty"),
        QStringLiteral("dirty=%1").arg(dirty));
    emit renderDirtyChanged();
}

bool ContentsDisplayPresentationRefreshController::editorInputFocused() const noexcept
{
    return m_editorInputFocused;
}

void ContentsDisplayPresentationRefreshController::setEditorInputFocused(const bool focused)
{
    if (m_editorInputFocused == focused)
    {
        return;
    }

    m_editorInputFocused = focused;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("setEditorInputFocused"),
        QStringLiteral("focused=%1").arg(focused));
    emit editorInputFocusedChanged();
}

bool ContentsDisplayPresentationRefreshController::typingSessionSyncProtected() const noexcept
{
    return m_typingSessionSyncProtected;
}

void ContentsDisplayPresentationRefreshController::setTypingSessionSyncProtected(const bool protectedSync)
{
    if (m_typingSessionSyncProtected == protectedSync)
    {
        return;
    }

    m_typingSessionSyncProtected = protectedSync;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("setTypingSessionSyncProtected"),
        QStringLiteral("typingProtected=%1").arg(protectedSync));
    emit typingSessionSyncProtectedChanged();
}

bool ContentsDisplayPresentationRefreshController::pendingWhileFocused() const noexcept
{
    return m_pendingWhileFocused;
}

QVariantMap ContentsDisplayPresentationRefreshController::planDeferredRequest()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("planDeferredRequest"),
        QStringLiteral("projectionEnabled=%1 focused=%2 typingProtected=%3")
            .arg(m_projectionEnabled)
            .arg(m_editorInputFocused)
            .arg(m_typingSessionSyncProtected));

    if (!m_projectionEnabled)
    {
        setPendingWhileFocused(false);
        return buildPlan(false, true, false, false, false, false, QStringLiteral("projection-disabled"));
    }

    if (m_editorInputFocused || m_typingSessionSyncProtected)
    {
        setPendingWhileFocused(true);
        return buildPlan(false, true, false, false, false, false, QStringLiteral("defer-while-focused"));
    }

    setPendingWhileFocused(false);
    return buildPlan(false, true, true, false, false, false, QStringLiteral("start-deferred-timer"));
}

QVariantMap ContentsDisplayPresentationRefreshController::planRefreshRequest(const bool immediate)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("planRefreshRequest"),
        QStringLiteral("immediate=%1 projectionEnabled=%2 renderDirty=%3 focused=%4 typingProtected=%5")
            .arg(immediate)
            .arg(m_projectionEnabled)
            .arg(m_renderDirty)
            .arg(m_editorInputFocused)
            .arg(m_typingSessionSyncProtected));

    if (!m_projectionEnabled)
    {
        setPendingWhileFocused(false);
        return buildPlan(
            true,
            true,
            false,
            false,
            true,
            true,
            QStringLiteral("projection-disabled"));
    }

    if (!m_renderDirty)
    {
        setPendingWhileFocused(false);
        return buildPlan(
            false,
            true,
            false,
            false,
            true,
            true,
            QStringLiteral("presentation-clean"));
    }

    if (m_typingSessionSyncProtected)
    {
        return planDeferredRequest();
    }

    if (immediate || !m_editorInputFocused)
    {
        setPendingWhileFocused(false);
        return buildPlan(false, true, false, true, false, false, QStringLiteral("commit-now"));
    }

    return planDeferredRequest();
}

QVariantMap ContentsDisplayPresentationRefreshController::planDeferredTrigger()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("planDeferredTrigger"),
        QStringLiteral("projectionEnabled=%1 focused=%2 typingProtected=%3")
            .arg(m_projectionEnabled)
            .arg(m_editorInputFocused)
            .arg(m_typingSessionSyncProtected));

    if (!m_projectionEnabled)
    {
        setPendingWhileFocused(false);
        return buildPlan(true, true, false, false, false, false, QStringLiteral("projection-disabled"));
    }

    if (m_editorInputFocused || m_typingSessionSyncProtected)
    {
        setPendingWhileFocused(true);
        return buildPlan(false, true, false, false, false, false, QStringLiteral("still-focused"));
    }

    setPendingWhileFocused(false);
    return buildPlan(false, true, false, true, false, false, QStringLiteral("commit-on-trigger"));
}

void ContentsDisplayPresentationRefreshController::clearPendingWhileFocused()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("clearPendingWhileFocused"));
    setPendingWhileFocused(false);
}

QVariantMap ContentsDisplayPresentationRefreshController::buildPlan(
    const bool clearPresentation,
    const bool stopTimer,
    const bool startTimer,
    const bool commitRefresh,
    const bool requestMinimapRefresh,
    const bool requestMinimapRepaint,
    const QString& reason) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("clearPresentation"), clearPresentation);
    plan.insert(QStringLiteral("commitRefresh"), commitRefresh);
    plan.insert(QStringLiteral("pendingWhileFocused"), m_pendingWhileFocused);
    plan.insert(QStringLiteral("reason"), reason);
    plan.insert(QStringLiteral("requestMinimapRefresh"), requestMinimapRefresh);
    plan.insert(QStringLiteral("requestMinimapRepaint"), requestMinimapRepaint);
    plan.insert(QStringLiteral("startTimer"), startTimer);
    plan.insert(QStringLiteral("stopTimer"), stopTimer);
    return plan;
}

void ContentsDisplayPresentationRefreshController::setPendingWhileFocused(const bool pending)
{
    if (m_pendingWhileFocused == pending)
    {
        return;
    }

    m_pendingWhileFocused = pending;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("presentationRefreshController"),
        QStringLiteral("setPendingWhileFocused"),
        QStringLiteral("pending=%1").arg(pending));
    emit pendingWhileFocusedChanged();
}
