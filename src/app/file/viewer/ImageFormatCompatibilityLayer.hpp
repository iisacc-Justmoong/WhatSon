#pragma once

#include <QString>

namespace WhatSon::Viewer::ImageFormatCompatibilityLayer
{
    QString normalizedBitmapFormat(const QString& value);
    bool isBitmapFormatCompatible(const QString& value);
    QString unsupportedBitmapFormatMessage(const QString& value);
} // namespace WhatSon::Viewer::ImageFormatCompatibilityLayer

