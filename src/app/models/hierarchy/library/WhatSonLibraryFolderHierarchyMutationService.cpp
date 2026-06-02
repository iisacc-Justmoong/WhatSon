#include "app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.hpp"

#include "app/models/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"

#include <utility>

WhatSonLibraryFolderHierarchyMutationService::WhatSonLibraryFolderHierarchyMutationService() = default;

WhatSonLibraryFolderHierarchyMutationService::~WhatSonLibraryFolderHierarchyMutationService() = default;

bool WhatSonLibraryFolderHierarchyMutationService::commitMutation(
    Request request,
    Result* outResult,
    QString* errorMessage) const
{
    if (!request.foldersFilePath.trimmed().isEmpty())
    {
        WhatSonFoldersHierarchyStore stagedStore;
        stagedStore.setFolderEntries(request.stagedFolderEntries);

        QString writeError;
        if (!stagedStore.writeToFile(request.foldersFilePath, &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (outResult != nullptr)
    {
        outResult->notes = std::move(request.notes);
    }

    return true;
}
