#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QVector>

struct ResourcesListItem
{
    QString id;
    QString primaryText;
    QString searchableText;
    QString bodyText;
    QString displayDate;
    QStringList folders;
    QStringList tags;
    bool image = false;
    QString imageSource;
    bool bookmarked = false;
    QString bookmarkColor;
    QString type;
    QString format;
    QString resourcePath;
    QString resolvedPath;
    QString source;
    QString renderMode;
    QString displayName;
    QString previewText;
};

class ResourcesListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText NOTIFY currentBodyTextChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        NoteIdRole,
        PrimaryTextRole,
        BodyTextRole,
        ImageRole,
        ImageSourceRole,
        DisplayDateRole,
        FoldersRole,
        TagsRole,
        BookmarkedRole,
        BookmarkColorRole,
        TypeRole,
        FormatRole,
        ResourcePathRole,
        ResolvedPathRole,
        SourceRole,
        RenderModeRole,
        DisplayNameRole,
        PreviewTextRole
    };

    Q_ENUM(Role)

    explicit ResourcesListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int itemCount() const noexcept;
    int currentIndex() const noexcept;
    QString currentNoteId() const;
    QString currentBodyText() const;
    Q_INVOKABLE void setCurrentIndex(int index);
    QString searchText() const;
    void setSearchText(const QString& text);

    void setItems(QVector<ResourcesListItem> items);
    const QVector<ResourcesListItem>& items() const noexcept;

public
    slots  :




    void requestModelHook()
    {
        emit modelHookRequested();
    }

    signals  :



    void itemCountChanged(int itemCount);
    void itemsChanged();
    void currentIndexChanged();
    void currentNoteIdChanged();
    void currentBodyTextChanged();
    void searchTextChanged();
    void modelHookRequested();

private:
    void applySearchFilter();

    QVector<ResourcesListItem> m_sourceItems;
    QVector<ResourcesListItem> m_items;
    QString m_searchText;
    int m_currentIndex = -1;
};

