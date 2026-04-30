#pragma once

#include <QObject>
#include <QString>

class PanelController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString panelKey READ panelKey CONSTANT)
    Q_PROPERTY(int hookRequestCount READ hookRequestCount NOTIFY hookRequestCountChanged)

public:
    explicit PanelController(QString panelKey, QObject* parent = nullptr);

    QString panelKey() const;
    int hookRequestCount() const noexcept;

public
    slots  :


    Q_INVOKABLE
    void requestControllerHook(const QString& reason = QString());

signals  :



    void hookRequestCountChanged();
    void controllerHookRequested(const QString& reason);

private:
    QString m_panelKey;
    int m_hookRequestCount = 0;
};
