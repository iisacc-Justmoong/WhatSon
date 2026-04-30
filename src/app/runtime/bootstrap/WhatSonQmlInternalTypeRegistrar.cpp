#include "app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"

#include "app/models/editor/tags/ContentsAgendaBackend.hpp"
#include "app/models/editor/tags/ContentsCalloutBackend.hpp"
#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "app/models/display/paper/ContentsA4PaperBackground.hpp"
#include "app/models/display/paper/ContentsPaperSelection.hpp"
#include "app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp"
#include "app/models/editor/format/ContentsPlainTextSourceMutator.hpp"
#include "app/models/editor/format/ContentsTextFormatRenderer.hpp"
#include "app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "app/models/editor/tags/ContentsStructuredTagValidator.hpp"
#include "app/platform/Apple/WhatSonIosHubPickerBridge.hpp"
#include "app/models/file/viewer/ContentsBodyResourceRenderer.hpp"
#include "app/models/file/viewer/ResourceBitmapViewer.hpp"
#include "app/models/editor/display/ContentsDisplayContextMenuCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayDocumentSourceResolver.hpp"
#include "app/models/editor/display/ContentsDisplayEditOperationCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayGeometryState.hpp"
#include "app/models/editor/display/ContentsDisplayGutterCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayHostModePolicy.hpp"
#include "app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayNoteBodyMountCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayPresentationState.hpp"
#include "app/models/editor/display/ContentsDisplayResourceUiState.hpp"
#include "app/models/content/mobile/MobileHierarchyBackSwipeCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyCanonicalRoutePlanner.hpp"
#include "app/models/content/mobile/MobileHierarchyNavigationCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyPopRepairPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteSelectionSyncPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "app/models/content/mobile/MobileHierarchySelectionCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayPresentationRefreshController.hpp"
#include "app/models/editor/display/ContentsDisplayRefreshCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayStructuredFlowCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayTraceFormatter.hpp"
#include "app/models/editor/display/ContentsDisplayViewportCoordinator.hpp"
#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "app/models/editor/session/ContentsEditorSessionController.hpp"
#include "app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"
#include "app/models/editor/bridge/ContentsGutterMarkerBridge.hpp"
#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"
#include "app/models/editor/tags/ContentsResourceTagTextGenerator.hpp"
#include "app/models/editor/tags/ContentsResourceTagController.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentBlocksModel.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentFocusPolicy.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentHost.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentMutationPolicy.hpp"
#include "app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp"
#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"
#include "app/models/editor/resource/ContentsResourceDropPayloadParser.hpp"
#include "app/models/editor/resource/ContentsResourceImportConflictController.hpp"
#include "app/models/editor/resource/ContentsResourceImportController.hpp"
#include "app/models/editor/input/ContentsBreakBlockController.hpp"
#include "app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"
#include "app/models/editor/input/ContentsRemainingInputControllers.hpp"
#include "app/viewmodel/editor/display/ContentsDisplayGeometryInteraction.hpp"
#include "app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.hpp"
#include "app/viewmodel/editor/display/ContentsDisplayInteractionViewModel.hpp"
#include "app/viewmodel/editor/display/ContentsDisplayMutationViewModel.hpp"
#include "app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.hpp"
#include "app/viewmodel/editor/display/ContentsDisplaySelectionMountInteraction.hpp"
#include "app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.hpp"
#include "app/viewmodel/panel/FocusedNoteDeletionBridge.hpp"
#include "app/viewmodel/panel/HierarchyDragDropBridge.hpp"
#include "app/viewmodel/panel/HierarchyInteractionBridge.hpp"
#include "app/viewmodel/panel/NoteListModelContractBridge.hpp"

namespace WhatSon::Runtime::Bootstrap
{
namespace
{
    template <typename T>
    lvrs::QmlTypeRegistration whatsonInternalCreatableType(const QString& qmlName)
    {
        return lvrs::qmlCreatableType<T>(
            QStringLiteral("WhatSon.App.Internal"),
            1,
            0,
            qmlName,
            qmlName);
    }
} // namespace

    QList<lvrs::QmlTypeRegistration> internalQmlTypeRegistrationManifest()
    {
        return {
            whatsonInternalCreatableType<ContentsEditorSelectionBridge>(
                QStringLiteral("ContentsEditorSelectionBridge")),
            whatsonInternalCreatableType<ContentsDisplaySelectionSyncCoordinator>(
                QStringLiteral("ContentsDisplaySelectionSyncCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayPresentationRefreshController>(
                QStringLiteral("ContentsDisplayPresentationRefreshController")),
            whatsonInternalCreatableType<ContentsDisplayPresentationState>(
                QStringLiteral("ContentsDisplayPresentationState")),
            whatsonInternalCreatableType<ContentsDisplayRefreshCoordinator>(
                QStringLiteral("ContentsDisplayRefreshCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayResourceUiState>(
                QStringLiteral("ContentsDisplayResourceUiState")),
            whatsonInternalCreatableType<ContentsDisplayContextMenuCoordinator>(
                QStringLiteral("ContentsDisplayContextMenuCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayGutterCoordinator>(
                QStringLiteral("ContentsDisplayGutterCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayMinimapCoordinator>(
                QStringLiteral("ContentsDisplayMinimapCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayNoteBodyMountCoordinator>(
                QStringLiteral("ContentsDisplayNoteBodyMountCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayDocumentSourceResolver>(
                QStringLiteral("ContentsDisplayDocumentSourceResolver")),
            whatsonInternalCreatableType<ContentsDisplayEditOperationCoordinator>(
                QStringLiteral("ContentsDisplayEditOperationCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayGeometryState>(
                QStringLiteral("ContentsDisplayGeometryState")),
            whatsonInternalCreatableType<ContentsDisplayHostModePolicy>(
                QStringLiteral("ContentsDisplayHostModePolicy")),
            whatsonInternalCreatableType<MobileHierarchyBackSwipeCoordinator>(
                QStringLiteral("MobileHierarchyBackSwipeCoordinator")),
            whatsonInternalCreatableType<MobileHierarchyCanonicalRoutePlanner>(
                QStringLiteral("MobileHierarchyCanonicalRoutePlanner")),
            whatsonInternalCreatableType<MobileHierarchyNavigationCoordinator>(
                QStringLiteral("MobileHierarchyNavigationCoordinator")),
            whatsonInternalCreatableType<MobileHierarchyPopRepairPolicy>(
                QStringLiteral("MobileHierarchyPopRepairPolicy")),
            whatsonInternalCreatableType<MobileHierarchyRouteSelectionSyncPolicy>(
                QStringLiteral("MobileHierarchyRouteSelectionSyncPolicy")),
            whatsonInternalCreatableType<MobileHierarchyRouteStateStore>(
                QStringLiteral("MobileHierarchyRouteStateStore")),
            whatsonInternalCreatableType<MobileHierarchySelectionCoordinator>(
                QStringLiteral("MobileHierarchySelectionCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayViewportCoordinator>(
                QStringLiteral("ContentsDisplayViewportCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayStructuredFlowCoordinator>(
                QStringLiteral("ContentsDisplayStructuredFlowCoordinator")),
            whatsonInternalCreatableType<ContentsDisplayTraceFormatter>(
                QStringLiteral("ContentsDisplayTraceFormatter")),
            whatsonInternalCreatableType<ContentsEditorPresentationProjection>(
                QStringLiteral("ContentsEditorPresentationProjection")),
            whatsonInternalCreatableType<ContentsEditorSessionController>(
                QStringLiteral("ContentsEditorSessionController")),
            whatsonInternalCreatableType<ContentsLogicalTextBridge>(
                QStringLiteral("ContentsLogicalTextBridge")),
            whatsonInternalCreatableType<ContentsGutterMarkerBridge>(
                QStringLiteral("ContentsGutterMarkerBridge")),
            whatsonInternalCreatableType<ContentsResourceTagTextGenerator>(
                QStringLiteral("ContentsResourceTagTextGenerator")),
            whatsonInternalCreatableType<ContentsResourceTagController>(
                QStringLiteral("ContentsResourceTagController")),
            whatsonInternalCreatableType<ContentsResourceDropPayloadParser>(
                QStringLiteral("ContentsResourceDropPayloadParser")),
            whatsonInternalCreatableType<ContentsEditorSurfaceGuardController>(
                QStringLiteral("ContentsEditorSurfaceGuardController")),
            whatsonInternalCreatableType<ContentsInlineResourcePresentationController>(
                QStringLiteral("ContentsInlineResourcePresentationController")),
            whatsonInternalCreatableType<ContentsResourceImportConflictController>(
                QStringLiteral("ContentsResourceImportConflictController")),
            whatsonInternalCreatableType<ContentsResourceImportController>(
                QStringLiteral("ContentsResourceImportController")),
            whatsonInternalCreatableType<ContentsStructuredDocumentBlocksModel>(
                QStringLiteral("ContentsStructuredDocumentBlocksModel")),
            whatsonInternalCreatableType<ContentsStructuredDocumentCollectionPolicy>(
                QStringLiteral("ContentsStructuredDocumentCollectionPolicy")),
            whatsonInternalCreatableType<ContentsStructuredDocumentFocusPolicy>(
                QStringLiteral("ContentsStructuredDocumentFocusPolicy")),
            whatsonInternalCreatableType<ContentsStructuredDocumentHost>(
                QStringLiteral("ContentsStructuredDocumentHost")),
            whatsonInternalCreatableType<ContentsStructuredDocumentMutationPolicy>(
                QStringLiteral("ContentsStructuredDocumentMutationPolicy")),
            whatsonInternalCreatableType<ContentsBreakBlockController>(
                QStringLiteral("ContentsBreakBlockController")),
            whatsonInternalCreatableType<ContentsEditorInputPolicyAdapter>(
                QStringLiteral("ContentsEditorInputPolicyAdapter")),
            whatsonInternalCreatableType<ContentsAgendaBlockController>(
                QStringLiteral("ContentsAgendaBlockController")),
            whatsonInternalCreatableType<ContentsAgendaTaskRowController>(
                QStringLiteral("ContentsAgendaTaskRowController")),
            whatsonInternalCreatableType<ContentsCalloutBlockController>(
                QStringLiteral("ContentsCalloutBlockController")),
            whatsonInternalCreatableType<ContentsDocumentBlockController>(
                QStringLiteral("ContentsDocumentBlockController")),
            whatsonInternalCreatableType<ContentsDocumentTextBlockController>(
                QStringLiteral("ContentsDocumentTextBlockController")),
            whatsonInternalCreatableType<ContentsEditorSelectionController>(
                QStringLiteral("ContentsEditorSelectionController")),
            whatsonInternalCreatableType<ContentsEditorTypingController>(
                QStringLiteral("ContentsEditorTypingController")),
            whatsonInternalCreatableType<ContentsInlineFormatEditorController>(
                QStringLiteral("ContentsInlineFormatEditorController")),
            whatsonInternalCreatableType<ContentsDisplayGeometryInteraction>(
                QStringLiteral("ContentsDisplayGeometryInteraction")),
            whatsonInternalCreatableType<ContentsDisplayGeometryViewModel>(
                QStringLiteral("ContentsDisplayGeometryViewModel")),
            whatsonInternalCreatableType<ContentsDisplayInteractionViewModel>(
                QStringLiteral("ContentsDisplayInteractionViewModel")),
            whatsonInternalCreatableType<ContentsDisplayMutationViewModel>(
                QStringLiteral("ContentsDisplayMutationViewModel")),
            whatsonInternalCreatableType<ContentsDisplayPresentationViewModel>(
                QStringLiteral("ContentsDisplayPresentationViewModel")),
            whatsonInternalCreatableType<ContentsDisplaySelectionMountInteraction>(
                QStringLiteral("ContentsDisplaySelectionMountInteraction")),
            whatsonInternalCreatableType<ContentsDisplaySurfacePolicy>(
                QStringLiteral("ContentsDisplaySurfacePolicy")),
            whatsonInternalCreatableType<ContentsPaperSelection>(
                QStringLiteral("ContentsPaperSelection")),
            whatsonInternalCreatableType<ContentsA4PaperBackground>(
                QStringLiteral("ContentsA4PaperBackground")),
            whatsonInternalCreatableType<ContentsTextFormatRenderer>(
                QStringLiteral("ContentsTextFormatRenderer")),
            whatsonInternalCreatableType<ContentsInlineStyleOverlayRenderer>(
                QStringLiteral("ContentsInlineStyleOverlayRenderer")),
            whatsonInternalCreatableType<ContentsPlainTextSourceMutator>(
                QStringLiteral("ContentsPlainTextSourceMutator")),
            whatsonInternalCreatableType<ContentsStructuredBlockRenderer>(
                QStringLiteral("ContentsStructuredBlockRenderer")),
            whatsonInternalCreatableType<ContentsStructuredTagValidator>(
                QStringLiteral("ContentsStructuredTagValidator")),
            whatsonInternalCreatableType<ContentsAgendaBackend>(
                QStringLiteral("ContentsAgendaBackend")),
            whatsonInternalCreatableType<ContentsCalloutBackend>(
                QStringLiteral("ContentsCalloutBackend")),
            whatsonInternalCreatableType<ContentsPagePrintLayoutRenderer>(
                QStringLiteral("ContentsPagePrintLayoutRenderer")),
            whatsonInternalCreatableType<ContentsBodyResourceRenderer>(
                QStringLiteral("ContentsBodyResourceRenderer")),
            whatsonInternalCreatableType<ResourceBitmapViewer>(
                QStringLiteral("ResourceBitmapViewer")),
            whatsonInternalCreatableType<FocusedNoteDeletionBridge>(
                QStringLiteral("FocusedNoteDeletionBridge")),
            whatsonInternalCreatableType<NoteListModelContractBridge>(
                QStringLiteral("NoteListModelContractBridge")),
            whatsonInternalCreatableType<HierarchyDragDropBridge>(
                QStringLiteral("HierarchyDragDropBridge")),
            whatsonInternalCreatableType<HierarchyInteractionBridge>(
                QStringLiteral("HierarchyInteractionBridge")),
            whatsonInternalCreatableType<WhatSonIosHubPickerBridge>(
                QStringLiteral("WhatSonIosHubPickerBridge"))
        };
    }

    lvrs::QmlTypeRegistrationReport registerInternalQmlTypes()
    {
        return lvrs::registerQmlTypes(internalQmlTypeRegistrationManifest());
    }
} // namespace WhatSon::Runtime::Bootstrap
