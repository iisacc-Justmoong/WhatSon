#include "WhatSonNoteHeaderParser.hpp"

#include "../hierarchy/WhatSonFolderIdentity.hpp"
#include "WhatSonBookmarkColorPalette.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QRegularExpression>

namespace
{
    QString unescapeXmlText(QString value)
    {
        value.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        value.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        value.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        value.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        value.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return value;
    }

    QString normalizeSingleValue(QString value)
    {
        return unescapeXmlText(value.trimmed());
    }

    bool parseBooleanValue(const QString& rawValue, bool fallback)
    {
        const QString normalized = rawValue.trimmed().toCaseFolded();
        if (normalized.isEmpty())
        {
            return fallback;
        }

        if (normalized == QStringLiteral("1")
            || normalized == QStringLiteral("true")
            || normalized == QStringLiteral("yes")
            || normalized == QStringLiteral("on"))
        {
            return true;
        }

        if (normalized == QStringLiteral("0")
            || normalized == QStringLiteral("false")
            || normalized == QStringLiteral("no")
            || normalized == QStringLiteral("off"))
        {
            return false;
        }

        return fallback;
    }

    QString extractTagText(const QString& source, const QString& tagName)
    {
        const QRegularExpression regex(
            QStringLiteral(R"(<\s*%1\b[^>]*>([\s\S]*?)<\s*/\s*%1\s*>)")
            .arg(QRegularExpression::escape(tagName)),
            QRegularExpression::CaseInsensitiveOption);

        const QRegularExpressionMatch match = regex.match(source);
        if (!match.hasMatch())
        {
            return {};
        }

        return normalizeSingleValue(match.captured(1));
    }

    QStringList extractTagTexts(const QString& source, const QString& tagName)
    {
        const QRegularExpression regex(
            QStringLiteral(R"(<\s*%1\b[^>]*>([\s\S]*?)<\s*/\s*%1\s*>)")
            .arg(QRegularExpression::escape(tagName)),
            QRegularExpression::CaseInsensitiveOption);

        QStringList values;
        QRegularExpressionMatchIterator it = regex.globalMatch(source);
        while (it.hasNext())
        {
            const QRegularExpressionMatch match = it.next();
            values.push_back(normalizeSingleValue(match.captured(1)));
        }

        return values;
    }

    struct ParsedFolderBindings final
    {
        QStringList folders;
        QStringList folderUuids;
    };

    ParsedFolderBindings extractFolderBindings(const QString& source)
    {
        static const QRegularExpression regex(
            QStringLiteral(R"(<\s*folder\b([^>]*)>([\s\S]*?)<\s*/\s*folder\s*>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression uuidRegex(
            QStringLiteral(R"(\buuid\s*=\s*(?:\"([^\"]*)\"|'([^']*)'|([^\s/>]+)))"),
            QRegularExpression::CaseInsensitiveOption);

        ParsedFolderBindings bindings;
        QRegularExpressionMatchIterator it = regex.globalMatch(source);
        while (it.hasNext())
        {
            const QRegularExpressionMatch match = it.next();
            bindings.folders.push_back(normalizeSingleValue(match.captured(2)));

            const QRegularExpressionMatch uuidMatch = uuidRegex.match(match.captured(1));
            QString folderUuid;
            if (uuidMatch.hasMatch())
            {
                for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
                {
                    const QString captured = normalizeSingleValue(uuidMatch.captured(captureIndex));
                    if (!captured.isEmpty())
                    {
                        folderUuid = WhatSon::FolderIdentity::normalizeFolderUuid(captured);
                        break;
                    }
                }
            }
            bindings.folderUuids.push_back(folderUuid);
        }

        return bindings;
    }

    QString extractStartTagAttributes(const QString& source, const QString& tagName)
    {
        const QRegularExpression regex(
            QStringLiteral(R"(<\s*%1\b([^>]*)>)").arg(QRegularExpression::escape(tagName)),
            QRegularExpression::CaseInsensitiveOption);

        const QRegularExpressionMatch match = regex.match(source);
        if (!match.hasMatch())
        {
            return {};
        }

        return match.captured(1);
    }

    QString extractAttributeValue(
        const QString& source,
        const QString& tagName,
        const QString& attributeName)
    {
        const QString attributesText = extractStartTagAttributes(source, tagName);
        if (attributesText.isEmpty())
        {
            return {};
        }

        const QRegularExpression attrRegex(
            QStringLiteral("\\b%1\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s/>]+))")
            .arg(QRegularExpression::escape(attributeName)),
            QRegularExpression::CaseInsensitiveOption);

        const QRegularExpressionMatch match = attrRegex.match(attributesText);
        if (!match.hasMatch())
        {
            return {};
        }

        for (int index = 1; index <= 3; ++index)
        {
            const QString captured = match.captured(index);
            if (!captured.isNull())
            {
                return normalizeSingleValue(captured);
            }
        }

        return {};
    }

    QStringList parseProgressEnums(const QString& rawEnums)
    {
        QString value = rawEnums.trimmed();
        if (value.startsWith(QLatin1Char('{')) && value.endsWith(QLatin1Char('}')) && value.size() >= 2)
        {
            value = value.mid(1, value.size() - 2);
        }

        QStringList labels;
        const QStringList tokens = value.split(QLatin1Char(','), Qt::SkipEmptyParts);
        labels.reserve(tokens.size());

        for (QString token : tokens)
        {
            token = token.trimmed();
            if (token.startsWith(QLatin1Char('"')) && token.endsWith(QLatin1Char('"')) && token.size() >= 2)
            {
                token = token.mid(1, token.size() - 2).trimmed();
            }
            if (token.startsWith(QLatin1Char('\'')) && token.endsWith(QLatin1Char('\'')) && token.size() >= 2)
            {
                token = token.mid(1, token.size() - 2).trimmed();
            }
            if (!token.isEmpty())
            {
                labels.push_back(unescapeXmlText(token));
            }
        }

        return labels;
    }

    int parseProgressValue(const QString& source)
    {
        static const QRegularExpression progressTagRegex(
            QStringLiteral(R"(<\s*progress\b)"),
            QRegularExpression::CaseInsensitiveOption);
        if (!progressTagRegex.match(source).hasMatch())
        {
            return -1;
        }

        const QString progressText = extractTagText(source, QStringLiteral("progress"));
        bool ok = false;
        const int progressNumeric = progressText.toInt(&ok);
        if (ok)
        {
            return progressNumeric;
        }

        const QString valueAttr = extractAttributeValue(source, QStringLiteral("progress"), QStringLiteral("value"));
        if (!valueAttr.isEmpty())
        {
            const int valueNumeric = valueAttr.toInt(&ok);
            if (ok)
            {
                return valueNumeric;
            }
        }

        if (progressText.isEmpty() && valueAttr.isEmpty())
        {
            return -1;
        }

        const QString enumsAttr = extractAttributeValue(source, QStringLiteral("progress"), QStringLiteral("enums"));
        const QStringList enumLabels = parseProgressEnums(enumsAttr);

        if (!progressText.isEmpty())
        {
            for (int i = 0; i < enumLabels.size(); ++i)
            {
                if (QString::compare(progressText, enumLabels.at(i), Qt::CaseInsensitive) == 0)
                {
                    return i;
                }
            }
        }

        if (!valueAttr.isEmpty())
        {
            for (int i = 0; i < enumLabels.size(); ++i)
            {
                if (QString::compare(valueAttr, enumLabels.at(i), Qt::CaseInsensitive) == 0)
                {
                    return i;
                }
            }
        }

        return 0;
    }
} // namespace

WhatSonNoteHeaderParser::WhatSonNoteHeaderParser() = default;

WhatSonNoteHeaderParser::~WhatSonNoteHeaderParser() = default;

bool WhatSonNoteHeaderParser::parse(
    const QString& wsnHeadText,
    WhatSonNoteHeaderStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.parser"),
                              QStringLiteral("parse.begin"),
                              QStringLiteral("textLength=%1").arg(wsnHeadText.size()));

    if (outStore == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outStore must not be null.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("note.header.parser"),
                                  QStringLiteral("parse.failed"),
                                  QStringLiteral("outStore is null"));
        return false;
    }

    if (wsnHeadText.trimmed().isEmpty())
    {
        outStore->clear();
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wsnHeadText must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("note.header.parser"),
                                  QStringLiteral("parse.failed"),
                                  QStringLiteral("wsnHeadText is empty"));
        return false;
    }

    outStore->clear();
    outStore->setNoteId(extractAttributeValue(wsnHeadText, QStringLiteral("contents"), QStringLiteral("id")));
    outStore->setCreatedAt(extractTagText(wsnHeadText, QStringLiteral("created")));
    outStore->setAuthor(extractTagText(wsnHeadText, QStringLiteral("author")));
    outStore->setLastModifiedAt(extractTagText(wsnHeadText, QStringLiteral("lastModified")));
    outStore->setModifiedBy(extractTagText(wsnHeadText, QStringLiteral("modifiedBy")));
    const ParsedFolderBindings folderBindings = extractFolderBindings(wsnHeadText);
    outStore->setFolderBindings(folderBindings.folders, folderBindings.folderUuids);
    outStore->setProject(extractTagText(wsnHeadText, QStringLiteral("project")));
    outStore->setBookmarked(parseBooleanValue(
        extractAttributeValue(wsnHeadText, QStringLiteral("bookmarks"), QStringLiteral("state")),
        false));
    outStore->setBookmarkColors(WhatSon::Bookmarks::parseBookmarkColorsAttribute(
        extractAttributeValue(wsnHeadText, QStringLiteral("bookmarks"), QStringLiteral("colors"))));
    outStore->setTags(extractTagTexts(wsnHeadText, QStringLiteral("tag")));
    outStore->setProgress(parseProgressValue(wsnHeadText));

    QString isPresetValue = extractTagText(wsnHeadText, QStringLiteral("isPreset"));
    if (isPresetValue.isEmpty())
    {
        isPresetValue = extractAttributeValue(wsnHeadText, QStringLiteral("isPreset"), QStringLiteral("value"));
    }
    outStore->setPreset(parseBooleanValue(isPresetValue, false));

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.parser"),
                              QStringLiteral("parse.success"),
                              QStringLiteral(
                                  "id=%1 folderCount=%2 bookmarkColorCount=%3 tagCount=%4 progress=%5 bookmarked=%6 preset=%7")
                              .arg(outStore->noteId())
                              .arg(outStore->folders().size())
                              .arg(outStore->bookmarkColors().size())
                              .arg(outStore->tags().size())
                              .arg(outStore->progress())
                              .arg(outStore->isBookmarked() ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(outStore->isPreset() ? QStringLiteral("true") : QStringLiteral("false")));

    return true;
}
