#pragma once

#include <QString>

class WhatSonHubPlacement
{
public:
    WhatSonHubPlacement() = default;
    WhatSonHubPlacement(QString hubPath, double x, double y);
    ~WhatSonHubPlacement();

    WhatSonHubPlacement(const WhatSonHubPlacement&) = default;
    WhatSonHubPlacement& operator=(const WhatSonHubPlacement&) = default;
    WhatSonHubPlacement(WhatSonHubPlacement&&) = default;
    WhatSonHubPlacement& operator=(WhatSonHubPlacement&&) = default;

    const QString& hubPath() const noexcept;
    void setHubPath(QString hubPath);

    double x() const noexcept;
    void setX(double x);

    double y() const noexcept;
    void setY(double y);

private:
    QString m_hubPath;
    double m_x = 0.0;
    double m_y = 0.0;
};
