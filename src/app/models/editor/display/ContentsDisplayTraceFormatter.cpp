#include "app/models/editor/display/ContentsDisplayTraceFormatter.hpp"

#include <QStringList>

namespace
{
    QString optionalString(const QVariantMap& map, const QString& key)
    {
        return map.value(key).toString();
    }

    bool optionalBool(const QVariantMap& map, const QString& key)
    {
        return map.value(key).toBool();
    }
}

ContentsDisplayTraceFormatter::ContentsDisplayTraceFormatter(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayTraceFormatter::~ContentsDisplayTraceFormatter() = default;

QString ContentsDisplayTraceFormatter::describeSelectionSyncOptions(const QVariantMap& options) const
{
    return QStringLiteral(
               "resetSnapshot=%1 scheduleReconcile=%2 focusEditor=%3 fallbackRefresh=%4 forceVisualRefresh=%5")
        .arg(optionalBool(options, QStringLiteral("resetSnapshot")))
        .arg(optionalBool(options, QStringLiteral("scheduleReconcile")))
        .arg(optionalBool(options, QStringLiteral("focusEditor")))
        .arg(optionalBool(options, QStringLiteral("fallbackRefresh")))
        .arg(optionalBool(options, QStringLiteral("forceVisualRefresh")));
}

QString ContentsDisplayTraceFormatter::describeSelectionPlan(const QVariantMap& plan) const
{
    return QStringLiteral(
               "reason=%1 noteId=%2 selectedNoteId=%3 bodyNoteId=%4 bodyResolved=%5 allowSnapshotRefresh=%6 attemptReconcile=%7 attemptSnapshotRefresh=%8 attemptEditorSessionMount=%9 attemptSelectionSync=%10 resetSelectionCache=%11 scheduleSnapshotReconcile=%12 focusEditor=%13 flushPendingEditorText=%14 fallbackRefresh=%15 forceVisualRefresh=%16")
        .arg(optionalString(plan, QStringLiteral("reason")))
        .arg(optionalString(plan, QStringLiteral("noteId")))
        .arg(optionalString(plan, QStringLiteral("selectedNoteId")))
        .arg(optionalString(plan, QStringLiteral("selectedNoteBodyNoteId")))
        .arg(optionalBool(plan, QStringLiteral("selectedNoteBodyResolved")))
        .arg(optionalBool(plan, QStringLiteral("allowSnapshotRefresh")))
        .arg(optionalBool(plan, QStringLiteral("attemptReconcile")))
        .arg(optionalBool(plan, QStringLiteral("attemptSnapshotRefresh")))
        .arg(optionalBool(plan, QStringLiteral("attemptEditorSessionMount")))
        .arg(optionalBool(plan, QStringLiteral("attemptSelectionSync")))
        .arg(optionalBool(plan, QStringLiteral("resetSelectionCache")))
        .arg(optionalBool(plan, QStringLiteral("scheduleSnapshotReconcile")))
        .arg(optionalBool(plan, QStringLiteral("focusEditorForSelectedNote")))
        .arg(optionalBool(plan, QStringLiteral("flushPendingEditorText")))
        .arg(optionalBool(plan, QStringLiteral("fallbackRefreshIfMountSkipped")) || optionalBool(plan, QStringLiteral("fallbackRefreshIfSyncSkipped")))
        .arg(optionalBool(plan, QStringLiteral("forceVisualRefresh")));
}

QString ContentsDisplayTraceFormatter::loaderStatusName(const int status) const
{
    switch (status)
    {
        case 0:
            return QStringLiteral("null");
        case 1:
            return QStringLiteral("ready");
        case 2:
            return QStringLiteral("loading");
        case 3:
            return QStringLiteral("error");
        default:
            break;
    }
    return QStringLiteral("unknown(%1)").arg(status);
}

QString ContentsDisplayTraceFormatter::describeEditorSurfaceObject(QObject* objectValue) const
{
    if (objectValue == nullptr)
    {
        return QStringLiteral("<null>");
    }

    const QString className = objectValue->metaObject() == nullptr
        ? QStringLiteral("unknown")
        : QString::fromUtf8(objectValue->metaObject()->className());
    const QString objectName = objectValue->objectName().trimmed().isEmpty()
        ? QStringLiteral("<empty>")
        : objectValue->objectName().trimmed();
    return QStringLiteral("class=%1 objectName=%2").arg(className, objectName);
}
