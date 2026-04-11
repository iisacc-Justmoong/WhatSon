#include "WhatSonQmlInternalTypeRegistrar.hpp"

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"
#include "editor/renderer/ContentsPagePrintLayoutRenderer.hpp"
#include "editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "editor/renderer/ContentsTextFormatRenderer.hpp"
#include "file/validator/ContentsStructuredTagValidator.hpp"
#include "file/viewer/ContentsBodyResourceRenderer.hpp"
#include "file/viewer/ResourceBitmapViewer.hpp"
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
        qmlRegisterType<ContentsTextFormatRenderer>(
            "WhatSon.App.Internal", 1, 0, "ContentsTextFormatRenderer");
        qmlRegisterType<ContentsStructuredBlockRenderer>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredBlockRenderer");
        qmlRegisterType<ContentsStructuredTagValidator>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredTagValidator");
        qmlRegisterType<ContentsAgendaBackend>(
            "WhatSon.App.Internal", 1, 0, "ContentsAgendaBackend");
        qmlRegisterType<ContentsCalloutBackend>(
            "WhatSon.App.Internal", 1, 0, "ContentsCalloutBackend");
        qmlRegisterType<ContentsPagePrintLayoutRenderer>(
            "WhatSon.App.Internal", 1, 0, "ContentsPagePrintLayoutRenderer");
        qmlRegisterType<ContentsBodyResourceRenderer>(
            "WhatSon.App.Internal", 1, 0, "ContentsBodyResourceRenderer");
        qmlRegisterType<ResourceBitmapViewer>(
            "WhatSon.App.Internal", 1, 0, "ResourceBitmapViewer");
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
