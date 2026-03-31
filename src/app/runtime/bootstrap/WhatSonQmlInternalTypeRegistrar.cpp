#include "WhatSonQmlInternalTypeRegistrar.hpp"

#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
#include "viewmodel/panel/FocusedNoteDeletionBridge.hpp"
#include "viewmodel/panel/HierarchyDragDropBridge.hpp"
#include "viewmodel/panel/HierarchyInteractionBridge.hpp"
#include "viewmodel/panel/NoteListModelContractBridge.hpp"

#include <qqml.h>

namespace WhatSon::Runtime::Bootstrap
{
    void registerInternalQmlTypes()
    {
        qmlRegisterType<ContentsEditorSelectionBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsEditorSelectionBridge");
        qmlRegisterType<ContentsLogicalTextBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsLogicalTextBridge");
        qmlRegisterType<ContentsGutterMarkerBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsGutterMarkerBridge");
        qmlRegisterType<FocusedNoteDeletionBridge>(
            "WhatSon.App.Internal", 1, 0, "FocusedNoteDeletionBridge");
        qmlRegisterType<NoteListModelContractBridge>(
            "WhatSon.App.Internal", 1, 0, "NoteListModelContractBridge");
        qmlRegisterType<HierarchyDragDropBridge>(
            "WhatSon.App.Internal", 1, 0, "HierarchyDragDropBridge");
        qmlRegisterType<HierarchyInteractionBridge>(
            "WhatSon.App.Internal", 1, 0, "HierarchyInteractionBridge");
    }
} // namespace WhatSon::Runtime::Bootstrap
