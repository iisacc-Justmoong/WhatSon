#pragma once

#include "app/models/panel/PanelController.hpp"

#include <QHash>
#include <QObject>
#include <QStringList>

class PanelControllerRegistry final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList panelKeys READ panelKeys CONSTANT)
    Q_PROPERTY(int panelCount READ panelCount NOTIFY panelCountChanged)

public:
    explicit PanelControllerRegistry(QObject* parent = nullptr);

    QStringList panelKeys() const;
    int panelCount() const noexcept;

    Q_INVOKABLE QObject* panelController(const QString& panelKey) const;
    Q_INVOKABLE bool containsPanel(const QString& panelKey) const;

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

    signals  :



    void panelCountChanged();
    void controllerHookRequested();

private:
    QHash<QString, PanelController*> m_panelControllers;
    QStringList m_panelKeys;
};
