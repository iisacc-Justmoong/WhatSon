#include "WhatSonQmlInternalTypeRegistrar.hpp"

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"
#include "editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "display/paper/ContentsA4PaperBackground.hpp"
#include "display/paper/ContentsPaperSelection.hpp"
#include "display/paper/ContentsTextFormatRenderer.hpp"
#include "display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "file/validator/ContentsStructuredTagValidator.hpp"
#include "platform/Apple/WhatSonIosHubPickerBridge.hpp"
#include "file/viewer/ContentsBodyResourceRenderer.hpp"
#include "file/viewer/ResourceBitmapViewer.hpp"
#include "viewmodel/content/ContentsDisplayPresentationRefreshController.hpp"
#include "viewmodel/content/ContentsDisplayRefreshCoordinator.hpp"
#include "viewmodel/content/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "viewmodel/content/ContentsDisplayViewportCoordinator.hpp"
#include "viewmodel/content/ContentsEditorPresentationProjection.hpp"
#include "viewmodel/content/ContentsEditorSessionController.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
#include "viewmodel/content/ContentsResourceTagTextGenerator.hpp"
#include "viewmodel/content/ContentsStructuredDocumentBlocksModel.hpp"
#include "viewmodel/content/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "viewmodel/content/ContentsStructuredDocumentFocusPolicy.hpp"
#include "viewmodel/content/ContentsStructuredDocumentHost.hpp"
#include "viewmodel/content/ContentsStructuredDocumentMutationPolicy.hpp"
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
        qmlRegisterType<ContentsDisplayViewportCoordinator>(
            "WhatSon.App.Internal", 1, 0, "ContentsDisplayViewportCoordinator");
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
