#pragma once

#include "app/models/detailPanel/DetailPanelState.hpp"

#include <QVariantList>

namespace WhatSon::DetailPanel
{
    QVariantList buildToolbarItems(ContentState activeState);
} // namespace WhatSon::DetailPanel
