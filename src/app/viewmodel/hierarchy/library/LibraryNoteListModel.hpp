#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct LibraryNoteListItem
{
    QString noteId;
    QString titleText;
    QString summaryText;
    QString foldersText;
    bool bookmarked = false;
    bool highlighted = false;
};

class LibraryNoteListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        NoteIdRole = Qt::UserRole + 1,
        TitleTextRole,
        SummaryTextRole,
        FoldersTextRole,
        BookmarkedRole,
        HighlightedRole
    };

    Q_ENUM(Role)

    explicit LibraryNoteListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QVector<LibraryNoteListItem> items);
    const QVector<LibraryNoteListItem>& items() const noexcept;

private:
    QVector<LibraryNoteListItem> m_items;
};
