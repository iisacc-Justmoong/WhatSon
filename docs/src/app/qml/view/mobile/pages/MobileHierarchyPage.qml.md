# `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`

`MobileHierarchyPage.qml` provides the compact workspace page and mounts `ContentViewLayout.qml` for the mobile editor route.

The page forwards navigation, hierarchy, resource, note-list, calendar, clipboard import, and panel registry state needed by the current view surface.

It no longer forwards a document session, editor paste bridge, or native input command filter.
