#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ContentsStructuredDocumentCollectionPolicy : public QObject
{
    Q_OBJECT

public:
    explicit ContentsStructuredDocumentCollectionPolicy(QObject* parent = nullptr);
    ~ContentsStructuredDocumentCollectionPolicy() override;

    Q_INVOKABLE QVariantList normalizeEntries(const QVariant& rawEntries) const;
    Q_INVOKABLE QString normalizeSourceText(const QString& value) const;
    Q_INVOKABLE QString spliceSourceRange(
        const QString& sourceText,
        int start,
        int end,
        const QString& replacementText) const;
    Q_INVOKABLE int floorNumberOrFallback(const QVariant& value, int fallbackValue) const;
    Q_INVOKABLE QVariantMap resourceEntryForBlock(
        const QVariantMap& blockEntry,
        const QVariant& renderedResources) const;

private:
    static bool resourceEntryHasResolvedPayload(const QVariantMap& entry);
};
