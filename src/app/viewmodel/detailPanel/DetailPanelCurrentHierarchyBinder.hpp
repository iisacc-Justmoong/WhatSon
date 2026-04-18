#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class DetailPanelViewModel;
class IActiveHierarchyContextSource;
class NoteDetailPanelViewModel;
class ResourceDetailPanelViewModel;

class DetailPanelCurrentHierarchyBinder final : public QObject
{
    Q_OBJECT

public:
    explicit DetailPanelCurrentHierarchyBinder(QObject* parent = nullptr);
    ~DetailPanelCurrentHierarchyBinder() override;

    NoteDetailPanelViewModel* noteDetailPanelViewModel() const noexcept;
    void setNoteDetailPanelViewModel(NoteDetailPanelViewModel* detailPanelViewModel);

    ResourceDetailPanelViewModel* resourceDetailPanelViewModel() const noexcept;
    void setResourceDetailPanelViewModel(ResourceDetailPanelViewModel* detailPanelViewModel);

    IActiveHierarchyContextSource* hierarchyContextSource() const noexcept;
    void setHierarchyContextSource(IActiveHierarchyContextSource* hierarchyContextSource);

signals:
    void noteDetailPanelViewModelChanged();
    void resourceDetailPanelViewModelChanged();
    void hierarchyContextSourceChanged();

private slots:
    void clearHierarchyContextSource();
    void synchronize();

private:
    QPointer<NoteDetailPanelViewModel> m_noteDetailPanelViewModel;
    QPointer<ResourceDetailPanelViewModel> m_resourceDetailPanelViewModel;
    QPointer<IActiveHierarchyContextSource> m_hierarchyContextSource;
    QMetaObject::Connection m_hierarchyBindingsConnection;
    QMetaObject::Connection m_hierarchyDestroyedConnection;
};
