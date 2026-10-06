# `src/app/models/sensor`

## Responsibility
Owns read-side hub inspection objects that derive sensor outputs from the unpacked `.wshub` filesystem layout.

## Scope
- Source directory: `src/app/models/sensor`
- Child files:
  - `MonthlyUnusedNote.hpp`
  - `MonthlyUnusedNote.cpp`
  - `UnusedNoteSensorSupport.hpp`
  - `UnusedNoteSensorSupport.cpp`
  - `UnusedResourcesSensor.hpp`
  - `UnusedResourcesSensor.cpp`
  - `WeeklyUnusedNote.hpp`
  - `WeeklyUnusedNote.cpp`

## Current Contract
- `UnusedNoteSensorSupport` validates the hub path but no longer scans note packages while the note package model is
  deleted.
- `WeeklyUnusedNote` and `MonthlyUnusedNote` currently return empty note result lists.
- Both note sensors keep their public properties so callers do not need a view contract change during the package-model
  removal.
- `UnusedResourcesSensor` scans hub-local `.wsresource` packages and returns package inventory as unused candidates.
- The sensor no longer reads note body source or editor-side DOM projections.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/sensor``  ( `docs/src/app/models/sensor/README.md` )
- Location:  `docs/src/app/models/sensor`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
