#pragma once

#include "SidebarHierarchyLvrsSupport.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class SidebarHierarchyLvrsAdapter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QObject
        * hierarchyViewModel READ hierarchyViewModel WRITE setHierarchyViewModel NOTIFY hierarchyViewModelChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QVariantList nodes READ nodes NOTIFY nodesChanged)
    Q_PROPERTY(QVariantList flatNodes READ flatNodes NOTIFY nodesChanged)
    Q_PROPERTY(QString selectedItemKey READ selectedItemKey NOTIFY selectedItemKeyChanged)
    Q_PROPERTY(bool editable READ editable NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool noteDropEnabled READ noteDropEnabled NOTIFY capabilitiesChanged)

public:
    explicit SidebarHierarchyLvrsAdapter(QObject* parent = nullptr);
    ~SidebarHierarchyLvrsAdapter() override;

    QObject* hierarchyViewModel() const noexcept;
    void setHierarchyViewModel(QObject* viewModel);

    QString searchQuery() const;
    void setSearchQuery(QString query);

    QVariantList nodes() const;
    QVariantList flatNodes() const;
    QString selectedItemKey() const;
    bool editable() const noexcept;
    bool noteDropEnabled() const noexcept;

    Q_INVOKABLE void activateKey(const QString& itemKey);
    Q_INVOKABLE bool canRenameKey(const QString& itemKey) const;
    Q_INVOKABLE QString labelForKey(const QString& itemKey) const;
    Q_INVOKABLE bool renameKey(const QString& itemKey, const QString& displayName);
    Q_INVOKABLE bool commitEditableNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey);
    Q_INVOKABLE bool canAcceptNoteDrop(const QString& itemKey, const QString& noteId) const;
    Q_INVOKABLE bool assignNoteToKey(const QString& itemKey, const QString& noteId);

    signals  :


    void hierarchyViewModelChanged();
    void searchQueryChanged();
    void nodesChanged();
    void selectedItemKeyChanged();
    void capabilitiesChanged();

private:
    void connectViewModelSignals();
    void disconnectViewModelSignals();
    void rebuild();
    int sourceIndexForKey(const QString& itemKey) const;
    bool invokeBooleanMethod(const char* methodName, const QVariantList& nodes, const QString& activeItemKey) const;
    bool invokeBooleanMethod(const char* methodName, int index, const QString& text = QString()) const;

private
    slots  :


    void rebuildFromViewModelSignal();

private:
    QPointer<QObject> m_hierarchyViewModel;
    QVector<QMetaObject::Connection> m_connections;
    QString m_searchQuery;
    QVariantList m_nodes;
    QVariantList m_flatNodes;
    QString m_selectedItemKey;
    bool m_editable = false;
    bool m_noteDropEnabled = false;
    QHash<QString, int> m_keyToSourceIndex;
    QHash<QString, QString> m_keyToLabel;
};
