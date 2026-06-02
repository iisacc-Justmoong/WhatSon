# `src/app/qml/view/panels/BodyLayout.qml`

`BodyLayout.qml` arranges the desktop workspace body: hierarchy sidebar, list/detail panels, and the central content surface.

It forwards hierarchy, note-list, clipboard import state, editor font provider, calendar controllers, and panel registry objects into `ContentViewLayout.qml`.

The deleted editor document session, editor paste bridge, and native input command filter are no longer properties of this layout and must not be threaded through this shell.
