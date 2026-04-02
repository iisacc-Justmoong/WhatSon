# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

## Responsibility
Implements root context-property binding for the workspace runtime graph.

## Bound Properties
The implementation binds all existing workspace properties used by `Main.qml` and panel/view
modules, including hierarchy viewmodels, calendar stores/viewmodels (`agendaViewModel`, day/week/month/year),
scheduler, and panel registry.

## Test Coverage

Automated test files were removed from this repository; confirm bound property keys by inspecting runtime context wiring logs.
