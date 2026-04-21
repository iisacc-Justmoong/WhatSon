#pragma once

#include <QString>
#include <QStringList>

class WhatSonNoteFolderBindingService final
{
public:
    struct Bindings final
    {
        QStringList folders;
        QStringList folderUuids;
    };

    WhatSonNoteFolderBindingService();
    ~WhatSonNoteFolderBindingService();

    Bindings bindings(const QStringList& folders, const QStringList& folderUuids) const;
    Bindings mergeBindings(const Bindings& primary, const Bindings& secondary) const;
    Bindings mergeBindings(
        const QStringList& primaryFolders,
        const QStringList& primaryFolderUuids,
        const QStringList& secondaryFolders,
        const QStringList& secondaryFolderUuids) const;
    Bindings assignFolder(const Bindings& existing, const QString& folderPath, const QString& folderUuid) const;
    bool contains(const Bindings& bindings, const QString& folderPath, const QString& folderUuid) const;
    bool matches(const Bindings& lhs, const Bindings& rhs) const;
};
