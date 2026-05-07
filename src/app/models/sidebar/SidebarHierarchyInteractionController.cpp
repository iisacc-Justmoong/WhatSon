#include "app/models/sidebar/SidebarHierarchyInteractionController.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QJSValue>

#include <algorithm>

SidebarHierarchyInteractionController::SidebarHierarchyInteractionController(QObject* parent)
    : QObject(parent)
{
    m_expansionArmTimer.setInterval(5000);
    m_expansionArmTimer.setSingleShot(true);
    connect(&m_expansionArmTimer, &QTimer::timeout, this, &SidebarHierarchyInteractionController::clearArmedExpansionKey);

    m_activationBlockTimer.setInterval(160);
    m_activationBlockTimer.setSingleShot(true);
    connect(
        &m_activationBlockTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            m_expansionActivationSuppressed = false;
        });
}

SidebarHierarchyInteractionController::~SidebarHierarchyInteractionController() = default;

QObject* SidebarHierarchyInteractionController::hierarchyInteractionBridge() const noexcept
{
    return m_hierarchyInteractionBridge;
}

void SidebarHierarchyInteractionController::setHierarchyInteractionBridge(QObject* bridge)
{
    if (bridge != nullptr
        && !WhatSon::Policy::verifyDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("SidebarHierarchyInteractionController::setHierarchyInteractionBridge")))
    {
        return;
    }

    if (m_hierarchyInteractionBridge == bridge)
    {
        return;
    }

    m_hierarchyInteractionBridge = bridge;
    emit hierarchyInteractionBridgeChanged();
}

int SidebarHierarchyInteractionController::activeHierarchyIndex() const noexcept
{
    return m_activeHierarchyIndex;
}

void SidebarHierarchyInteractionController::setActiveHierarchyIndex(int index)
{
    const int normalizedIndex = std::max(0, index);
    if (m_activeHierarchyIndex == normalizedIndex)
    {
        return;
    }

    m_activeHierarchyIndex = normalizedIndex;
    emit activeHierarchyIndexChanged();
}

QString SidebarHierarchyInteractionController::armedExpansionKey() const
{
    return m_armedExpansionKey;
}

void SidebarHierarchyInteractionController::captureExpansionState(const QVariant& modelValue)
{
    const QVariantList nodes = normalizedVariantList(modelValue);
    bool changed = false;
    for (int index = 0; index < nodes.size(); ++index)
    {
        const QVariant& node = nodes.at(index);
        const QString key = itemExpansionKey(node, index);
        if (key.isEmpty() || m_expansionStateByKey.contains(key))
        {
            continue;
        }
        m_expansionStateByKey.insert(key, itemProperty(node, QStringLiteral("expanded")).toBool());
        changed = true;
    }
    if (changed)
    {
        emit expansionStateChanged();
    }
}

QVariantList SidebarHierarchyInteractionController::modelWithPreservedExpansion(const QVariant& modelValue) const
{
    const QVariantList nodes = normalizedVariantList(modelValue);
    QVariantList preserved;
    preserved.reserve(nodes.size());
    for (int index = 0; index < nodes.size(); ++index)
    {
        const QVariant& node = nodes.at(index);
        const QString key = itemExpansionKey(node, index);
        if (key.isEmpty() || !m_expansionStateByKey.contains(key))
        {
            preserved.push_back(node);
            continue;
        }

        QVariantMap copy = node.toMap();
        if (copy.isEmpty())
        {
            preserved.push_back(node);
            continue;
        }
        copy.insert(QStringLiteral("expanded"), m_expansionStateByKey.value(key));
        preserved.push_back(copy);
    }
    return preserved;
}

QString SidebarHierarchyInteractionController::itemExpansionKey(const QVariant& item, int fallbackIndex) const
{
    const QString itemKey = normalizedString(itemProperty(item, QStringLiteral("itemKey")));
    if (!itemKey.isEmpty())
    {
        return scopedExpansionKey(itemKey);
    }
    const QString key = normalizedString(itemProperty(item, QStringLiteral("key")));
    if (!key.isEmpty())
    {
        return scopedExpansionKey(key);
    }
    const QString resolvedItemKey = normalizedString(itemProperty(item, QStringLiteral("resolvedItemKey")));
    if (!resolvedItemKey.isEmpty())
    {
        return scopedExpansionKey(resolvedItemKey);
    }
    const QString uuid = normalizedString(itemProperty(item, QStringLiteral("uuid")));
    if (!uuid.isEmpty())
    {
        return scopedExpansionKey(QStringLiteral("folder:%1").arg(uuid));
    }
    const QString id = normalizedString(itemProperty(item, QStringLiteral("id")));
    if (!id.isEmpty())
    {
        return scopedExpansionKey(id);
    }

    int itemId = normalizedInteger(itemProperty(item, QStringLiteral("itemId")), -1);
    if (itemId < 0)
    {
        itemId = normalizedInteger(itemProperty(item, QStringLiteral("resolvedItemId")), -1);
    }
    if (itemId >= 0)
    {
        return scopedExpansionKey(QString::number(itemId));
    }

    return fallbackIndex >= 0 ? scopedExpansionKey(QString::number(fallbackIndex)) : QString();
}

bool SidebarHierarchyInteractionController::expansionStateContainsKey(const QString& key) const
{
    return m_expansionStateByKey.contains(key.trimmed());
}

bool SidebarHierarchyInteractionController::expansionStateForKey(const QString& key, bool fallbackValue) const
{
    const QString normalizedKey = key.trimmed();
    if (!m_expansionStateByKey.contains(normalizedKey))
    {
        return fallbackValue;
    }
    return m_expansionStateByKey.value(normalizedKey);
}

void SidebarHierarchyInteractionController::rememberExpansionState(const QString& key, bool expanded)
{
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty())
    {
        return;
    }

    const bool hadKey = m_expansionStateByKey.contains(normalizedKey);
    if (hadKey && m_expansionStateByKey.value(normalizedKey) == expanded)
    {
        return;
    }
    m_expansionStateByKey.insert(normalizedKey, expanded);
    emit expansionStateChanged();
}

bool SidebarHierarchyInteractionController::requestBulkExpansion(const QVariant& modelValue, bool expanded)
{
    const QHash<QString, bool> previousState = m_expansionStateByKey;
    setAllExpansionStates(modelValue, expanded);
    if (!invokeBridgeBoolMethod("setAllItemsExpanded", expanded))
    {
        m_expansionStateByKey = previousState;
        emit expansionStateChanged();
        return false;
    }

    return true;
}

QVariantMap SidebarHierarchyInteractionController::handleExpansionSignal(
    const QVariant& item,
    int resolvedIndex,
    bool expanded)
{
    const QString expansionKey = itemExpansionKey(item, resolvedIndex);
    const bool stateKnown = expansionStateContainsKey(expansionKey);
    const bool preservedExpanded = expansionStateForKey(expansionKey, expanded);
    const bool userExpansionArmed = !expansionKey.isEmpty() && expansionKey == m_armedExpansionKey;

    QVariantMap result{
        {QStringLiteral("accepted"), true},
        {QStringLiteral("committed"), false},
        {QStringLiteral("key"), expansionKey},
        {QStringLiteral("rollbackRequired"), false},
        {QStringLiteral("rollbackExpanded"), preservedExpanded}
    };

    if (!userExpansionArmed)
    {
        if (!stateKnown)
        {
            return commitExpansionChange(resolvedIndex, expansionKey, expanded, !expanded);
        }
        if (preservedExpanded == expanded)
        {
            return result;
        }
        return commitExpansionChange(resolvedIndex, expansionKey, expanded, preservedExpanded);
    }

    const bool previousExpanded = stateKnown ? preservedExpanded : !expanded;
    return commitExpansionChange(resolvedIndex, expansionKey, expanded, previousExpanded);
}

QVariantMap SidebarHierarchyInteractionController::requestChevronExpansion(
    int resolvedIndex,
    const QString& expansionKey,
    bool currentExpanded,
    const QString& expectedKey)
{
    const QString normalizedKey = expansionKey.trimmed();
    const QString normalizedExpectedKey = expectedKey.trimmed();
    QVariantMap rejected{
        {QStringLiteral("accepted"), false},
        {QStringLiteral("committed"), false},
        {QStringLiteral("key"), normalizedKey},
        {QStringLiteral("rollbackRequired"), false},
        {QStringLiteral("rollbackExpanded"), currentExpanded}
    };

    if (resolvedIndex < 0 || normalizedKey.isEmpty())
    {
        return rejected;
    }
    if (!normalizedExpectedKey.isEmpty() && normalizedExpectedKey != normalizedKey)
    {
        return rejected;
    }
    if (!normalizedExpectedKey.isEmpty() && m_armedExpansionKey.isEmpty())
    {
        return rejected;
    }
    if (!m_armedExpansionKey.isEmpty() && m_armedExpansionKey != normalizedKey)
    {
        return rejected;
    }

    return commitExpansionChange(resolvedIndex, normalizedKey, !currentExpanded, currentExpanded);
}

bool SidebarHierarchyInteractionController::armExpansionKey(const QString& key)
{
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty())
    {
        return false;
    }
    setArmedExpansionKey(normalizedKey);
    m_expansionArmTimer.start();
    return true;
}

void SidebarHierarchyInteractionController::clearArmedExpansionKey()
{
    setArmedExpansionKey(QString());
    m_expansionArmTimer.stop();
}

int SidebarHierarchyInteractionController::beginActivationAttempt()
{
    ++m_activationPendingSerial;
    return m_activationPendingSerial;
}

bool SidebarHierarchyInteractionController::activationAttemptCurrent(int serial) const
{
    return serial == m_activationPendingSerial;
}

bool SidebarHierarchyInteractionController::shouldSuppressActivation() const
{
    return m_expansionActivationSuppressed || m_activationBlockTimer.isActive();
}

QString SidebarHierarchyInteractionController::normalizedString(const QVariant& value)
{
    return value.toString().trimmed();
}

int SidebarHierarchyInteractionController::normalizedInteger(const QVariant& value, int fallbackValue)
{
    bool converted = false;
    const int parsed = value.toInt(&converted);
    return converted ? parsed : fallbackValue;
}

QVariant SidebarHierarchyInteractionController::itemProperty(const QVariant& item, const QString& propertyName) const
{
    const QVariantMap itemMap = item.toMap();
    if (itemMap.contains(propertyName))
    {
        return itemMap.value(propertyName);
    }

    QObject* object = qvariant_cast<QObject*>(item);
    if (object != nullptr)
    {
        return object->property(propertyName.toUtf8().constData());
    }

    if (item.canConvert<QJSValue>())
    {
        const QJSValue jsValue = item.value<QJSValue>();
        if (jsValue.hasProperty(propertyName))
        {
            return jsValue.property(propertyName).toVariant();
        }
    }

    return {};
}

QVariantList SidebarHierarchyInteractionController::normalizedVariantList(const QVariant& value) const
{
    if (value.canConvert<QVariantList>())
    {
        return value.toList();
    }

    if (value.canConvert<QJSValue>())
    {
        const QJSValue jsValue = value.value<QJSValue>();
        if (jsValue.isArray())
        {
            const quint32 length = jsValue.property(QStringLiteral("length")).toUInt();
            QVariantList items;
            items.reserve(static_cast<int>(length));
            for (quint32 index = 0; index < length; ++index)
            {
                items.push_back(jsValue.property(index).toVariant());
            }
            return items;
        }
    }

    return {};
}

QString SidebarHierarchyInteractionController::scopedExpansionKey(const QString& key) const
{
    const QString normalizedKey = key.trimmed();
    if (normalizedKey.isEmpty())
    {
        return {};
    }
    return QStringLiteral("hierarchy:%1:%2").arg(m_activeHierarchyIndex).arg(normalizedKey);
}

bool SidebarHierarchyInteractionController::invokeBridgeBoolMethod(const char* methodName, bool argument) const
{
    if (m_hierarchyInteractionBridge == nullptr)
    {
        return false;
    }

    bool handled = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_hierarchyInteractionBridge.data(),
        methodName,
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, handled),
        Q_ARG(bool, argument));
    return invoked && handled;
}

QVariantMap SidebarHierarchyInteractionController::commitExpansionChange(
    int index,
    const QString& expansionKey,
    bool expanded,
    bool previousExpanded)
{
    const QString normalizedKey = expansionKey.trimmed();
    QVariantMap result{
        {QStringLiteral("accepted"), false},
        {QStringLiteral("committed"), false},
        {QStringLiteral("key"), normalizedKey},
        {QStringLiteral("rollbackRequired"), false},
        {QStringLiteral("rollbackExpanded"), previousExpanded}
    };

    if (m_hierarchyInteractionBridge == nullptr || index < 0 || normalizedKey.isEmpty())
    {
        result.insert(QStringLiteral("rollbackRequired"), true);
        return result;
    }

    clearArmedExpansionKey();
    rememberExpansionState(normalizedKey, expanded);
    suppressActivationForExpansion();

    bool handled = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_hierarchyInteractionBridge.data(),
        "setItemExpanded",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, handled),
        Q_ARG(int, index),
        Q_ARG(bool, expanded));

    if (!invoked || !handled)
    {
        rememberExpansionState(normalizedKey, previousExpanded);
        result.insert(QStringLiteral("rollbackRequired"), true);
        return result;
    }

    result.insert(QStringLiteral("accepted"), true);
    result.insert(QStringLiteral("committed"), true);
    return result;
}

void SidebarHierarchyInteractionController::setAllExpansionStates(const QVariant& modelValue, bool expanded)
{
    const QVariantList nodes = normalizedVariantList(modelValue);
    bool changed = false;
    for (int index = 0; index < nodes.size(); ++index)
    {
        const QVariant& node = nodes.at(index);
        if (!itemProperty(node, QStringLiteral("showChevron")).toBool())
        {
            continue;
        }

        const QString key = itemExpansionKey(node, index);
        if (key.isEmpty())
        {
            continue;
        }
        if (m_expansionStateByKey.contains(key) && m_expansionStateByKey.value(key) == expanded)
        {
            continue;
        }
        m_expansionStateByKey.insert(key, expanded);
        changed = true;
    }
    if (changed)
    {
        emit expansionStateChanged();
    }
}

void SidebarHierarchyInteractionController::setArmedExpansionKey(const QString& key)
{
    const QString normalizedKey = key.trimmed();
    if (m_armedExpansionKey == normalizedKey)
    {
        return;
    }
    m_armedExpansionKey = normalizedKey;
    emit armedExpansionKeyChanged();
}

void SidebarHierarchyInteractionController::suppressActivationForExpansion()
{
    ++m_activationPendingSerial;
    m_expansionActivationSuppressed = true;
    m_activationBlockTimer.start();
}
