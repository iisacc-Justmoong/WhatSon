#pragma once

#include <QObject>
#include <QString>

class PanelViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString panelKey READ panelKey CONSTANT)
    Q_PROPERTY(int hookRequestCount READ hookRequestCount NOTIFY hookRequestCountChanged)

public:
    explicit PanelViewModel(QString panelKey, QObject* parent = nullptr);

    QString panelKey() const;
    int hookRequestCount() const noexcept;

public
    slots  :



    void requestViewModelHook();

    signals  :



    void hookRequestCountChanged();
    void viewModelHookRequested();

private:
    QString m_panelKey;
    int m_hookRequestCount = 0;
};
