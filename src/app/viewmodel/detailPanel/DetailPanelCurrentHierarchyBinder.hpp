#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class DetailPanelViewModel;
class IActiveHierarchyContextSource;

class DetailPanelCurrentHierarchyBinder final : public QObject
{
    Q_OBJECT

public:
    explicit DetailPanelCurrentHierarchyBinder(QObject* parent = nullptr);
    ~DetailPanelCurrentHierarchyBinder() override;

    DetailPanelViewModel* detailPanelViewModel() const noexcept;
    void setDetailPanelViewModel(DetailPanelViewModel* detailPanelViewModel);

    IActiveHierarchyContextSource* hierarchyContextSource() const noexcept;
    void setHierarchyContextSource(IActiveHierarchyContextSource* hierarchyContextSource);

signals:
    void detailPanelViewModelChanged();
    void hierarchyContextSourceChanged();

private slots:
    void clearHierarchyContextSource();
    void synchronize();

private:
    QPointer<DetailPanelViewModel> m_detailPanelViewModel;
    QPointer<IActiveHierarchyContextSource> m_hierarchyContextSource;
    QMetaObject::Connection m_hierarchyBindingsConnection;
    QMetaObject::Connection m_hierarchyDestroyedConnection;
};
