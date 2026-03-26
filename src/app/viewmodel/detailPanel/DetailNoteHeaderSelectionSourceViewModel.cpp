#include "DetailNoteHeaderSelectionSourceViewModel.hpp"

#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QVariantMap>

DetailNoteHeaderSelectionSourceViewModel::DetailNoteHeaderSelectionSourceViewModel(Field field, QObject* parent)
    : QObject(parent)
    , m_field(field)
{
}

QVariantList DetailNoteHeaderSelectionSourceViewModel::hierarchyModel() const
{
    return m_hierarchyModel;
}

int DetailNoteHeaderSelectionSourceViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

QString DetailNoteHeaderSelectionSourceViewModel::noteId() const
{
    return m_noteId;
}

QString DetailNoteHeaderSelectionSourceViewModel::noteDirectoryPath() const
{
    return m_noteDirectoryPath;
}

QObject* DetailNoteHeaderSelectionSourceViewModel::optionsSourceViewModel() const noexcept
{
    return m_optionsSourceViewModel.data();
}

void DetailNoteHeaderSelectionSourceViewModel::setSessionStore(WhatSonNoteHeaderSessionStore* sessionStore)
{
    m_sessionStore = sessionStore;
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceViewModel::setNoteId(const QString& noteId)
{
    const QString normalized = noteId.trimmed();
    if (m_noteId == normalized)
    {
        return;
    }
    m_noteId = normalized;
    emit noteIdChanged();
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceViewModel::setNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalized = noteDirectoryPath.trimmed();
    if (m_noteDirectoryPath == normalized)
    {
        return;
    }
    m_noteDirectoryPath = normalized;
    emit noteDirectoryPathChanged();
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceViewModel::setOptionsSourceViewModel(QObject* optionsSourceViewModel)
{
    if (m_optionsSourceViewModel == optionsSourceViewModel)
    {
        return;
    }
    m_optionsSourceViewModel = optionsSourceViewModel;
    emit optionsSourceViewModelChanged();
    synchronize(false);
}

void DetailNoteHeaderSelectionSourceViewModel::setSelectedIndex(int index)
{
    if (m_selectedIndex == index)
    {
        return;
    }
    if (!persistSelection(index))
    {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
}

void DetailNoteHeaderSelectionSourceViewModel::synchronize(bool reloadSession)
{
    QVariantList nextHierarchyModel;
    if (m_optionsSourceViewModel != nullptr)
    {
        nextHierarchyModel = m_optionsSourceViewModel->property("hierarchyModel").toList();
    }

    if (m_hierarchyModel != nextHierarchyModel)
    {
        m_hierarchyModel = nextHierarchyModel;
        emit hierarchyModelChanged();
    }

    if (m_sessionStore != nullptr && reloadSession && !m_noteId.isEmpty() && !m_noteDirectoryPath.isEmpty())
    {
        m_sessionStore->ensureLoaded(m_noteId, m_noteDirectoryPath, nullptr);
    }

    const int nextSelectedIndex = resolveSelectedIndexForHeader(m_hierarchyModel);
    if (m_selectedIndex != nextSelectedIndex)
    {
        m_selectedIndex = nextSelectedIndex;
        emit selectedIndexChanged();
    }
}

int DetailNoteHeaderSelectionSourceViewModel::resolveSelectedIndexForHeader(const QVariantList& hierarchyModel) const
{
    if (m_sessionStore == nullptr || m_noteId.isEmpty() || !m_sessionStore->hasEntry(m_noteId))
    {
        return -1;
    }

    const WhatSonNoteHeaderStore header = m_sessionStore->header(m_noteId);
    QString selectedLabel;
    switch (m_field)
    {
    case Field::Project:
        selectedLabel = header.project().trimmed();
        break;
    case Field::Bookmark:
        if (!header.bookmarkColors().isEmpty())
        {
            selectedLabel = WhatSon::Bookmarks::bookmarkDisplayNameForName(header.bookmarkColors().first());
        }
        break;
    case Field::Progress:
        switch (header.progress())
        {
        case 0: selectedLabel = QStringLiteral("Ready"); break;
        case 1: selectedLabel = QStringLiteral("Pending"); break;
        case 2: selectedLabel = QStringLiteral("InProgress"); break;
        case 3: selectedLabel = QStringLiteral("Done"); break;
        default: break;
        }
        break;
    }

    for (int index = 0; index < hierarchyModel.size(); ++index)
    {
        const QString label = hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
        if (label.compare(selectedLabel, Qt::CaseInsensitive) == 0)
        {
            return index;
        }
    }
    return -1;
}

bool DetailNoteHeaderSelectionSourceViewModel::persistSelection(int index)
{
    if (m_sessionStore == nullptr || m_noteId.isEmpty() || !m_sessionStore->hasEntry(m_noteId))
    {
        return false;
    }

    const QString label = entryLabelAt(index);
    switch (m_field)
    {
    case Field::Project:
        return m_sessionStore->updateProject(m_noteId, label, nullptr);
    case Field::Bookmark:
    {
        const QString colorName = label.trimmed().toLower();
        return m_sessionStore->updateBookmarked(
            m_noteId,
            !colorName.isEmpty(),
            colorName.isEmpty() ? QStringList{} : QStringList{colorName},
            nullptr);
    }
    case Field::Progress:
    {
        int progress = 0;
        if (label.compare(QStringLiteral("Pending"), Qt::CaseInsensitive) == 0)
            progress = 1;
        else if (label.compare(QStringLiteral("InProgress"), Qt::CaseInsensitive) == 0)
            progress = 2;
        else if (label.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0)
            progress = 3;
        return m_sessionStore->updateProgress(m_noteId, progress, nullptr);
    }
    }
    return false;
}

QString DetailNoteHeaderSelectionSourceViewModel::entryLabelAt(int index) const
{
    if (index < 0 || index >= m_hierarchyModel.size())
    {
        return {};
    }
    return m_hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
}
