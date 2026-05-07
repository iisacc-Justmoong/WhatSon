#pragma once

#include <QRectF>
#include <QtPlugin>

struct ContentsEditorGeometryMeasurement
{
    QRectF rectangle;
    bool geometryAvailable = false;
};

class IContentsEditorGeometryProvider
{
public:
    virtual ~IContentsEditorGeometryProvider() = default;

    virtual ContentsEditorGeometryMeasurement measureTextRange(
        int logicalStart,
        int logicalEnd,
        int logicalLength,
        qreal fallbackLineHeight,
        qreal fallbackWidth) const = 0;

};

#define IContentsEditorGeometryProvider_iid "com.iisacc.WhatSon.IContentsEditorGeometryProvider/1.0"
Q_DECLARE_INTERFACE(IContentsEditorGeometryProvider, IContentsEditorGeometryProvider_iid)
