#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class MobileHierarchyCanonicalRoutePlanner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hierarchyRoutePath READ hierarchyRoutePath WRITE setHierarchyRoutePath NOTIFY hierarchyRoutePathChanged)
    Q_PROPERTY(QString noteListRoutePath READ noteListRoutePath WRITE setNoteListRoutePath NOTIFY noteListRoutePathChanged)
    Q_PROPERTY(QString editorRoutePath READ editorRoutePath WRITE setEditorRoutePath NOTIFY editorRoutePathChanged)
    Q_PROPERTY(QString detailRoutePath READ detailRoutePath WRITE setDetailRoutePath NOTIFY detailRoutePathChanged)

public:
    explicit MobileHierarchyCanonicalRoutePlanner(QObject* parent = nullptr);
    ~MobileHierarchyCanonicalRoutePlanner() override;

    QString hierarchyRoutePath() const;
    void setHierarchyRoutePath(const QString& value);

    QString noteListRoutePath() const;
    void setNoteListRoutePath(const QString& value);

    QString editorRoutePath() const;
    void setEditorRoutePath(const QString& value);

    QString detailRoutePath() const;
    void setDetailRoutePath(const QString& value);

    Q_INVOKABLE int routeStackDepth(const QVariant& depthValue) const;
    Q_INVOKABLE QVariantMap canonicalRoutePlan(const QString& targetPath, bool hasRouter, bool hasNoteListModel, bool transitionActive) const;
    Q_INVOKABLE QVariantMap hierarchyRootPlan(bool hasRouter, bool transitionActive, const QString& displayedPath, bool canGoBack) const;

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
