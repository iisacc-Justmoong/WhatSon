#pragma once

#include "app/models/file/sync/WhatSonHubSyncObservation.hpp"

#include <QString>

class WhatSonHubSyncObservationBuilder final
{
public:
    [[nodiscard]] WhatSonHubSyncObservation inspectHub(const QString& hubPath) const;
};
