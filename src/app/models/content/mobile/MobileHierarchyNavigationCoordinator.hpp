#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class MobileHierarchyNavigationCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hierarchyRoutePath READ hierarchyRoutePath WRITE setHierarchyRoutePath NOTIFY hierarchyRoutePathChanged)
    Q_PROPERTY(QString noteListRoutePath READ noteListRoutePath WRITE setNoteListRoutePath NOTIFY noteListRoutePathChanged)
    Q_PROPERTY(QString editorRoutePath READ editorRoutePath WRITE setEditorRoutePath NOTIFY editorRoutePathChanged)
    Q_PROPERTY(QString detailRoutePath READ detailRoutePath WRITE setDetailRoutePath NOTIFY detailRoutePathChanged)

public:
    explicit MobileHierarchyNavigationCoordinator(QObject* parent = nullptr);
    ~MobileHierarchyNavigationCoordinator() override;

    QString hierarchyRoutePath() const;
    void setHierarchyRoutePath(const QString& value);

    QString noteListRoutePath() const;
    void setNoteListRoutePath(const QString& value);

    QString editorRoutePath() const;
    void setEditorRoutePath(const QString& value);

    QString detailRoutePath() const;
    void setDetailRoutePath(const QString& value);

    Q_INVOKABLE QString displayedBodyRoutePath(const QVariant& bodyItem, const QVariant& activePageRouter) const;
    Q_INVOKABLE QVariantMap dismissPagePlan(bool hasRouter, bool hasNoteListModel, const QString& displayedPath) const;
    Q_INVOKABLE QVariantMap openDetailPanelPlan(bool hasRouter, bool hasNoteListModel, const QString& currentPath, const QString& displayedPath, int depth) const;
    Q_INVOKABLE QVariantMap openNoteListPlan(bool hasRouter, bool hasNoteListModel, const QString& currentPath, const QString& displayedPath, int depth) const;
    Q_INVOKABLE QVariantMap openEditorPlan(const QVariant& noteId, bool hasActiveContentViewModel, bool hasNoteListModel, bool hasRouter, const QString& currentPath, const QString& displayedPath, int depth) const;
    Q_INVOKABLE QVariantMap calendarSurfacePlan(bool hasRouter, bool hasNoteListModel, const QString& displayedPath) const;
    Q_INVOKABLE QVariantMap overlayDismissPlan(bool agendaVisible, bool dayVisible, bool weekVisible, bool monthVisible, bool yearVisible) const;

signals:
    void hierarchyRoutePathChanged();
    void noteListRoutePathChanged();
    void editorRoutePathChanged();
    void detailRoutePathChanged();

private:
    QString m_hierarchyRoutePath;
    QString m_noteListRoutePath;
    QString m_editorRoutePath;
    QString m_detailRoutePath;
};
