#include "app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"

#include "app/models/agenda/ContentsAgendaBackend.hpp"
#include "app/models/callout/ContentsCalloutBackend.hpp"
#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "app/models/display/paper/ContentsA4PaperBackground.hpp"
#include "app/models/display/paper/ContentsPaperSelection.hpp"
#include "app/models/display/paper/ContentsTextFormatRenderer.hpp"
#include "app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "app/models/file/validator/ContentsStructuredTagValidator.hpp"
#include "app/platform/Apple/WhatSonIosHubPickerBridge.hpp"
#include "app/models/file/viewer/ContentsBodyResourceRenderer.hpp"
#include "app/models/file/viewer/ResourceBitmapViewer.hpp"
#include "app/models/content/display/ContentsDisplayContextMenuCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayGutterCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayMinimapCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyBackSwipeCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyCanonicalRoutePlanner.hpp"
#include "app/models/content/mobile/MobileHierarchyNavigationCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyPopRepairPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteSelectionSyncPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "app/models/content/mobile/MobileHierarchySelectionCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayPresentationRefreshController.hpp"
#include "app/models/content/display/ContentsDisplayRefreshCoordinator.hpp"
#include "app/models/content/display/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "app/models/content/display/ContentsDisplaySessionCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayStructuredFlowCoordinator.hpp"
#include "app/models/content/display/ContentsDisplayViewportCoordinator.hpp"
#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "app/models/editor/session/ContentsEditorSessionController.hpp"
#include "app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"
#include "app/models/editor/bridge/ContentsGutterMarkerBridge.hpp"
#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"
#include "app/models/editor/resource/ContentsResourceTagTextGenerator.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentBlocksModel.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentFocusPolicy.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentHost.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentMutationPolicy.hpp"
#include "app/viewmodel/panel/FocusedNoteDeletionBridge.hpp"
#include "app/viewmodel/panel/HierarchyDragDropBridge.hpp"
#include "app/viewmodel/panel/HierarchyInteractionBridge.hpp"
#include "app/viewmodel/panel/NoteListModelContractBridge.hpp"

#include <qqml.h>

namespace WhatSon::Runtime::Bootstrap
{
    void registerInternalQmlTypes()
    {
        qmlRegisterType<ContentsEditorSelectionBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsEditorSelectionBridge");
        qmlRegisterType<ContentsDisplaySelectionSyncCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplaySelectionSyncCoordinator");
        qmlRegisterType<ContentsDisplayPresentationRefreshController>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayPresentationRefreshController");
        qmlRegisterType<ContentsDisplayRefreshCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayRefreshCoordinator");
        qmlRegisterType<ContentsDisplayContextMenuCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayContextMenuCoordinator");
        qmlRegisterType<ContentsDisplayGutterCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayGutterCoordinator");
        qmlRegisterType<ContentsDisplayMinimapCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayMinimapCoordinator");
        qmlRegisterType<ContentsDisplayNoteBodyMountCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayNoteBodyMountCoordinator");
        qmlRegisterType<MobileHierarchyBackSwipeCoordinator>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyBackSwipeCoordinator");
        qmlRegisterType<MobileHierarchyCanonicalRoutePlanner>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyCanonicalRoutePlanner");
        qmlRegisterType<MobileHierarchyNavigationCoordinator>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyNavigationCoordinator");
        qmlRegisterType<MobileHierarchyPopRepairPolicy>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyPopRepairPolicy");
        qmlRegisterType<MobileHierarchyRouteSelectionSyncPolicy>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyRouteSelectionSyncPolicy");
        qmlRegisterType<MobileHierarchyRouteStateStore>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchyRouteStateStore");
        qmlRegisterType<MobileHierarchySelectionCoordinator>(
            "WhatSon.App.Internal", 1, 0, "MobileHierarchySelectionCoordinator");
        qmlRegisterType<ContentsDisplaySessionCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplaySessionCoordinator");
        qmlRegisterType<ContentsDisplayViewportCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayViewportCoordinator");
        qmlRegisterType<ContentsDisplayStructuredFlowCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayStructuredFlowCoordinator");
        qmlRegisterType<ContentsEditorPresentationProjection>(
            "WhatSon.App.Internal", 1, 0, "ContentsEditorPresentationProjection");
        qmlRegisterType<ContentsEditorSessionController>(
            "WhatSon.App.Internal", 1, 0, "ContentsEditorSessionController");
        qmlRegisterType<ContentsLogicalTextBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsLogicalTextBridge");
        qmlRegisterType<ContentsGutterMarkerBridge>(
            "WhatSon.App.Internal", 1, 0, "ContentsGutterMarkerBridge");
        qmlRegisterType<ContentsResourceTagTextGenerator>(
            "WhatSon.App.Internal", 1, 0, "ContentsResourceTagTextGenerator");
        qmlRegisterType<ContentsStructuredDocumentBlocksModel>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredDocumentBlocksModel");
        qmlRegisterType<ContentsStructuredDocumentCollectionPolicy>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredDocumentCollectionPolicy");
        qmlRegisterType<ContentsStructuredDocumentFocusPolicy>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredDocumentFocusPolicy");
        qmlRegisterType<ContentsStructuredDocumentHost>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredDocumentHost");
        qmlRegisterType<ContentsStructuredDocumentMutationPolicy>(
            "WhatSon.App.Internal", 1, 0, "ContentsStructuredDocumentMutationPolicy");
        qmlRegisterType<ContentsPaperSelection>(
            "WhatSon.App.Internal", 1, 0, "ContentsPaperSelection");
        qmlRegisterType<ContentsA4PaperBackground>(
            "WhatSon.App.Internal", 1, 0, "ContentsA4PaperBackground");
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
        qmlRegisterType<WhatSonIosHubPickerBridge>(
            "WhatSon.App.Internal", 1, 0, "WhatSonIosHubPickerBridge");
    }
} // namespace WhatSon::Runtime::Bootstrap
