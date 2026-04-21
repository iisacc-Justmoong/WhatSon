#include "app/runtime/startup/WhatSonStartupHubResolver.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hub/WhatSonHubMountValidator.hpp"
#include "app/store/hub/ISelectedHubStore.hpp"

#include <QDebug>

namespace WhatSon::Runtime::Startup
{
    StartupHubSelection resolveStartupHubSelection(
        ISelectedHubStore& selectedHubStore,
        const WhatSonHubMountValidator& hubMountValidator)
    {
        StartupHubSelection selection;
        const QString persistedHubSelectionPath = selectedHubStore.selectedHubPath();
        const QByteArray persistedHubAccessBookmark =
            persistedHubSelectionPath.isEmpty()
                ? QByteArray()
                : selectedHubStore.selectedHubAccessBookmark();

        auto tryResolve = [&selection](const QString& selectionPath,
                                       const QByteArray& selectionBookmark,
                                       const QString& selectionLabel,
                                       const StartupHubSource selectionSource,
                                       const WhatSonHubMountValidator& validator) -> bool
        {
            const WhatSonHubMountValidation mountValidation =
                validator.resolveMountedHub(selectionPath, selectionBookmark);
            if (!mountValidation.mounted)
            {
                if (!mountValidation.failureMessage.trimmed().isEmpty())
                {
                    qWarning().noquote()
                        << QStringLiteral("Failed to resolve %1 startup WhatSon Hub mount '%2': %3")
                               .arg(selectionLabel, selectionPath, mountValidation.failureMessage.trimmed());
                    selection.failureMessage = mountValidation.failureMessage.trimmed();
                }
                return false;
            }

            selection.source = selectionSource;
            selection.hubPath = mountValidation.hubPath;
            selection.accessBookmark = selectionBookmark;
            selection.mounted = true;
            selection.failureMessage.clear();
            return true;
        };

        if (!persistedHubSelectionPath.isEmpty())
        {
            tryResolve(
                persistedHubSelectionPath,
                persistedHubAccessBookmark,
                QStringLiteral("persisted"),
                StartupHubSource::PersistedSelection,
                hubMountValidator);
        }

        if (!selection.mounted)
        {
            qWarning().noquote() << QStringLiteral("No startup WhatSon Hub could be resolved.");
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadFromWshub.skipped"),
                QStringLiteral("no startup .wshub detected"));
        }

        return selection;
    }
}
