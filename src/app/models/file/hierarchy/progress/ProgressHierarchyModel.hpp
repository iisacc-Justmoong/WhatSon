#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>

struct ProgressHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

inline QString progressHierarchyIconName(const ProgressHierarchyItem& item)
{
    QString normalizedLabel = item.label.trimmed().toLower();
    normalizedLabel.remove(QLatin1Char(' '));
    normalizedLabel.remove(QLatin1Char('-'));
    normalizedLabel.remove(QLatin1Char('_'));
    if (normalizedLabel == QStringLiteral("ready") || normalizedLabel == QStringLiteral("firstdraft"))
    {
        return QStringLiteral("inlineinlineEdit");
    }
    if (normalizedLabel == QStringLiteral("modifieddraft"))
    {
        return QStringLiteral("rendererKit");
    }
    if (normalizedLabel == QStringLiteral("inprogress"))
    {
        return QStringLiteral("progressresume");
    }
    if (normalizedLabel == QStringLiteral("pending"))
    {
        return QStringLiteral("pending");
    }
    if (normalizedLabel == QStringLiteral("reviewing"))
    {
        return QStringLiteral("showLogs");
    }
    if (normalizedLabel == QStringLiteral("waitingforapproval"))
    {
        return QStringLiteral("toolWindowTimer");
    }
    if (normalizedLabel == QStringLiteral("done"))
    {
        return QStringLiteral("validator");
    }
    if (normalizedLabel == QStringLiteral("lagacy") || normalizedLabel == QStringLiteral("legacy"))
    {
        return QStringLiteral("nodesexcludeRoot");
    }
    if (normalizedLabel == QStringLiteral("archived"))
    {
        return QStringLiteral("projectModels");
    }
    if (normalizedLabel == QStringLiteral("deletereview"))
    {
        return QStringLiteral("gutterCheckBoxIndeterminate@14x14");
    }
    return QString();
}

class ProgressHierarchyModel final : public QAbstractListModel
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
        DepthRole,
        IndentLevelRole,
        AccentRole,
        ExpandedRole,
        ShowChevronRole,
        IconNameRole
    };

    Q_ENUM(Role)

    explicit ProgressHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int itemCount() const noexcept;
    bool strictValidation() const noexcept;
    void setStrictValidation(bool enabled);
    int correctionCount() const noexcept;
    QString lastValidationCode() const;
    QString lastValidationMessage() const;

    void setItems(QVector<ProgressHierarchyItem> items);
    const QVector<ProgressHierarchyItem>& items() const noexcept;

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

    QVector<ProgressHierarchyItem> m_items;
    bool m_strictValidation = false;
    int m_correctionCount = 0;
    QString m_lastValidationCode;
    QString m_lastValidationMessage;
};
