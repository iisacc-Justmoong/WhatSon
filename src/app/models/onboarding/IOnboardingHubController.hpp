#pragma once

#include <QObject>
#include <QString>

class IOnboardingHubController : public QObject
{
    Q_OBJECT

public:
    explicit IOnboardingHubController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IOnboardingHubController() override = default;

    Q_INVOKABLE virtual void beginWorkspaceTransition() = 0;
    Q_INVOKABLE virtual void completeWorkspaceTransition() = 0;
    Q_INVOKABLE virtual void failWorkspaceTransition(const QString& message) = 0;
};
