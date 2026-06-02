# `src/app/main.cpp`

The application entrypoint creates core runtime objects, configures LVRS, wires controllers, binds workspace context objects, and starts the Qt Quick shell.

The active editor document session, editor paste bridge, and native editor input filter are no longer constructed or exported to QML.
