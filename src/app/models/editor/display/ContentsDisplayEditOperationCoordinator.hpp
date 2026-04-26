#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class ContentsDisplayEditOperationCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit ContentsDisplayEditOperationCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayEditOperationCoordinator() override;

    Q_INVOKABLE QVariantMap documentSourceMutationPlan(
        const QVariant& nextSourceText,
        const QString& currentSourceText,
        bool showStructuredDocumentFlow) const;
    Q_INVOKABLE QVariantMap structuredShortcutPlan(const QString& shortcutKind, bool showStructuredDocumentFlow, bool canUseStructuredShortcut) const;
    Q_INVOKABLE QVariantMap structuredEndEditPlan(bool showStructuredDocumentFlow, bool canRequestStructuredEndEdit, int logicalTextLength) const;
    Q_INVOKABLE QVariantMap focusStructuredSourceOffsetPlan(bool showStructuredDocumentFlow, bool canRequestStructuredFocus, int sourceOffset) const;
};
