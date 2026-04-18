# `src/app/viewmodel/detailPanel/ResourceDetailPanelViewModel.cpp`

## Runtime Behavior
- Accepts a resource list model only through `setCurrentResourceListModel(QObject*)`.
- Treats the context as linked only when the injected object is a `ResourcesListModel`.
- Mirrors `ResourcesListModel::currentResourceEntry()` as the exported `currentResourceEntry` snapshot.
- Re-emits entry changes when the active resource row changes and clears the snapshot when the resource list model is
  removed or destroyed.
