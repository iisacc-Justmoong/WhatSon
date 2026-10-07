#pragma once

#include "app/models/hierarchy/tags/WhatSonHubTagsPathResolver.hpp"
#include "app/models/hierarchy/tags/WhatSonTagDepthEntry.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsDepthFlattener.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsFileReader.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsJsonParser.hpp"

#include <QVector>

class WhatSonHubTagsDepthProvider
{
public:
    WhatSonHubTagsDepthProvider();
    ~WhatSonHubTagsDepthProvider();

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    QVector<WhatSonTagDepthEntry> tagDepthEntries() const;
    void clear();

private:
    WhatSonHubTagsPathResolver m_pathResolver;
    WhatSonTagsFileReader m_fileReader;
    WhatSonTagsJsonParser m_jsonParser;
    WhatSonTagsDepthFlattener m_depthFlattener;
    QVector<WhatSonTagDepthEntry> m_entries;
};
