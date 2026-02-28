#include "WhatSonHubPlacement.hpp"

#include <utility>

WhatSonHubPlacement::WhatSonHubPlacement(QString hubPath, const double x, const double y)
    : m_hubPath(std::move(hubPath))
      , m_x(x)
      , m_y(y)
{
}

WhatSonHubPlacement::~WhatSonHubPlacement() = default;

const QString& WhatSonHubPlacement::hubPath() const noexcept
{
    return m_hubPath;
}

void WhatSonHubPlacement::setHubPath(QString hubPath)
{
    m_hubPath = std::move(hubPath);
}

double WhatSonHubPlacement::x() const noexcept
{
    return m_x;
}

void WhatSonHubPlacement::setX(const double x)
{
    m_x = x;
}

double WhatSonHubPlacement::y() const noexcept
{
    return m_y;
}

void WhatSonHubPlacement::setY(const double y)
{
    m_y = y;
}
