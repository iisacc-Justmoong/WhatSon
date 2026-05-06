#pragma once

#include <QHash>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class SidebarHierarchyInteractionController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* hierarchyInteractionBridge READ hierarchyInteractionBridge WRITE setHierarchyInteractionBridge
                   NOTIFY hierarchyInteractionBridgeChanged)
    Q_PROPERTY(int activeHierarchyIndex READ activeHierarchyIndex WRITE setActiveHierarchyIndex
                   NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(QString armedExpansionKey READ armedExpansionKey NOTIFY armedExpansionKeyChanged)

public:
    explicit SidebarHierarchyInteractionController(QObject* parent = nullptr);
    ~SidebarHierarchyInteractionController() override;

    QObject* hierarchyInteractionBridge() const noexcept;
    void setHierarchyInteractionBridge(QObject* bridge);

    int activeHierarchyIndex() const noexcept;
    void setActiveHierarchyIndex(int index);

    QString armedExpansionKey() const;

    Q_INVOKABLE QString footerActionName(int index, const QString& eventName) const;
    Q_INVOKABLE bool requestFooterAction(const QString& action);

    Q_INVOKABLE void captureExpansionState(const QVariant& modelValue);
    Q_INVOKABLE QVariantList modelWithPreservedExpansion(const QVariant& modelValue) const;
    Q_INVOKABLE QString itemExpansionKey(const QVariant& item, int fallbackIndex) const;
    Q_INVOKABLE bool expansionStateContainsKey(const QString& key) const;
    Q_INVOKABLE bool expansionStateForKey(const QString& key, bool fallbackValue) const;
    Q_INVOKABLE void rememberExpansionState(const QString& key, bool expanded);
    Q_INVOKABLE bool requestBulkExpansion(const QVariant& modelValue, bool expanded);

    Q_INVOKABLE QVariantMap handleExpansionSignal(const QVariant& item, int resolvedIndex, bool expanded);
    Q_INVOKABLE QVariantMap requestChevronExpansion(
        int resolvedIndex,
        const QString& expansionKey,
        bool currentExpanded,
        const QString& expectedKey);
    Q_INVOKABLE bool armExpansionKey(const QString& key);
    Q_INVOKABLE void clearArmedExpansionKey();

    Q_INVOKABLE int beginActivationAttempt();
    Q_INVOKABLE bool activationAttemptCurrent(int serial) const;
    Q_INVOKABLE bool shouldSuppressActivation() const;

signals:
    void hierarchyInteractionBridgeChanged();
    void activeHierarchyIndexChanged();
    void armedExpansionKeyChanged();
    void expansionStateChanged();
    void footerCreateRequested();
    void footerDeleteRequested();
    void footerOptionsRequested();
    void selectedHierarchySyncRequested(bool focusView);

private:
    static QString normalizedString(const QVariant& value);
    static int normalizedInteger(const QVariant& value, int fallbackValue);

    QVariant itemProperty(const QVariant& item, const QString& propertyName) const;
    QVariantList normalizedVariantList(const QVariant& value) const;
    QString scopedExpansionKey(const QString& key) const;
    bool bridgeBoolProperty(const char* propertyName, bool fallbackValue) const;
    bool invokeBridgeBoolMethod(const char* methodName, bool argument) const;
    QVariantMap commitExpansionChange(int index, const QString& expansionKey, bool expanded, bool previousExpanded);
    void setAllExpansionStates(const QVariant& modelValue, bool expanded);
    void setArmedExpansionKey(const QString& key);
    void suppressActivationForExpansion();

    QPointer<QObject> m_hierarchyInteractionBridge;
    int m_activeHierarchyIndex = 0;
    QString m_armedExpansionKey;
    QString m_queuedFooterAction;
    QHash<QString, bool> m_expansionStateByKey;
    QTimer m_expansionArmTimer;
    QTimer m_activationBlockTimer;
    int m_activationPendingSerial = 0;
    bool m_expansionActivationSuppressed = false;
};
