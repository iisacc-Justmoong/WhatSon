#pragma once

#include "PanelViewModel.hpp"

#include <QHash>
#include <QObject>
#include <QStringList>

class PanelViewModelRegistry final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList panelKeys READ panelKeys CONSTANT)
    Q_PROPERTY(int panelCount READ panelCount NOTIFY panelCountChanged)

public:
    explicit PanelViewModelRegistry(QObject* parent = nullptr);

    QStringList panelKeys() const;
    int panelCount() const noexcept;

    Q_INVOKABLE QObject* panelViewModel(const QString& panelKey) const;
    Q_INVOKABLE bool containsPanel(const QString& panelKey) const;

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void panelCountChanged();
    void viewModelHookRequested();

private:
    QHash<QString, PanelViewModel*> m_panelViewModels;
    QStringList m_panelKeys;
};
