#include "WhatSonQmlInternalTypeRegistrar.hpp"

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"
#include "models/editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "display/paper/ContentsA4PaperBackground.hpp"
#include "display/paper/ContentsPaperSelection.hpp"
#include "display/paper/ContentsTextFormatRenderer.hpp"
#include "display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "models/file/validator/ContentsStructuredTagValidator.hpp"
#include "platform/Apple/WhatSonIosHubPickerBridge.hpp"
#include "models/file/viewer/ContentsBodyResourceRenderer.hpp"
#include "models/file/viewer/ResourceBitmapViewer.hpp"
#include "content/display/ContentsDisplayContextMenuCoordinator.hpp"
#include "content/display/ContentsDisplayGutterCoordinator.hpp"
#include "content/display/ContentsDisplayMinimapCoordinator.hpp"
#include "content/mobile/MobileHierarchyBackSwipeCoordinator.hpp"
#include "content/mobile/MobileHierarchyCanonicalRoutePlanner.hpp"
#include "content/mobile/MobileHierarchyNavigationCoordinator.hpp"
#include "content/mobile/MobileHierarchyPopRepairPolicy.hpp"
#include "content/mobile/MobileHierarchyRouteSelectionSyncPolicy.hpp"
#include "content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "content/mobile/MobileHierarchySelectionCoordinator.hpp"
#include "content/display/ContentsDisplayPresentationRefreshController.hpp"
#include "content/display/ContentsDisplayRefreshCoordinator.hpp"
#include "content/display/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "content/display/ContentsDisplaySessionCoordinator.hpp"
#include "content/display/ContentsDisplayStructuredFlowCoordinator.hpp"
#include "content/display/ContentsDisplayViewportCoordinator.hpp"
#include "models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "models/editor/session/ContentsEditorSessionController.hpp"
#include "models/editor/bridge/ContentsEditorSelectionBridge.hpp"
#include "models/editor/bridge/ContentsGutterMarkerBridge.hpp"
#include "models/editor/text/ContentsLogicalTextBridge.hpp"
#include "models/editor/resource/ContentsResourceTagTextGenerator.hpp"
#include "content/structured/ContentsStructuredDocumentBlocksModel.hpp"
#include "content/structured/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "content/structured/ContentsStructuredDocumentFocusPolicy.hpp"
#include "content/structured/ContentsStructuredDocumentHost.hpp"
#include "content/structured/ContentsStructuredDocumentMutationPolicy.hpp"
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
