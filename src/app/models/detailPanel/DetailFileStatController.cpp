#include "app/models/detailPanel/DetailFileStatController.hpp"

#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"

#include <QVariantMap>

namespace
{
    QVariantMap buildMetricItem(const QString& key, const QString& label, const int value)
    {
        QVariantMap item;
        item.insert(QStringLiteral("key"), key);
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("value"), value);
        return item;
    }

    QString displayProjectName(const QString& project)
    {
        const QString trimmed = project.trimmed();
        return trimmed.isEmpty() ? QStringLiteral("No project") : trimmed;
    }

    QString displayDateText(const QString& value)
    {
        const QString trimmed = value.trimmed();
        return trimmed.isEmpty() ? QStringLiteral("yyyy-mm-dd") : trimmed;
    }

    QStringList displayFolderItems(const QStringList& folderPaths)
    {
        QStringList items;
        items.reserve(folderPaths.size());

        for (const QString& folderPath : folderPaths)
        {
            const QString leafName = WhatSon::NoteFolders::leafFolderName(folderPath);
            if (!leafName.isEmpty())
            {
                items.push_back(leafName);
                continue;
            }

            const QString displayFolderPath = WhatSon::NoteFolders::displayFolderPath(folderPath);
            if (!displayFolderPath.isEmpty())
            {
                items.push_back(displayFolderPath);
            }
        }

        return items;
    }

    QString joinDisplayItems(const QStringList& values)
    {
        const QStringList filtered = values;
        return filtered.isEmpty() ? QStringLiteral("-") : filtered.join(QStringLiteral(", "));
    }
}

DetailFileStatController::DetailFileStatController(QObject* parent)
    : DetailContentSectionController(DetailContentState::FileStat, parent)
{
}

int DetailFileStatController::totalFolders() const noexcept
{
    return m_header.totalFolders();
}

int DetailFileStatController::totalTags() const noexcept
{
    return m_header.totalTags();
}

int DetailFileStatController::letterCount() const noexcept
{
    return m_header.letterCount();
}

int DetailFileStatController::wordCount() const noexcept
{
    return m_header.wordCount();
}

int DetailFileStatController::sentenceCount() const noexcept
{
    return m_header.sentenceCount();
}

int DetailFileStatController::paragraphCount() const noexcept
{
    return m_header.paragraphCount();
}

int DetailFileStatController::spaceCount() const noexcept
{
    return m_header.spaceCount();
}

int DetailFileStatController::indentCount() const noexcept
{
    return m_header.indentCount();
}

int DetailFileStatController::lineCount() const noexcept
{
    return m_header.lineCount();
}

int DetailFileStatController::openCount() const noexcept
{
    return m_header.openCount();
}

int DetailFileStatController::modifiedCount() const noexcept
{
    return m_header.modifiedCount();
}

int DetailFileStatController::backlinkToCount() const noexcept
{
    return m_header.backlinkToCount();
}

int DetailFileStatController::backlinkByCount() const noexcept
{
    return m_header.backlinkByCount();
}

int DetailFileStatController::includedResourceCount() const noexcept
{
    return m_header.includedResourceCount();
}

QStringList DetailFileStatController::summaryLines() const
{
    const QStringList folders = displayFolderItems(m_header.folders());
    const QStringList tags = m_header.tags();

    return QStringList{
        QStringLiteral("Projects: %1").arg(displayProjectName(m_header.project())),
        QStringLiteral("Total folders: %1").arg(totalFolders()),
        QStringLiteral("Folders: %1").arg(joinDisplayItems(folders)),
        QStringLiteral("Total tags: %1").arg(totalTags()),
        QStringLiteral("Tags: %1").arg(joinDisplayItems(tags)),
        QStringLiteral("Created at: %1").arg(displayDateText(m_header.createdAt())),
        QStringLiteral("Modified at: %1").arg(displayDateText(m_header.lastModifiedAt()))};
}

QStringList DetailFileStatController::textMetricLines() const
{
    return QStringList{
        QStringLiteral("Letter: %1").arg(letterCount()),
        QStringLiteral("Word: %1").arg(wordCount()),
        QStringLiteral("Sentence: %1").arg(sentenceCount()),
        QStringLiteral("Paragraph: %1").arg(paragraphCount()),
        QStringLiteral("Space: %1").arg(spaceCount()),
        QStringLiteral("Indent: %1").arg(indentCount()),
        QStringLiteral("Line: %1").arg(lineCount())};
}

QStringList DetailFileStatController::activityLines() const
{
    return QStringList{
        QStringLiteral("Open count: %1").arg(openCount()),
        QStringLiteral("Modified count: %1").arg(modifiedCount()),
        QStringLiteral("Backlink to: %1").arg(backlinkToCount()),
        QStringLiteral("Backlink by: %1").arg(backlinkByCount()),
        QStringLiteral("Include resources: %1").arg(includedResourceCount()),
        QString()};
}

QVariantList DetailFileStatController::overviewItems() const
{
    return QVariantList{
        buildMetricItem(QStringLiteral("totalFolders"), QStringLiteral("Folders"), totalFolders()),
        buildMetricItem(QStringLiteral("totalTags"), QStringLiteral("Tags"), totalTags()),
        buildMetricItem(QStringLiteral("openCount"), QStringLiteral("Opens"), openCount()),
        buildMetricItem(QStringLiteral("modifiedCount"), QStringLiteral("Modified"), modifiedCount())};
}

QVariantList DetailFileStatController::textItems() const
{
    return QVariantList{
        buildMetricItem(QStringLiteral("letterCount"), QStringLiteral("Letters"), letterCount()),
        buildMetricItem(QStringLiteral("wordCount"), QStringLiteral("Words"), wordCount()),
        buildMetricItem(QStringLiteral("sentenceCount"), QStringLiteral("Sentences"), sentenceCount()),
        buildMetricItem(QStringLiteral("paragraphCount"), QStringLiteral("Paragraphs"), paragraphCount()),
        buildMetricItem(QStringLiteral("spaceCount"), QStringLiteral("Spaces"), spaceCount()),
        buildMetricItem(QStringLiteral("indentCount"), QStringLiteral("Indents"), indentCount()),
        buildMetricItem(QStringLiteral("lineCount"), QStringLiteral("Lines"), lineCount())};
}

QVariantList DetailFileStatController::relationItems() const
{
    return QVariantList{
        buildMetricItem(QStringLiteral("backlinkToCount"), QStringLiteral("Linked To"), backlinkToCount()),
        buildMetricItem(QStringLiteral("backlinkByCount"), QStringLiteral("Linked By"), backlinkByCount()),
        buildMetricItem(
            QStringLiteral("includedResourceCount"),
            QStringLiteral("Resources"),
            includedResourceCount())};
}

void DetailFileStatController::applyHeader(const WhatSonNoteHeaderStore& header)
{
    m_header = header;
    emit statsChanged();
}

void DetailFileStatController::clearHeader()
{
    m_header.clear();
    emit statsChanged();
}
