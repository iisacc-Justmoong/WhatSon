#pragma once

#include "WhatSonHubTagsPathResolver.hpp"
#include "WhatSonTagDepthEntry.hpp"
#include "WhatSonTagsDepthFlattener.hpp"
#include "WhatSonTagsFileReader.hpp"
#include "WhatSonTagsJsonParser.hpp"

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
