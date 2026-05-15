# `src/app/models/file/sync/WhatSonHubSyncScheduler.cpp`

## Role
Implements the hub sync scheduling policy.

## Behavior
- Uses a default periodic interval of 5000 ms.
- Uses a default debounce interval of 350 ms.
- Periodic ticks request the same debounced sync check as watcher hints.
- A non-positive debounce interval schedules the check on the next event-loop turn.

## Boundary
- The scheduler never inspects the hub and never invokes runtime reload callbacks.
