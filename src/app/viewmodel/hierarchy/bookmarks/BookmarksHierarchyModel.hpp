#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>

struct BookmarksHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

inline QString bookmarksHierarchyIconName(const BookmarksHierarchyItem&)
{
    return QStringLiteral("bookmarksbookmarksList");
}

class BookmarksHierarchyModel final : public QAbstractListModel
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

    explicit BookmarksHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int itemCount() const noexcept;
    bool strictValidation() const noexcept;
    void setStrictValidation(bool enabled);
    int correctionCount() const noexcept;
    QString lastValidationCode() const;
    QString lastValidationMessage() const;

    void setItems(QVector<BookmarksHierarchyItem> items);
    const QVector<BookmarksHierarchyItem>& items() const noexcept;

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

    QVector<BookmarksHierarchyItem> m_items;
    bool m_strictValidation = false;
    int m_correctionCount = 0;
    QString m_lastValidationCode;
    QString m_lastValidationMessage;
};
