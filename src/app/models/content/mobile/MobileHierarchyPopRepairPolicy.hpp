#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class MobileHierarchyPopRepairPolicy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString noteListRoutePath READ noteListRoutePath WRITE setNoteListRoutePath NOTIFY noteListRoutePathChanged)
    Q_PROPERTY(QString editorRoutePath READ editorRoutePath WRITE setEditorRoutePath NOTIFY editorRoutePathChanged)
    Q_PROPERTY(QString detailRoutePath READ detailRoutePath WRITE setDetailRoutePath NOTIFY detailRoutePathChanged)
    Q_PROPERTY(int editorPopRepairRequestId READ editorPopRepairRequestId WRITE setEditorPopRepairRequestId NOTIFY editorPopRepairRequestIdChanged)
    Q_PROPERTY(int detailPopRepairRequestId READ detailPopRepairRequestId WRITE setDetailPopRepairRequestId NOTIFY detailPopRepairRequestIdChanged)

public:
    explicit MobileHierarchyPopRepairPolicy(QObject* parent = nullptr);
    ~MobileHierarchyPopRepairPolicy() override;

    QString noteListRoutePath() const;
    void setNoteListRoutePath(const QString& value);

    QString editorRoutePath() const;
    void setEditorRoutePath(const QString& value);

    QString detailRoutePath() const;
    void setDetailRoutePath(const QString& value);

    int editorPopRepairRequestId() const noexcept;
    void setEditorPopRepairRequestId(int value);

    int detailPopRepairRequestId() const noexcept;
    void setDetailPopRepairRequestId(int value);

    Q_INVOKABLE QVariantMap repairVerificationPlan(int requestId, bool editorRepair, bool hasRouter, bool hasNoteListModel, const QString& displayedPath, const QString& currentPath, int depth, int attemptsRemaining) const;
    Q_INVOKABLE QVariantMap committedTransitionPlan(const QVariantMap& state, bool hasNoteListModel, const QString& displayedPath) const;

signals:
    void noteListRoutePathChanged();
    void editorRoutePathChanged();
    void detailRoutePathChanged();
    void editorPopRepairRequestIdChanged();
    void detailPopRepairRequestIdChanged();

private:
    QString m_noteListRoutePath;
    QString m_editorRoutePath;
    QString m_detailRoutePath;
    int m_editorPopRepairRequestId = 0;
    int m_detailPopRepairRequestId = 0;
};
