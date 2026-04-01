#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>

struct ResourcesHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    int count = 0;
    bool showChevron = true;
    QString key;
    QString kind;
    QString bucket;
    QString type;
    QString format;
    QString resourceId;
    QString resourcePath;
    QString assetPath;
};

inline QString resourcesHierarchyIconName(const ResourcesHierarchyItem& item)
{
    if (item.kind == QStringLiteral("asset"))
    {
        if (item.type == QStringLiteral("image"))
        {
            return QStringLiteral("imageToImage");
        }
        if (item.type == QStringLiteral("video"))
        {
            return QStringLiteral("generalshow");
        }
        if (item.type == QStringLiteral("document"))
        {
            return QStringLiteral("dataView");
        }
        if (item.type == QStringLiteral("link"))
        {
            return QStringLiteral("cwmPermissionView");
        }
        if (item.type == QStringLiteral("music") || item.type == QStringLiteral("audio"))
        {
            return QStringLiteral("audioToAudio");
        }
        if (item.type == QStringLiteral("archive"))
        {
            return QStringLiteral("sortByType");
        }
        if (item.type == QStringLiteral("model"))
        {
            return QStringLiteral("dataFile");
        }
    }
    return QStringLiteral("virtualFolder");
}

class ResourcesHierarchyModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool strictValidation READ strictValidation WRITE setStrictValidation NOTIFY strictValidationChanged)
    Q_PROPERTY(int correctionCount READ correctionCount NOTIFY correctionCountChanged)
    Q_PROPERTY(QString lastValidationCode READ lastValidationCode NOTIFY validationStateChanged)
    Q_PROPERTY(QString lastValidationMessage READ lastValidationMessage NOTIFY validationStateChanged)

public:
    enum Role
    {
        LabelRole = Qt::UserRole + 1,
        CountRole,
        DepthRole,
        IndentLevelRole,
        AccentRole,
        ExpandedRole,
        ShowChevronRole,
        IconNameRole,
        KeyRole,
        KindRole,
        BucketRole,
        TypeRole,
        FormatRole,
        ResourceIdRole,
        ResourcePathRole,
        AssetPathRole
    };

    Q_ENUM(Role)

    explicit ResourcesHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int itemCount() const noexcept;
    bool strictValidation() const noexcept;
    void setStrictValidation(bool enabled);
    int correctionCount() const noexcept;
    QString lastValidationCode() const;
    QString lastValidationMessage() const;

    void setItems(QVector<ResourcesHierarchyItem> items);
    const QVector<ResourcesHierarchyItem>& items() const noexcept;

public
    slots  :




    void requestModelHook()
    {
        emit modelHookRequested();
    }

    signals  :



    void itemCountChanged(int itemCount);
    void itemsChanged();
    void strictValidationChanged();
    void correctionCountChanged();
    void validationStateChanged();
    void validationIssueRaised(const QString& code, const QString& message, const QVariantMap& context);
    void itemCorrected(const QString& code, const QVariantMap& context);
    void modelHookRequested();

private:
    void setValidationState(QString code, QString message);

    QVector<ResourcesHierarchyItem> m_items;
    bool m_strictValidation = false;
    int m_correctionCount = 0;
    QString m_lastValidationCode;
    QString m_lastValidationMessage;
};
