#include "LibraryDraft.hpp"
#include "WhatSonDebugTrace.hpp"

LibraryDraft::LibraryDraft()
{
    WhatSon::Debug::trace(QStringLiteral("library.draft"), QStringLiteral("ctor"));
}

LibraryDraft::~LibraryDraft()
{
    WhatSon::Debug::trace(QStringLiteral("library.draft"), QStringLiteral("dtor"));
}
