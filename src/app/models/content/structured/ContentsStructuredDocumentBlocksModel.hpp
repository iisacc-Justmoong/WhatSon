#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class ContentsStructuredDocumentBlocksModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList blocks READ blocks WRITE setBlocks NOTIFY blocksChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role
    {
        BlockDataRole = Qt::UserRole + 1,
        BlockStableKeyRole,
    };
    Q_ENUM(Role)

    explicit ContentsStructuredDocumentBlocksModel(QObject* parent = nullptr);
    ~ContentsStructuredDocumentBlocksModel() override;

    QVariantList blocks() const;
    void setBlocks(const QVariantList& blocks);

    int count() const noexcept;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void blocksChanged();
    void countChanged();

private:
    struct Entry final
    {
        QVariantMap blockData;
        QString stableKey;

        bool operator==(const Entry& other) const
        {
            return blockData == other.blockData
                && stableKey == other.stableKey;
        }
    };

    static Entry entryFromVariant(const QVariant& value);
    static bool entriesMatchForRetention(const Entry& lhs, const Entry& rhs);
    static QString stableKeyForBlock(const QVariantMap& blockData);

    int commonPrefixCount(const QVector<Entry>& nextEntries) const;
    int commonSuffixCount(const QVector<Entry>& nextEntries, int prefixCount) const;
    bool replaceEntryIfChanged(int row, const Entry& nextEntry);
    void emitDataChangedForRows(const QVector<int>& rows);

    QVector<Entry> m_entries;
};
