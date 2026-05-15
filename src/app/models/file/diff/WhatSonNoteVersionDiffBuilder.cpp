#include "app/models/file/diff/WhatSonNoteVersionDiffBuilder.hpp"

#include <QDateTime>
#include <QStringList>

namespace
{
    QStringList linesForPatch(const QString& text)
    {
        if (text.isEmpty())
        {
            return {};
        }
        QString normalized = text;
        if (normalized.endsWith(QLatin1Char('\n')))
        {
            normalized.chop(1);
        }
        return normalized.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    }

    QString unifiedPatch(const QString& label, const QString& before, const QString& after)
    {
        const QStringList beforeLines = linesForPatch(before);
        const QStringList afterLines = linesForPatch(after);

        QString patch;
        patch += QStringLiteral("--- a/%1\n").arg(label);
        patch += QStringLiteral("+++ b/%1\n").arg(label);
        patch += QStringLiteral("@@ -1,%1 +1,%2 @@\n")
                     .arg(QString::number(beforeLines.size()), QString::number(afterLines.size()));
        for (const QString& line : beforeLines)
        {
            patch += QLatin1Char('-') + line + QLatin1Char('\n');
        }
        for (const QString& line : afterLines)
        {
            patch += QLatin1Char('+') + line + QLatin1Char('\n');
        }
        return patch;
    }
} // namespace

WhatSonNoteVersionDiffSegment WhatSonNoteVersionDiffBuilder::diffSegment(
    const QString& before,
    const QString& after,
    const QString& label) const
{
    WhatSonNoteVersionDiffSegment segment;
    const int prefixLimit = qMin(before.size(), after.size());
    while (segment.prefixLength < prefixLimit
           && before.at(segment.prefixLength) == after.at(segment.prefixLength))
    {
        ++segment.prefixLength;
    }

    const int suffixLimit = qMin(before.size(), after.size()) - segment.prefixLength;
    while (segment.suffixLength < suffixLimit
           && before.at(before.size() - 1 - segment.suffixLength)
                  == after.at(after.size() - 1 - segment.suffixLength))
    {
        ++segment.suffixLength;
    }

    const int removedLength = qMax(0, before.size() - segment.prefixLength - segment.suffixLength);
    const int insertedLength = qMax(0, after.size() - segment.prefixLength - segment.suffixLength);
    segment.removedText = before.mid(segment.prefixLength, removedLength);
    segment.insertedText = after.mid(segment.prefixLength, insertedLength);
    segment.unifiedPatch = unifiedPatch(label, before, after);
    segment.generatedAtUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    return segment;
}
