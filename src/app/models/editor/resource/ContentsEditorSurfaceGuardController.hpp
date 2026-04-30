#pragma once

#include <QObject>

class ContentsEditorSurfaceGuardController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentEditor MEMBER m_contentEditor)
    Q_PROPERTY(QObject* editorProjection MEMBER m_editorProjection)
    Q_PROPERTY(
        bool
            resourceDropEditorSurfaceGuardActive READ resourceDropEditorSurfaceGuardActive
                NOTIFY resourceDropEditorSurfaceGuardActiveChanged)
    Q_PROPERTY(
        bool
            programmaticEditorSurfaceSyncActive READ programmaticEditorSurfaceSyncActive
                NOTIFY programmaticEditorSurfaceSyncActiveChanged)

public:
    explicit ContentsEditorSurfaceGuardController(QObject* parent = nullptr);
    ~ContentsEditorSurfaceGuardController() override;

    bool resourceDropEditorSurfaceGuardActive() const noexcept;
    bool programmaticEditorSurfaceSyncActive() const noexcept;

    Q_INVOKABLE void activateResourceDropEditorSurfaceGuard();
    Q_INVOKABLE void markProgrammaticEditorSurfaceSync();
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE QString presentationSurfaceText() const;
    Q_INVOKABLE bool shouldRejectEditorSurfaceRestore(const QString& nextSurfaceText) const;
    Q_INVOKABLE bool restoreEditorSurfaceFromPresentation();
    Q_INVOKABLE bool restorePendingEditorSurfaceFromPresentationIfInputSettled();
    Q_INVOKABLE void releaseResourceDropEditorSurfaceGuard(bool restoreSurface);

signals:
    void resourceDropEditorSurfaceGuardActiveChanged();
    void programmaticEditorSurfaceSyncActiveChanged();

private:
    void setResourceDropEditorSurfaceGuardActive(bool active);
    void decrementProgrammaticEditorSurfaceSyncDepth();

    QObject* m_contentEditor = nullptr;
    QObject* m_editorProjection = nullptr;
    bool m_resourceDropEditorSurfaceGuardActive = false;
    int m_resourceDropEditorSurfaceGuardToken = 0;
    bool m_pendingEditorSurfaceRestore = false;
    int m_programmaticEditorSurfaceSyncDepth = 0;
};
