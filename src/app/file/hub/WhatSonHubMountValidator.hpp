#pragma once

#include <QByteArray>
#include <QString>

struct WhatSonHubMountValidation final
{
    bool mounted = false;
    QString hubPath;
    QString failureMessage;
};

class WhatSonHubMountValidator final
{
public:
    [[nodiscard]] WhatSonHubMountValidation resolveMountedHub(
        const QString& hubPath,
        const QByteArray& hubAccessBookmark = {}) const;
};
