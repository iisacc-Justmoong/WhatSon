#include "app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QPointer>
#include <QTimer>

using namespace WhatSon::Editor::DynamicObjectSupport;

ContentsEditorSurfaceGuardController::ContentsEditorSurfaceGuardController(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorSurfaceGuardController::~ContentsEditorSurfaceGuardController() = default;

bool ContentsEditorSurfaceGuardController::resourceDropEditorSurfaceGuardActive() const noexcept
{
    return m_resourceDropEditorSurfaceGuardActive;
}

bool ContentsEditorSurfaceGuardController::programmaticEditorSurfaceSyncActive() const noexcept
{
    return m_programmaticEditorSurfaceSyncDepth > 0;
}

void ContentsEditorSurfaceGuardController::activateResourceDropEditorSurfaceGuard()
{
    ++m_resourceDropEditorSurfaceGuardToken;
    if (!m_resourceDropEditorSurfaceGuardActive)
    {
        setResourceDropEditorSurfaceGuardActive(true);
    }
}

void ContentsEditorSurfaceGuardController::markProgrammaticEditorSurfaceSync()
{
    const bool wasActive = programmaticEditorSurfaceSyncActive();
    ++m_programmaticEditorSurfaceSyncDepth;
    if (!wasActive)
    {
        emit programmaticEditorSurfaceSyncActiveChanged();
    }

    QPointer<ContentsEditorSurfaceGuardController> self(this);
    QTimer::singleShot(0, this, [self]() {
        if (!self)
        {
            return;
        }
        QTimer::singleShot(0, self, [self]() {
            if (!self)
            {
                return;
            }
            self->decrementProgrammaticEditorSurfaceSyncDepth();
        });
    });
}

bool ContentsEditorSurfaceGuardController::nativeCompositionActive() const
{
    if (!m_contentEditor)
    {
        return false;
    }

    const QVariant nativeCompositionResult = invokeVariant(m_contentEditor, "nativeCompositionActive");
    if (nativeCompositionResult.isValid())
    {
        return nativeCompositionResult.toBool();
    }

    const QString activePreeditText = stringProperty(m_contentEditor, "preeditText");
    return boolProperty(m_contentEditor, "inputMethodComposing") || !activePreeditText.isEmpty();
}

QString ContentsEditorSurfaceGuardController::presentationSurfaceText() const
{
    return stringProperty(m_editorProjection, "logicalText");
}

bool ContentsEditorSurfaceGuardController::shouldRejectEditorSurfaceRestore(const QString& nextSurfaceText) const
{
    if (!m_contentEditor)
    {
        return true;
    }

    const QVariant rejectionResult = invokeVariant(
        m_contentEditor,
        "shouldRejectFocusedProgrammaticTextSync",
        { nextSurfaceText });
    return rejectionResult.isValid() && rejectionResult.toBool();
}

bool ContentsEditorSurfaceGuardController::restoreEditorSurfaceFromPresentation()
{
    if (!m_contentEditor)
    {
        m_pendingEditorSurfaceRestore = false;
        return false;
    }

    const QString nextSurfaceText = presentationSurfaceText();
    if (nativeCompositionActive())
    {
        m_pendingEditorSurfaceRestore = true;
        return false;
    }
    if (shouldRejectEditorSurfaceRestore(nextSurfaceText))
    {
        m_pendingEditorSurfaceRestore = false;
        return false;
    }

    m_pendingEditorSurfaceRestore = false;
    markProgrammaticEditorSurfaceSync();
    const QVariant programmaticResult = invokeVariant(
        m_contentEditor,
        "setProgrammaticText",
        { nextSurfaceText });
    if (programmaticResult.isValid())
    {
        return true;
    }

    if (m_contentEditor->property("text").toString() != nextSurfaceText)
    {
        m_contentEditor->setProperty("text", nextSurfaceText);
        return true;
    }
    return false;
}

bool ContentsEditorSurfaceGuardController::restorePendingEditorSurfaceFromPresentationIfInputSettled()
{
    if (!m_pendingEditorSurfaceRestore || nativeCompositionActive())
    {
        return false;
    }
    return restoreEditorSurfaceFromPresentation();
}

void ContentsEditorSurfaceGuardController::releaseResourceDropEditorSurfaceGuard(const bool restoreSurface)
{
    const int scheduledToken = std::max(0, m_resourceDropEditorSurfaceGuardToken);
    if (!restoreSurface)
    {
        setResourceDropEditorSurfaceGuardActive(false);
        return;
    }

    QPointer<ContentsEditorSurfaceGuardController> self(this);
    QTimer::singleShot(0, this, [self, scheduledToken]() {
        if (!self)
        {
            return;
        }
        QTimer::singleShot(0, self, [self, scheduledToken]() {
            if (!self || self->m_resourceDropEditorSurfaceGuardToken != scheduledToken)
            {
                return;
            }
            self->restoreEditorSurfaceFromPresentation();
            QTimer::singleShot(0, self, [self, scheduledToken]() {
                if (!self || self->m_resourceDropEditorSurfaceGuardToken != scheduledToken)
                {
                    return;
                }
                self->setResourceDropEditorSurfaceGuardActive(false);
            });
        });
    });
}

void ContentsEditorSurfaceGuardController::setResourceDropEditorSurfaceGuardActive(const bool active)
{
    if (m_resourceDropEditorSurfaceGuardActive == active)
    {
        return;
    }

    m_resourceDropEditorSurfaceGuardActive = active;
    emit resourceDropEditorSurfaceGuardActiveChanged();
}

void ContentsEditorSurfaceGuardController::decrementProgrammaticEditorSurfaceSyncDepth()
{
    const bool wasActive = programmaticEditorSurfaceSyncActive();
    m_programmaticEditorSurfaceSyncDepth = std::max(0, m_programmaticEditorSurfaceSyncDepth - 1);
    if (wasActive != programmaticEditorSurfaceSyncActive())
    {
        emit programmaticEditorSurfaceSyncActiveChanged();
    }
}
