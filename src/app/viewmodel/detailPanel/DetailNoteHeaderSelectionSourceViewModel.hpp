#pragma once

#include "viewmodel/detailPanel/session/WhatSonNoteHeaderSessionStore.hpp"

#include <QObject>
#include <QPointer>
#include <QVariantList>

class DetailNoteHeaderSelectionSourceViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QString noteId READ noteId WRITE setNoteId NOTIFY noteIdChanged)
    Q_PROPERTY(QString noteDirectoryPath READ noteDirectoryPath WRITE setNoteDirectoryPath NOTIFY noteDirectoryPathChanged)
    Q_PROPERTY(QObject* optionsSourceViewModel READ optionsSourceViewModel WRITE setOptionsSourceViewModel NOTIFY optionsSourceViewModelChanged)

public:
    enum class Field
    {
        Project,
        Bookmark,
        Progress
    };

    explicit DetailNoteHeaderSelectionSourceViewModel(Field field, QObject* parent = nullptr);

    QVariantList hierarchyModel() const;
    int selectedIndex() const noexcept;
    QString noteId() const;
    QString noteDirectoryPath() const;
    QObject* optionsSourceViewModel() const noexcept;

    void setSessionStore(WhatSonNoteHeaderSessionStore* sessionStore);
    void setNoteId(const QString& noteId);
    void setNoteDirectoryPath(const QString& noteDirectoryPath);
    void setOptionsSourceViewModel(QObject* optionsSourceViewModel);

    Q_INVOKABLE void setSelectedIndex(int index);

signals:
    void hierarchyModelChanged();
    void selectedIndexChanged();
    void noteIdChanged();
    void noteDirectoryPathChanged();
    void optionsSourceViewModelChanged();

private:
    void synchronize(bool reloadSession);
    int resolveSelectedIndexForHeader(const QVariantList& hierarchyModel) const;
    bool persistSelection(int index);
    QString entryLabelAt(int index) const;

    Field m_field;
    QPointer<WhatSonNoteHeaderSessionStore> m_sessionStore;
    QPointer<QObject> m_optionsSourceViewModel;
    QVariantList m_hierarchyModel;
    QString m_noteId;
    QString m_noteDirectoryPath;
    int m_selectedIndex = -1;
};
