#include "LibraryToday.hpp"
#include "WhatSonDebugTrace.hpp"

LibraryToday::LibraryToday()
{
    WhatSon::Debug::trace(QStringLiteral("library.today"), QStringLiteral("ctor"));
}

LibraryToday::~LibraryToday()
{
    WhatSon::Debug::trace(QStringLiteral("library.today"), QStringLiteral("dtor"));
}
