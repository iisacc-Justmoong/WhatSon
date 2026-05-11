#include "app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"

#include "app/platform/Apple/WhatSonIosHubPickerBridge.hpp"
#include "app/models/content/mobile/MobileEventSurfaceController.hpp"
#include "app/models/content/mobile/MobileHierarchyBackSwipeCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyCanonicalRoutePlanner.hpp"
#include "app/models/content/mobile/MobileHierarchyNavigationCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyPopRepairPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteSelectionSyncPolicy.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "app/models/content/mobile/MobileHierarchySelectionCoordinator.hpp"
#include "app/models/panel/FocusedNoteDeletionBridge.hpp"
#include "app/models/panel/HierarchyDragDropBridge.hpp"
#include "app/models/panel/HierarchyInteractionBridge.hpp"
#include "app/models/panel/NoteListModelContractBridge.hpp"
#include "app/models/sidebar/SidebarHierarchyInteractionController.hpp"

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
            whatsonInternalCreatableType<MobileEventSurfaceController>(
                QStringLiteral("MobileEventSurfaceController")),
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
            whatsonInternalCreatableType<FocusedNoteDeletionBridge>(
                QStringLiteral("FocusedNoteDeletionBridge")),
            whatsonInternalCreatableType<NoteListModelContractBridge>(
                QStringLiteral("NoteListModelContractBridge")),
            whatsonInternalCreatableType<HierarchyDragDropBridge>(
                QStringLiteral("HierarchyDragDropBridge")),
            whatsonInternalCreatableType<HierarchyInteractionBridge>(
                QStringLiteral("HierarchyInteractionBridge")),
            whatsonInternalCreatableType<SidebarHierarchyInteractionController>(
                QStringLiteral("SidebarHierarchyInteractionController")),
            whatsonInternalCreatableType<WhatSonIosHubPickerBridge>(
                QStringLiteral("WhatSonIosHubPickerBridge"))
        };
    }

    lvrs::QmlTypeRegistrationReport registerInternalQmlTypes()
    {
        return lvrs::registerQmlTypes(internalQmlTypeRegistrationManifest());
    }
} // namespace WhatSon::Runtime::Bootstrap
