# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

`bindWorkspaceContextObjects(...)` builds the LVRS QML context binding plan for workspace runtime objects.

The binding list now excludes the deleted active editor document session, editor paste bridge, and native editor input command filter. Remaining bindings cover hierarchy controllers, detail panel controllers, navigation state, clipboard import state, calendar controllers, async scheduling, and panel controller registry.
