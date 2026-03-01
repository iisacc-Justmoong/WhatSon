#include "LibraryAll.hpp"
#include "WhatSonDebugTrace.hpp"

LibraryAll::LibraryAll()
{
    WhatSon::Debug::trace(QStringLiteral("library.all"), QStringLiteral("ctor"));
}

LibraryAll::~LibraryAll()
{
    WhatSon::Debug::trace(QStringLiteral("library.all"), QStringLiteral("dtor"));
}
