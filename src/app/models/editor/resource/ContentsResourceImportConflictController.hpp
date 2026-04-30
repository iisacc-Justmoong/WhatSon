#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ContentsEditorSurfaceGuardController;
class ContentsResourceTagController;

class ContentsResourceImportConflictController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* resourceTagController MEMBER m_resourceTagController)
    Q_PROPERTY(QObject* editorSurfaceGuardController MEMBER m_editorSurfaceGuardController)
    Q_PROPERTY(bool hasSelectedNote MEMBER m_hasSelectedNote)
    Q_PROPERTY(bool showDedicatedResourceViewer MEMBER m_showDedicatedResourceViewer)
    Q_PROPERTY(bool showFormattedTextRenderer MEMBER m_showFormattedTextRenderer)
    Q_PROPERTY(QObject* resourcesImportViewModel MEMBER m_resourcesImportViewModel)
    Q_PROPERTY(int resourceImportModeNone MEMBER m_resourceImportModeNone)
    Q_PROPERTY(int resourceImportModeUrls MEMBER m_resourceImportModeUrls)
    Q_PROPERTY(int resourceImportModeClipboard MEMBER m_resourceImportModeClipboard)
    Q_PROPERTY(int resourceImportConflictPolicyAbort MEMBER m_resourceImportConflictPolicyAbort)
    Q_PROPERTY(QObject* view MEMBER m_view)
    Q_PROPERTY(bool resourceImportConflictAlertOpen READ resourceImportConflictAlertOpen NOTIFY resourceImportConflictAlertOpenChanged)

public:
    explicit ContentsResourceImportConflictController(QObject* parent = nullptr);
    ~ContentsResourceImportConflictController() override;

    bool resourceImportConflictAlertOpen() const noexcept;

    Q_INVOKABLE bool canAcceptResourceDropUrls(const QVariant& urls) const;
    Q_INVOKABLE void clearPendingResourceImportConflict();
    Q_INVOKABLE QVariantMap normalizedResourceImportConflict(const QVariant& conflict) const;
    Q_INVOKABLE QString resourceImportConflictAlertMessage() const;
    Q_INVOKABLE bool scheduleResourceImportConflictPrompt(
        int importMode,
        const QVariant& urls,
        const QVariant& conflict);
    Q_INVOKABLE bool finalizeInsertedImportedResources(const QVariant& importedEntries);
    Q_INVOKABLE void cancelPendingResourceImportConflict();
    Q_INVOKABLE bool executePendingResourceImportWithPolicy(int conflictPolicy);
    Q_INVOKABLE bool importUrlsAsResourcesWithPrompt(const QVariant& urls);
    Q_INVOKABLE bool pasteClipboardImageAsResource();

signals:
    void resourceImportConflictAlertOpenChanged();

private:
    void setResourceImportConflictAlertOpen(bool open);

    QObject* m_resourceTagController = nullptr;
    QObject* m_editorSurfaceGuardController = nullptr;
    bool m_hasSelectedNote = false;
    bool m_showDedicatedResourceViewer = false;
    bool m_showFormattedTextRenderer = false;
    QObject* m_resourcesImportViewModel = nullptr;
    int m_resourceImportModeNone = 0;
    int m_resourceImportModeUrls = 1;
    int m_resourceImportModeClipboard = 2;
    int m_resourceImportConflictPolicyAbort = 0;
    QObject* m_view = nullptr;
    QVariantMap m_pendingResourceImportConflict;
    int m_pendingResourceImportMode = 0;
    QVariantList m_pendingResourceImportUrls;
    bool m_resourceImportConflictAlertOpen = false;
};
