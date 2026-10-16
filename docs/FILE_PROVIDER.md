# File creation ownership in WhatSon

Clipboard resources, hub/trial files and hierarchy-state directories use provider devices. Domain signal/slot, resource-package and workspace contracts remain unchanged.

The common [ownership contract](../../../../SDK/iiFileProvider/docs/FILE_OWNERSHIP.md) defines stream, staging, control-file and
installed-package rules. Source regression tests remain with this module; the
workspace creation audit also checks these boundaries. Build/deployment and target
runtime validation are separate evidence.

Changed boundaries:


- `src/app/models/clipboard/InAppClipboardManager.cpp`
- `src/app/models/file/hub/WhatSonHubCreator.cpp`
- `src/app/models/hierarchy/event/WhatSonEventHierarchyStore.cpp`
- `src/app/models/hierarchy/folders/WhatSonFoldersHierarchyStore.cpp`
- `src/app/models/hierarchy/preset/WhatSonPresetHierarchyStore.cpp`
- `src/app/models/hierarchy/progress/WhatSonProgressHierarchyStore.cpp`
- `src/app/models/hierarchy/projects/WhatSonProjectsHierarchyStore.cpp`
- `src/app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp`
- `src/app/models/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp`
- `src/app/models/hierarchy/tags/WhatSonTagsHierarchyStore.cpp`
- `src/extension/trial/WhatSonTrialRegisterXml.cpp`
