# `src/app/qml/view/panels/ContentEditorToolbar.qml`

`ContentEditorToolbar.qml` is a legacy toolbar surface.

The current contents layout does not mount this toolbar because the active editor document command surface was deleted. If this toolbar is reintroduced later, it must receive a newly defined document model contract rather than binding directly to QML-owned parsing or source mutation.
