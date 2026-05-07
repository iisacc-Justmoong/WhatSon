#pragma once

#include "app/models/editor/minimap/ContentsMinimapLayoutMetrics.hpp"
#include "app/models/editor/persistence/ContentsEditorSaveCoordinator.hpp"
#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"
#include "app/models/editor/session/ContentsEditorSessionController.hpp"
#include "app/models/editor/tags/ContentsResourceTagController.hpp"
#include "app/models/file/viewer/ContentsBodyResourceRenderer.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QVector>
#include <qqmlregistration.h>

class ContentsEditorDisplayBackend : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorDisplayBackend)

    Q_PROPERTY(QObject* contentController READ contentController WRITE setContentController NOTIFY contentControllerChanged FINAL)
    Q_PROPERTY(QObject* libraryHierarchyController READ libraryHierarchyController WRITE setLibraryHierarchyController NOTIFY libraryHierarchyControllerChanged FINAL)
    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged FINAL)
    Q_PROPERTY(QObject* noteActiveState READ noteActiveState WRITE setNoteActiveState NOTIFY noteActiveStateChanged FINAL)
    Q_PROPERTY(bool paperPaletteEnabled READ paperPaletteEnabled WRITE setPaperPaletteEnabled NOTIFY paperPaletteEnabledChanged FINAL)
    Q_PROPERTY(bool minimapVisible READ minimapVisible WRITE setMinimapVisible NOTIFY minimapVisibleChanged FINAL)
    Q_PROPERTY(int editorCursorPosition READ editorCursorPosition WRITE setEditorCursorPosition NOTIFY editorCursorPositionChanged FINAL)
    Q_PROPERTY(QObject* structuredDocumentFlow READ structuredDocumentFlow WRITE setStructuredDocumentFlow NOTIFY structuredDocumentFlowChanged FINAL)
    Q_PROPERTY(QObject* editorSession READ editorSession CONSTANT FINAL)
    Q_PROPERTY(QObject* presentationProjection READ presentationProjection CONSTANT FINAL)
    Q_PROPERTY(QObject* structuredBlockRenderer READ structuredBlockRenderer CONSTANT FINAL)
    Q_PROPERTY(QObject* bodyResourceRenderer READ bodyResourceRenderer CONSTANT FINAL)
    Q_PROPERTY(QObject* resourceTagController READ resourceTagController CONSTANT FINAL)
    Q_PROPERTY(QObject* inlineResourcePresentation READ inlineResourcePresentation CONSTANT FINAL)
    Q_PROPERTY(QObject* minimapLayoutMetrics READ minimapLayoutMetrics CONSTANT FINAL)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteChanged FINAL)
    Q_PROPERTY(QString currentNoteDirectoryPath READ currentNoteDirectoryPath NOTIFY currentNoteChanged FINAL)
    Q_PROPERTY(QString currentRawBodyText READ currentRawBodyText NOTIFY currentNoteChanged FINAL)
    Q_PROPERTY(bool noteDocumentParseMounted READ noteDocumentParseMounted NOTIFY currentNoteChanged FINAL)
    Q_PROPERTY(bool structuredBlockBackgroundRefreshEnabled READ structuredBlockBackgroundRefreshEnabled NOTIFY currentNoteChanged FINAL)

public:
    explicit ContentsEditorDisplayBackend(QObject* parent = nullptr);
    ~ContentsEditorDisplayBackend() override;

    QObject* contentController() const noexcept;
    void setContentController(QObject* value);
    QObject* libraryHierarchyController() const noexcept;
    void setLibraryHierarchyController(QObject* value);
    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* value);
    QObject* noteActiveState() const noexcept;
    void setNoteActiveState(QObject* value);
    bool paperPaletteEnabled() const noexcept;
    void setPaperPaletteEnabled(bool value);
    bool minimapVisible() const noexcept;
    void setMinimapVisible(bool value);
    int editorCursorPosition() const noexcept;
    void setEditorCursorPosition(int value);
    QObject* structuredDocumentFlow() const noexcept;
    void setStructuredDocumentFlow(QObject* value);

    QObject* editorSession() noexcept;
    QObject* presentationProjection() noexcept;
    QObject* structuredBlockRenderer() noexcept;
    QObject* bodyResourceRenderer() noexcept;
    QObject* resourceTagController() noexcept;
    QObject* inlineResourcePresentation() noexcept;
    QObject* minimapLayoutMetrics() noexcept;

    QString currentNoteId() const;
    QString currentNoteDirectoryPath() const;
    QString currentRawBodyText() const;
    bool noteDocumentParseMounted() const;
    bool structuredBlockBackgroundRefreshEnabled() const;

    Q_INVOKABLE QVariant applyDocumentSourceMutation(const QVariant& payload, const QVariant& focusRequest = {});
    Q_INVOKABLE bool commitEditedSourceText(const QString& text);
    Q_INVOKABLE void commitDocumentPresentationRefresh();
    Q_INVOKABLE QVariant encodeXmlAttributeValue(const QVariant& value) const;
    Q_INVOKABLE QVariant currentEditorCursorPosition() const noexcept;
    Q_INVOKABLE QVariant terminalBodyClickSourceOffset() const noexcept;
    Q_INVOKABLE QVariant requestEditorSelectionContextMenuFromPointer(const QVariant& pointerKind) const;
    Q_INVOKABLE QString renderInlineResourceEditorSurfaceHtml(
        const QString& editorHtml,
        const QVariant& renderedResources,
        int targetFrameWidth) const;
    Q_INVOKABLE bool syncSessionFromCurrentNote(bool resetViewport);

signals:
    void contentControllerChanged();
    void libraryHierarchyControllerChanged();
    void noteListModelChanged();
    void noteActiveStateChanged();
    void paperPaletteEnabledChanged();
    void minimapVisibleChanged();
    void editorCursorPositionChanged();
    void structuredDocumentFlowChanged();
    void currentNoteChanged();
    void editorViewportResetRequested();
    void editorTextEdited(const QString& text);

private:
    enum class SyncReset
    {
        PreserveViewport,
        ResetViewport
    };

private slots:
    void handleResetCurrentNoteFromModel();
    void handlePreserveCurrentNoteFromModel();
    void handleResetCurrentNoteFromActiveState();
    void handlePreserveCurrentNoteFromActiveState();

private:
    bool connectPropertyNotify(QObject* source, const char* propertyName, const char* slotSignature);
    bool invokeNoArgBool(QObject* target, const char* methodName) const;
    void clearNoteActiveStateSession(QObject* state);
    void clearNoteActiveStateSaveCoordinator(QObject* state);
    void attachEditorSessionToActiveState();
    void attachEditorSaveCoordinatorToActiveState();
    void disconnectNoteListModel();
    void disconnectNoteActiveState();
    QVariantMap currentNoteEntry() const;
    QString modelStringProperty(const char* propertyName) const;
    QString activeStateStringProperty(const char* propertyName) const;
    QString stringValue(const QVariantMap& payload, const QString& key, const QString& fallback) const;
    void publishCurrentNoteChanged(SyncReset reset);
    bool currentNoteIdentityDiffersFromBoundSession() const;
    void syncProjectionInputs();
    void syncResourceInputs();

    QPointer<QObject> m_contentController;
    QPointer<QObject> m_libraryHierarchyController;
    QPointer<QObject> m_noteListModel;
    QPointer<QObject> m_noteActiveState;
    QPointer<QObject> m_structuredDocumentFlow;
    QVector<QMetaObject::Connection> m_noteListConnections;
    QVector<QMetaObject::Connection> m_noteActiveStateConnections;
    ContentsEditorSessionController m_editorSession;
    ContentsEditorSaveCoordinator m_editorSaveCoordinator;
    ContentsEditorPresentationProjection m_presentationProjection;
    ContentsStructuredBlockRenderer m_structuredBlockRenderer;
    ContentsBodyResourceRenderer m_bodyResourceRenderer;
    ContentsResourceTagController m_resourceTagController;
    ContentsInlineResourcePresentationController m_inlineResourcePresentation;
    ContentsMinimapLayoutMetrics m_minimapLayoutMetrics;
    bool m_paperPaletteEnabled = false;
    bool m_minimapVisible = true;
    int m_editorCursorPosition = 0;
};
