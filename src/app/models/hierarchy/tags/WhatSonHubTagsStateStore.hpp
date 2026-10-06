#pragma once
#include "app/models/hierarchy/tags/WhatSonTagDepthEntry.hpp"
#include "app/models/hierarchy/tags/TagsHierarchyControllerSupport.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QStringList>
#include <QVector>

// Hub-scoped tag snapshots are values; committing one hub leaves others intact.
class WhatSonHubTagsStateStore final {
public:
    bool loadFromWshub(const QString& hub, QString* errorMessage = nullptr) {
        QStringList directories;
        if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(hub, &directories, errorMessage))
            return false;
        QVector<WhatSonTagDepthEntry> staged;
        WhatSonTagsHierarchyParser parser;
        for (const auto& directory : directories) {
            const auto path = QDir(directory).filePath(QStringLiteral("Tags.wstags"));
            if (!QFileInfo::exists(path)) continue;
            QString text;
            if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(path, &text, errorMessage)) return false;
            WhatSonTagsHierarchyStore parsed;
            if (!parser.parse(text, &parsed, errorMessage)) return false;
            staged += parsed.tagEntries();
        }
        setEntries(hub, std::move(staged));
        if (errorMessage) errorMessage->clear();
        return true;
    }
    bool contains(const QString& hub) const { return m_entries.contains(QDir::cleanPath(hub)); }
    QStringList hubPaths() const { return m_entries.keys(); }
    QVector<WhatSonTagDepthEntry> entries(const QString& hub) const {
        return m_entries.value(QDir::cleanPath(hub));
    }
    void setEntries(const QString& hub, QVector<WhatSonTagDepthEntry> entries) {
        m_entries.insert(QDir::cleanPath(hub), std::move(entries));
    }
    void remove(const QString& hub) { m_entries.remove(QDir::cleanPath(hub)); }
    void clear() { m_entries.clear(); }
private:
    QHash<QString, QVector<WhatSonTagDepthEntry>> m_entries;
};
