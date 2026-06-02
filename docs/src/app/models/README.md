# `src/app/models`

The model tree owns C++ state, controllers, persistence helpers, and parser/mutation code.

The active editor document model was deleted. QML must not replace it with backend logic, and model shards should not expose compatibility wrappers for the removed editor session, paste bridge, or native key filter.
