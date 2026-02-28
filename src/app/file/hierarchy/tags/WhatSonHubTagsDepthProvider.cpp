#include "WhatSonHubTagsDepthProvider.hpp"

#include <QJsonArray>

WhatSonHubTagsDepthProvider::WhatSonHubTagsDepthProvider() = default;

WhatSonHubTagsDepthProvider::~WhatSonHubTagsDepthProvider() = default;

bool WhatSonHubTagsDepthProvider::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    QString tagsPath;
    if (!m_pathResolver.resolveTagsFilePath(wshubPath, &tagsPath, errorMessage))
    {
        return false;
    }

    QString rawJson;
    if (!m_fileReader.readTextFile(tagsPath, &rawJson, errorMessage))
    {
        return false;
    }

    QJsonArray rootTags;
    if (!m_jsonParser.parseRootTags(rawJson, &rootTags, errorMessage))
    {
        return false;
    }

    m_entries = m_depthFlattener.flatten(rootTags);
    return true;
}

QVector<WhatSonTagDepthEntry> WhatSonHubTagsDepthProvider::tagDepthEntries() const
{
    return m_entries;
}

void WhatSonHubTagsDepthProvider::clear()
{
    m_entries.clear();
}
