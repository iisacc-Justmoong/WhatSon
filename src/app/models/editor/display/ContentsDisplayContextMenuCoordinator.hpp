#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplayContextMenuCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool structuredDocumentFlowVisible READ structuredDocumentFlowVisible WRITE setStructuredDocumentFlowVisible NOTIFY structuredDocumentFlowVisibleChanged)
    Q_PROPERTY(int structuredContextMenuBlockIndex READ structuredContextMenuBlockIndex WRITE setStructuredContextMenuBlockIndex NOTIFY structuredContextMenuBlockIndexChanged)
    Q_PROPERTY(QVariantMap structuredContextMenuSelectionSnapshot READ structuredContextMenuSelectionSnapshot WRITE setStructuredContextMenuSelectionSnapshot NOTIFY structuredContextMenuSelectionSnapshotChanged)

public:
    explicit ContentsDisplayContextMenuCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayContextMenuCoordinator() override;

    bool structuredDocumentFlowVisible() const noexcept;
    void setStructuredDocumentFlowVisible(bool value);

    int structuredContextMenuBlockIndex() const noexcept;
    void setStructuredContextMenuBlockIndex(int value);

    QVariantMap structuredContextMenuSelectionSnapshot() const;
    void setStructuredContextMenuSelectionSnapshot(const QVariantMap& value);

    Q_INVOKABLE QVariantMap normalizeStructuredSelectionSnapshot(const QVariantMap& snapshot) const;
    Q_INVOKABLE bool structuredSelectionValid() const;
    Q_INVOKABLE QString inlineStyleTagForEvent(const QString& eventName) const;
    Q_INVOKABLE QVariantMap primeStructuredSelectionSnapshotPlan(const QVariantMap& targetState) const;
    Q_INVOKABLE QVariantMap openSelectionContextMenuPlan(bool structuredSelectionValid, bool hasContextMenu, double localX, double localY) const;
    Q_INVOKABLE QVariantMap handleStructuredSelectionEventPlan(const QString& inlineStyleTag, bool structuredSelectionValid, bool canApplyInlineFormat) const;

signals:
    void structuredDocumentFlowVisibleChanged();
    void structuredContextMenuBlockIndexChanged();
    void structuredContextMenuSelectionSnapshotChanged();

private:
    bool m_structuredDocumentFlowVisible = false;
    int m_structuredContextMenuBlockIndex = 0;
    QVariantMap m_structuredContextMenuSelectionSnapshot;
};
