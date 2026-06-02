# App Architecture

WhatSon is a Qt Quick and LVRS application with C++ model/controller ownership and QML view composition.

## Current Editor Boundary

The previous active note document session model was removed. The workspace content surface keeps a blank read-only LVRS text editor shell so gutter and minimap layout code can remain mounted, but active note body parsing, projection, formatting shortcuts, paste mutation, and editor persistence are no longer wired into the contents route.

Image resource selection still routes through the resource list model into `ImageEditor.qml`.

## QML Rule

QML remains view-only. It must not parse `.wsnbody`, mutate RAW source, own editor persistence, or recreate deleted document-session behavior through wrapper objects.
