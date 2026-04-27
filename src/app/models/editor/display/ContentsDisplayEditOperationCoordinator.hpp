#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplayEditOperationCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit ContentsDisplayEditOperationCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayEditOperationCoordinator() override;

    Q_INVOKABLE QVariantMap structuredShortcutPlan(const QString& shortcutKind, bool showStructuredDocumentFlow, bool canUseStructuredShortcut) const;
    Q_INVOKABLE QVariantMap structuredEndEditPlan(bool showStructuredDocumentFlow, bool canRequestStructuredEndEdit, int logicalTextLength) const;
    Q_INVOKABLE QVariantMap focusStructuredSourceOffsetPlan(bool showStructuredDocumentFlow, bool canRequestStructuredFocus, int sourceOffset) const;
};
