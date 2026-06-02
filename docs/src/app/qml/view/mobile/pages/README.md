# Mobile Pages

Mobile workspace pages compose existing view surfaces without owning backend state.

`MobileHierarchyPage.qml` routes the editor area through `ContentViewLayout.qml`. The active editor document model was deleted, so mobile pages must not recreate or forward document session, editor paste, or native key-filter objects.
