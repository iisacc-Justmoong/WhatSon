#include "app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"

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
            whatsonInternalCreatableType<FocusedNoteDeletionBridge>(
                QStringLiteral("FocusedNoteDeletionBridge")),
            whatsonInternalCreatableType<NoteListModelContractBridge>(
                QStringLiteral("NoteListModelContractBridge")),
            whatsonInternalCreatableType<HierarchyDragDropBridge>(
                QStringLiteral("HierarchyDragDropBridge")),
            whatsonInternalCreatableType<HierarchyInteractionBridge>(
                QStringLiteral("HierarchyInteractionBridge")),
            whatsonInternalCreatableType<SidebarHierarchyInteractionController>(
                QStringLiteral("SidebarHierarchyInteractionController"))
        };
    }

    lvrs::QmlTypeRegistrationReport registerInternalQmlTypes()
    {
        return lvrs::registerQmlTypes(internalQmlTypeRegistrationManifest());
    }
} // namespace WhatSon::Runtime::Bootstrap
