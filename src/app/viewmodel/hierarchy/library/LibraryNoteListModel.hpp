#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>

struct LibraryNoteListItem
{
    QString noteId;
    QString titleText;
    QString summaryText;
    QString foldersText;
    bool bookmarked = false;
    QString bookmarkColorHex;
    bool highlighted = false;
};

class LibraryNoteListModel final : public QAbstractListModel
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
        NoteIdRole = Qt::UserRole + 1,
        TitleTextRole,
        SummaryTextRole,
        FoldersTextRole,
        BookmarkedRole,
        BookmarkColorHexRole,
        HighlightedRole
    };

    Q_ENUM(Role)

    explicit LibraryNoteListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int itemCount() const noexcept;
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
    void strictValidationChanged();
    void correctionCountChanged();
    void validationStateChanged();
    void validationIssueRaised(const QString& code, const QString& message, const QVariantMap& context);
    void itemCorrected(const QString& code, const QVariantMap& context);
    void modelHookRequested();

private:
    void setValidationState(QString code, QString message);

    QVector<LibraryNoteListItem> m_items;
    bool m_strictValidation = false;
    int m_correctionCount = 0;
    QString m_lastValidationCode;
    QString m_lastValidationMessage;
};
