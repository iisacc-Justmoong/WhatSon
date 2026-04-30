#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class DetailPanelController;
class IActiveHierarchyContextSource;
class NoteDetailPanelController;
class ResourceDetailPanelController;

class DetailPanelCurrentHierarchyBinder final : public QObject
{
    Q_OBJECT

public:
    explicit DetailPanelCurrentHierarchyBinder(QObject* parent = nullptr);
    ~DetailPanelCurrentHierarchyBinder() override;

    NoteDetailPanelController* noteDetailPanelController() const noexcept;
    void setNoteDetailPanelController(NoteDetailPanelController* detailPanelController);

    ResourceDetailPanelController* resourceDetailPanelController() const noexcept;
    void setResourceDetailPanelController(ResourceDetailPanelController* detailPanelController);

    IActiveHierarchyContextSource* hierarchyContextSource() const noexcept;
    void setHierarchyContextSource(IActiveHierarchyContextSource* hierarchyContextSource);

signals:
    void noteDetailPanelControllerChanged();
    void resourceDetailPanelControllerChanged();
    void hierarchyContextSourceChanged();

private slots:
    void clearHierarchyContextSource();
    void synchronize();

private:
    QPointer<NoteDetailPanelController> m_noteDetailPanelController;
    QPointer<ResourceDetailPanelController> m_resourceDetailPanelController;
    QPointer<IActiveHierarchyContextSource> m_hierarchyContextSource;
    QMetaObject::Connection m_hierarchyBindingsConnection;
    QMetaObject::Connection m_hierarchyDestroyedConnection;
};
