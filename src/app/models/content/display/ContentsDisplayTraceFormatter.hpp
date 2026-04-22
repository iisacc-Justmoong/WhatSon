#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplayTraceFormatter : public QObject
{
    Q_OBJECT

public:
    explicit ContentsDisplayTraceFormatter(QObject* parent = nullptr);
    ~ContentsDisplayTraceFormatter() override;

    Q_INVOKABLE QString describeSelectionSyncOptions(const QVariantMap& options) const;
    Q_INVOKABLE QString describeSelectionPlan(const QVariantMap& plan) const;
    Q_INVOKABLE QString loaderStatusName(int status) const;
    Q_INVOKABLE QString describeEditorSurfaceObject(QObject* objectValue) const;
};
