#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

struct LibraryNoteListItem
{
    QString id;
    QString noteDirectoryPath;
    QString primaryText;
    QString searchableText;
    QString bodyText;
    QString createdAt;
    QString lastModifiedAt;
    bool image = false;
    QString imageSource;
    QString displayDate;
    QStringList folders;
    QStringList tags;
    bool bookmarked = false;
    QString bookmarkColor;
};

class LibraryNoteListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentNoteDirectoryPath READ currentNoteDirectoryPath NOTIFY currentNoteDirectoryPathChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText NOTIFY currentBodyTextChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool strictValidation READ strictValidation WRITE setStrictValidation NOTIFY strictValidationChanged)
    Q_PROPERTY(int correctionCount READ correctionCount NOTIFY correctionCountChanged)
    Q_PROPERTY(QString lastValidationCode READ lastValidationCode NOTIFY validationStateChanged)
    Q_PROPERTY(QString lastValidationMessage READ lastValidationMessage NOTIFY validationStateChanged)

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        NoteIdRole,
        NoteDirectoryPathRole,
        PrimaryTextRole,
        BodyTextRole,
        ImageRole,
        ImageSourceRole,
        DisplayDateRole,
        FoldersRole,
        TagsRole,
        BookmarkedRole,
        BookmarkColorRole
    };

    Q_ENUM(Role)

    explicit LibraryNoteListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int itemCount() const noexcept;
    int currentIndex() const noexcept;
    QString currentNoteId() const;
    QString currentNoteDirectoryPath() const;
    QString currentBodyText() const;
    Q_INVOKABLE void setCurrentIndex(int index);
    QString searchText() const;
    void setSearchText(const QString& text);
    bool strictValidation() const noexcept;
    void setStrictValidation(bool enabled);
    int correctionCount() const noexcept;
    QString lastValidationCode() const;
    QString lastValidationMessage() const;

    void setItems(QVector<LibraryNoteListItem> items);
    const QVector<LibraryNoteListItem>& items() const noexcept;

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
    void currentNoteDirectoryPathChanged();
    void currentBodyTextChanged();
    void searchTextChanged();
    void strictValidationChanged();
    void correctionCountChanged();
    void validationStateChanged();
    void validationIssueRaised(const QString& code, const QString& message, const QVariantMap& context);
    void itemCorrected(const QString& code, const QVariantMap& context);
    void modelHookRequested();

private:
    void applySearchFilter();
    void setValidationState(QString code, QString message);

    QVector<LibraryNoteListItem> m_sourceItems;
    QVector<LibraryNoteListItem> m_items;
    QString m_searchText;
    bool m_strictValidation = false;
    int m_correctionCount = 0;
    int m_currentIndex = -1;
    QString m_lastValidationCode;
    QString m_lastValidationMessage;
};
