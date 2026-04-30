#pragma once

#include "app/models/detailPanel/DetailHierarchySelectionController.hpp"
#include "app/models/detailPanel/DetailContentSectionController.hpp"
#include "app/models/detailPanel/DetailCurrentNoteContextBridge.hpp"
#include "app/models/detailPanel/DetailFileStatController.hpp"
#include "app/models/detailPanel/DetailPropertiesController.hpp"
#include "app/models/detailPanel/DetailNoteHeaderSelectionSourceController.hpp"
#include "app/models/detailPanel/DetailPanelState.hpp"
#include "app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.hpp"
#include "app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVariantList>

class DetailPanelController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeState READ activeState WRITE setActiveState NOTIFY activeStateChanged)
    Q_PROPERTY(QObject* activeContentController READ activeContentController NOTIFY activeStateChanged)
    Q_PROPERTY(QString activeStateName READ activeStateName NOTIFY activeStateChanged)
    Q_PROPERTY(bool noteContextLinked READ noteContextLinked NOTIFY noteContextLinkedChanged)
    Q_PROPERTY(QObject* fileHistoryController READ fileHistoryController CONSTANT)
    Q_PROPERTY(QObject* fileStatController READ fileStatController CONSTANT)
    Q_PROPERTY(QObject* helpController READ helpController CONSTANT)
    Q_PROPERTY(QObject* insertController READ insertController CONSTANT)
    Q_PROPERTY(QObject* layerController READ layerController CONSTANT)
    Q_PROPERTY(QObject* projectSelectionController READ projectSelectionController CONSTANT)
    Q_PROPERTY(QObject* bookmarkSelectionController READ bookmarkSelectionController CONSTANT)
    Q_PROPERTY(QObject* progressSelectionController READ progressSelectionController CONSTANT)
    Q_PROPERTY(QObject* propertiesController READ propertiesController CONSTANT)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarItemsChanged)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailPanelController(QObject* parent = nullptr);
    ~DetailPanelController() override;

    int activeState() const noexcept;
    QObject* activeContentController() const noexcept;
    QString activeStateName() const;
    bool noteContextLinked() const noexcept;
    Q_INVOKABLE QObject* contentControllerForState(int stateValue) const noexcept;
    QObject* fileHistoryController() const noexcept;
    QObject* fileStatController() const noexcept;
    QObject* helpController() const noexcept;
    QObject* insertController() const noexcept;
    QObject* layerController() const noexcept;
    QObject* projectSelectionController() const noexcept;
    QObject* bookmarkSelectionController() const noexcept;
    QObject* progressSelectionController() const noexcept;
    QObject* propertiesController() const noexcept;
    QVariantList toolbarItems() const;

    Q_INVOKABLE void setActiveState(int stateValue);
    Q_INVOKABLE void requestStateChange(int stateValue);
    void setProjectSelectionSourceController(QObject* sourceController);
    void setBookmarkSelectionSourceController(QObject* sourceController);
    void setProgressSelectionSourceController(QObject* sourceController);
    void setTagsSourceController(QObject* sourceController);
    void setCurrentNoteListModel(QObject* noteListModel);
    void setCurrentNoteDirectorySourceController(QObject* sourceController);
    Q_INVOKABLE bool writeProjectSelection(int index);
    Q_INVOKABLE bool writeBookmarkSelection(int index);
    Q_INVOKABLE bool writeProgressSelection(int index);
    Q_INVOKABLE bool assignFolderByName(const QString& folderPath);
    Q_INVOKABLE bool assignTagByName(const QString& tag);
    Q_INVOKABLE bool removeActiveFolder();
    Q_INVOKABLE bool removeActiveTag();

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

signals  :


    void activeStateChanged();
    void noteContextLinkedChanged();
    void toolbarItemsChanged();
    void controllerHookRequested();

private slots:
    void handleCurrentNoteItemsChanged();

private:
    void applyActiveContentController(DetailContentState activeState);
    void setNoteContextLinked(bool linked);
    void reconnectCurrentNoteListModelSignals(QObject* noteListModel);
    void disconnectCurrentNoteListModelSignals();
    void reloadCurrentHeader(bool forceReload);
    void synchronizeCurrentNoteMetadataConsumers(const QString& noteId);
    bool ensureCurrentHeaderLoaded(QString* errorMessage = nullptr);
    QString currentNoteId() const;
    QString currentNoteDirectoryPath() const;
    bool writeSelectionIndex(
        const DetailNoteHeaderSelectionSourceController& selectionSourceController,
        int index,
        DetailNoteHeaderSelectionSourceController::Field field);
    bool removeMetadataEntry(bool removeFolder);

    WhatSon::DetailPanel::ContentState m_activeState = WhatSon::DetailPanel::ContentState::Properties;
    DetailPropertiesController m_propertiesController;
    DetailFileStatController m_fileStatController;
    DetailContentSectionController m_insertController;
    DetailContentSectionController m_fileHistoryController;
    DetailContentSectionController m_layerController;
    DetailContentSectionController m_helpController;
    WhatSonFoldersHierarchySessionService m_foldersHierarchySessionService;
    WhatSonNoteHeaderSessionStore m_noteHeaderSessionStore;
    DetailCurrentNoteContextBridge m_currentNoteContextBridge;
    DetailNoteHeaderSelectionSourceController m_projectSelectionSourceController;
    DetailNoteHeaderSelectionSourceController m_bookmarkSelectionSourceController;
    DetailNoteHeaderSelectionSourceController m_progressSelectionSourceController;
    DetailHierarchySelectionController m_projectSelectionController;
    DetailHierarchySelectionController m_bookmarkSelectionController;
    DetailHierarchySelectionController m_progressSelectionController;
    QObject* m_tagsSourceController = nullptr;
    QObject* m_activeContentController = nullptr;
    bool m_noteContextLinked = false;
    QVariantList m_toolbarItems;
    QMetaObject::Connection m_currentNoteListItemsChangedConnection;
};
