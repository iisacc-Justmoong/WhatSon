#include "app/models/file/hub/WhatSonHubPlacement.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <utility>

WhatSonHubPlacement::WhatSonHubPlacement(QString hubPath, const double x, const double y)
    : m_hubPath(std::move(hubPath))
      , m_x(x)
      , m_y(y)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement.object"),
                              QStringLiteral("ctor"),
                              QStringLiteral("path=%1 x=%2 y=%3").arg(m_hubPath).arg(m_x).arg(m_y));
}

WhatSonHubPlacement::~WhatSonHubPlacement() = default;

const QString& WhatSonHubPlacement::hubPath() const noexcept
{
    return m_hubPath;
}

void WhatSonHubPlacement::setHubPath(QString hubPath)
{
    m_hubPath = std::move(hubPath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement.object"),
                              QStringLiteral("setHubPath"),
                              QStringLiteral("value=%1").arg(m_hubPath));
}

double WhatSonHubPlacement::x() const noexcept
{
    return m_x;
}

void WhatSonHubPlacement::setX(const double x)
{
    m_x = x;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement.object"),
                              QStringLiteral("setX"),
                              QStringLiteral("value=%1").arg(m_x));
}

double WhatSonHubPlacement::y() const noexcept
{
    return m_y;
}

void WhatSonHubPlacement::setY(const double y)
{
    m_y = y;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement.object"),
                              QStringLiteral("setY"),
                              QStringLiteral("value=%1").arg(m_y));
}
