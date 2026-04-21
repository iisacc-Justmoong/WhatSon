# `src/app/models`

## Responsibility
`src/app/models` owns QObject-based app-domain helpers that sit between raw storage helpers and higher-level
viewmodels/QML surfaces.

## Current Domains
- `agenda`: agenda parsing and source-rewrite helpers used by the structured editor.
- `calendar`: calendar board state and system-calendar integration.
- `callout`: callout parsing and insertion/rewrite helpers for the structured editor.
- `content`: display, mobile-navigation, and structured-document helpers that coordinate QML-facing content surfaces.
- `editor`: parser, renderer, text, bridge, and session helpers that project RAW note source into editable views.
- `file`: persistent hub/note/resource storage, validation, import/export, sync, and hierarchy backends.
- `display`: paper/page/print presentation helpers that support editor view modes without living in the QML layer.
- `sensor`: hub inspection objects that derive lightweight read-side facts such as unused resource packages.

## Dependency Direction
- Model objects may depend on Qt core/gui primitives and lower file-domain helpers.
- They must not take ownership of LVRS view composition or sidebar/detail-panel routing concerns.
- Sensor-style models should prefer RAW hub files (`.wsnbody`, `.wsresource`, `resource.xml`) as their read source.
- `content/mobile` helpers should remain route/state coordinators only; they must not absorb sidebar or editor
  ownership that belongs to the viewmodel or RAW-source layers.
