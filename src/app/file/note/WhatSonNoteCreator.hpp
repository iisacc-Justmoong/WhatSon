#pragma once

#include <QString>
#include <QStringList>

class WhatSonNoteCreator
{
public:
    explicit WhatSonNoteCreator(QString workspaceRootPath, QString notesRootPath = QStringLiteral("notes"));
    virtual ~WhatSonNoteCreator();

    WhatSonNoteCreator(const WhatSonNoteCreator&) = delete;
    WhatSonNoteCreator& operator=(const WhatSonNoteCreator&) = delete;
    WhatSonNoteCreator(WhatSonNoteCreator&&) = default;
    WhatSonNoteCreator& operator=(WhatSonNoteCreator&&) = default;

    void setWorkspaceRootPath(QString workspaceRootPath);
    const QString& workspaceRootPath() const noexcept;

    void setNotesRootPath(QString notesRootPath);
    const QString& notesRootPath() const noexcept;

    virtual QString creatorName() const = 0;
    virtual QString targetPathForNote(const QString& noteId) const = 0;
    virtual QStringList requiredRelativePaths() const = 0;

protected:
    QString noteDirectoryPath(const QString& noteId) const;
    QString joinPath(const QString& left, const QString& right) const;

private:
    QString m_workspaceRootPath;
    QString m_notesRootPath;
};
