#pragma once

#include "app/viewmodel/detailPanel/DetailPanelState.hpp"

#include <QVariantList>

namespace WhatSon::DetailPanel
{
    QVariantList buildToolbarItems(ContentState activeState);
} // namespace WhatSon::DetailPanel
