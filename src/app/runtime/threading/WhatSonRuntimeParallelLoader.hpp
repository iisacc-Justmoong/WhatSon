#pragma once

#include "IWhatSonRuntimeParallelLoader.hpp"

class WhatSonRuntimeParallelLoader final : public IWhatSonRuntimeParallelLoader
{
public:
    using RequestedDomains = IWhatSonRuntimeParallelLoader::RequestedDomains;
    using DomainLoadResult = IWhatSonRuntimeParallelLoader::DomainLoadResult;
    using Targets = IWhatSonRuntimeParallelLoader::Targets;

    bool loadFromWshub(
        const QString& wshubPath,
        const Targets& targets,
        const RequestedDomains& requestedDomains,
        QVector<DomainLoadResult>* outResults = nullptr) const override;
};
