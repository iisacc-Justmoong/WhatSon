#pragma once

#include <QByteArray>
#include <QString>

class ISelectedHubStore
{
public:
    virtual ~ISelectedHubStore() = default;

    [[nodiscard]] virtual QString selectedHubPath() = 0;
    [[nodiscard]] virtual QByteArray selectedHubAccessBookmark() = 0;
    [[nodiscard]] virtual QString startupHubPath(const QString& blueprintFallbackHubPath) = 0;
    virtual void clearSelectedHubPath() = 0;
    virtual void setSelectedHubPath(const QString& hubPath) = 0;
    virtual void setSelectedHubSelection(const QString& hubPath, const QByteArray& accessBookmark) = 0;
};
