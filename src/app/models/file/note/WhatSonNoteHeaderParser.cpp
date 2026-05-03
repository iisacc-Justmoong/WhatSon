#include "app/models/file/note/WhatSonNoteHeaderParser.hpp"

#include "app/models/file/hierarchy/WhatSonFolderIdentity.hpp"
#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <iiXml.h>

#include <algorithm>
#include <string_view>
#include <vector>

namespace
{
    QString QStringFromUtf8View(std::string_view view)
    {
        return QString::fromUtf8(view.data(), static_cast<qsizetype>(view.size()));
    }

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

    bool tagNameEquals(const iiXml::Parser::TagNode& node, const QString& tagName)
    {
        return QString::compare(
                   QString::fromStdString(node.Range.TagName),
                   tagName,
                   Qt::CaseInsensitive)
            == 0;
    }

    bool fieldNameEquals(
        const iiXml::Parser::TagDocument& document,
        const iiXml::Parser::TagField& field,
        const QString& attributeName)
    {
        return QString::compare(
                   QStringFromUtf8View(document.FieldNameView(field)),
                   attributeName,
                   Qt::CaseInsensitive)
            == 0;
    }

    QString stripWsnHeadParserPreamble(QString source)
    {
        source = source.trimmed();

        if (source.startsWith(QStringLiteral("<?xml"), Qt::CaseInsensitive))
        {
            const qsizetype declarationEnd = source.indexOf(QStringLiteral("?>"));
            if (declarationEnd >= 0)
            {
                source = source.mid(declarationEnd + 2).trimmed();
            }
        }

        if (source.startsWith(QStringLiteral("<!DOCTYPE"), Qt::CaseInsensitive))
        {
            const qsizetype doctypeEnd = source.indexOf(QLatin1Char('>'));
            if (doctypeEnd >= 0)
            {
                source = source.mid(doctypeEnd + 1).trimmed();
            }
        }

        return source;
    }

    const iiXml::Parser::TagNode* findFirstDescendant(
        const std::vector<iiXml::Parser::TagNode>& nodes,
        const QString& tagName)
    {
        for (const iiXml::Parser::TagNode& node : nodes)
        {
            if (tagNameEquals(node, tagName))
            {
                return &node;
            }

            if (const iiXml::Parser::TagNode* child = findFirstDescendant(node.Children, tagName))
            {
                return child;
            }
        }

        return nullptr;
    }

    void collectDescendants(
        const std::vector<iiXml::Parser::TagNode>& nodes,
        const QString& tagName,
        std::vector<const iiXml::Parser::TagNode*>* outNodes)
    {
        if (outNodes == nullptr)
        {
            return;
        }

        for (const iiXml::Parser::TagNode& node : nodes)
        {
            if (tagNameEquals(node, tagName))
            {
                outNodes->push_back(&node);
            }
            collectDescendants(node.Children, tagName, outNodes);
        }
    }

    QString nodeText(
        const iiXml::Parser::TagDocument& document,
        const iiXml::Parser::TagNode* node)
    {
        if (node == nullptr)
        {
            return {};
        }

        return normalizeSingleValue(QStringFromUtf8View(document.ValueView(*node)));
    }

    QString attributeValue(
        const iiXml::Parser::TagDocument& document,
        const iiXml::Parser::TagNode* node,
        const QString& attributeName)
    {
        if (node == nullptr)
        {
            return {};
        }

        for (const iiXml::Parser::TagField& field : node->Fields)
        {
            if (!field.HasValue || !fieldNameEquals(document, field, attributeName))
            {
                continue;
            }

            return normalizeSingleValue(QStringFromUtf8View(document.FieldValueView(field)));
        }

        return {};
    }

    QString extractTagText(
        const iiXml::Parser::TagDocument& document,
        const QString& tagName)
    {
        return nodeText(document, findFirstDescendant(document.Nodes, tagName));
    }

    QStringList extractTagTexts(
        const iiXml::Parser::TagDocument& document,
        const QString& tagName)
    {
        std::vector<const iiXml::Parser::TagNode*> nodes;
        collectDescendants(document.Nodes, tagName, &nodes);

        QStringList values;
        values.reserve(static_cast<qsizetype>(nodes.size()));
        for (const iiXml::Parser::TagNode* node : nodes)
        {
            values.push_back(nodeText(document, node));
        }

        return values;
    }

    QString extractAttributeValue(
        const iiXml::Parser::TagDocument& document,
        const QString& tagName,
        const QString& attributeName)
    {
        return attributeValue(
            document,
            findFirstDescendant(document.Nodes, tagName),
            attributeName);
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

    struct ParsedFolderBindings final
    {
        QStringList folders;
        QStringList folderUuids;
    };

    ParsedFolderBindings extractFolderBindings(const iiXml::Parser::TagDocument& document)
    {
        std::vector<const iiXml::Parser::TagNode*> folderNodes;
        collectDescendants(document.Nodes, QStringLiteral("folder"), &folderNodes);
        ParsedFolderBindings bindings;
        for (const iiXml::Parser::TagNode* folderNode : folderNodes)
        {
            bindings.folders.push_back(nodeText(document, folderNode));

            const QString folderUuid = WhatSon::FolderIdentity::normalizeFolderUuid(
                attributeValue(document, folderNode, QStringLiteral("uuid")));
            bindings.folderUuids.push_back(folderUuid);
        }

        return bindings;
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

    int parseNonNegativeIntTagValue(const iiXml::Parser::TagDocument& document, const QString& tagName)
    {
        bool ok = false;
        const int value = extractTagText(document, tagName).toInt(&ok);
        return ok ? std::max(0, value) : 0;
    }

    int parseProgressValue(const iiXml::Parser::TagDocument& document)
    {
        const iiXml::Parser::TagNode* progressNode =
            findFirstDescendant(document.Nodes, QStringLiteral("progress"));
        if (progressNode == nullptr)
        {
            return -1;
        }

        const QString progressText = nodeText(document, progressNode);
        bool ok = false;
        const int progressNumeric = progressText.toInt(&ok);
        if (ok)
        {
            return progressNumeric;
        }

        const QString valueAttr = attributeValue(document, progressNode, QStringLiteral("value"));
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

        const QString enumsAttr = attributeValue(document, progressNode, QStringLiteral("enums"));
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

        return -1;
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

    const QString parseableHeaderText = stripWsnHeadParserPreamble(wsnHeadText);
    const QByteArray parseableHeaderBytes = parseableHeaderText.toUtf8();
    const iiXml::Parser::TagParser parser;
    const iiXml::Parser::TagDocumentResult parsedDocument =
        parser.ParseAllDocumentResult(
            std::string_view(parseableHeaderBytes.constData(), static_cast<std::size_t>(parseableHeaderBytes.size())));
    if (parsedDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed || !parsedDocument.Document.has_value())
    {
        outStore->clear();
        const QString diagnostic = QString::fromStdString(parsedDocument.Diagnostic.Reason);
        if (errorMessage != nullptr)
        {
            *errorMessage = diagnostic.isEmpty()
                                ? QStringLiteral("iiXml failed to parse .wsnhead document.")
                                : QStringLiteral("iiXml failed to parse .wsnhead document: %1").arg(diagnostic);
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("note.header.parser"),
                                  QStringLiteral("parse.failed"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const iiXml::Parser::TagDocument& document = parsedDocument.Document.value();
    outStore->clear();
    outStore->setNoteId(extractAttributeValue(document, QStringLiteral("contents"), QStringLiteral("id")));
    outStore->setCreatedAt(extractTagText(document, QStringLiteral("created")));
    outStore->setAuthor(extractTagText(document, QStringLiteral("author")));
    outStore->setLastModifiedAt(extractTagText(document, QStringLiteral("lastModified")));
    outStore->setLastOpenedAt(extractTagText(document, QStringLiteral("lastOpened")));
    outStore->setModifiedBy(extractTagText(document, QStringLiteral("modifiedBy")));
    const ParsedFolderBindings folderBindings = extractFolderBindings(document);
    outStore->setFolderBindings(folderBindings.folders, folderBindings.folderUuids);
    outStore->setProject(extractTagText(document, QStringLiteral("project")));
    outStore->setBookmarked(parseBooleanValue(
        extractAttributeValue(document, QStringLiteral("bookmarks"), QStringLiteral("state")),
        false));
    outStore->setBookmarkColors(WhatSon::Bookmarks::parseBookmarkColorsAttribute(
        extractAttributeValue(document, QStringLiteral("bookmarks"), QStringLiteral("colors"))));
    outStore->setTags(extractTagTexts(document, QStringLiteral("tag")));
    outStore->setTotalFolders(parseNonNegativeIntTagValue(document, QStringLiteral("totalFolders")));
    outStore->setTotalTags(parseNonNegativeIntTagValue(document, QStringLiteral("totalTags")));
    outStore->setLetterCount(parseNonNegativeIntTagValue(document, QStringLiteral("letterCount")));
    outStore->setWordCount(parseNonNegativeIntTagValue(document, QStringLiteral("wordCount")));
    outStore->setSentenceCount(parseNonNegativeIntTagValue(document, QStringLiteral("sentenceCount")));
    outStore->setParagraphCount(parseNonNegativeIntTagValue(document, QStringLiteral("paragraphCount")));
    outStore->setSpaceCount(parseNonNegativeIntTagValue(document, QStringLiteral("spaceCount")));
    outStore->setIndentCount(parseNonNegativeIntTagValue(document, QStringLiteral("indentCount")));
    outStore->setLineCount(parseNonNegativeIntTagValue(document, QStringLiteral("lineCount")));
    outStore->setOpenCount(parseNonNegativeIntTagValue(document, QStringLiteral("openCount")));
    outStore->setModifiedCount(parseNonNegativeIntTagValue(document, QStringLiteral("modifiedCount")));
    outStore->setBacklinkToCount(parseNonNegativeIntTagValue(document, QStringLiteral("backlinkToCount")));
    outStore->setBacklinkByCount(parseNonNegativeIntTagValue(document, QStringLiteral("backlinkByCount")));
    outStore->setIncludedResourceCount(
        parseNonNegativeIntTagValue(document, QStringLiteral("includedResourceCount")));
    outStore->setProgressEnums(parseProgressEnums(
        extractAttributeValue(document, QStringLiteral("progress"), QStringLiteral("enums"))));
    outStore->setProgress(parseProgressValue(document));

    QString isPresetValue = extractTagText(document, QStringLiteral("isPreset"));
    if (isPresetValue.isEmpty())
    {
        isPresetValue = extractAttributeValue(document, QStringLiteral("isPreset"), QStringLiteral("value"));
    }
    outStore->setPreset(parseBooleanValue(isPresetValue, false));

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.header.parser"),
                              QStringLiteral("parse.success"),
                              QStringLiteral(
                                  "id=%1 folderCount=%2 tagCount=%3 openCount=%4 modifiedCount=%5 backlinkTo=%6 backlinkBy=%7 progressEnumCount=%8 progress=%9 lastOpened=%10 bookmarked=%11 preset=%12")
                              .arg(outStore->noteId())
                              .arg(outStore->folders().size())
                              .arg(outStore->tags().size())
                              .arg(outStore->openCount())
                              .arg(outStore->modifiedCount())
                              .arg(outStore->backlinkToCount())
                              .arg(outStore->backlinkByCount())
                              .arg(outStore->progressEnums().size())
                              .arg(outStore->progress())
                              .arg(outStore->lastOpenedAt())
                              .arg(outStore->isBookmarked() ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(outStore->isPreset() ? QStringLiteral("true") : QStringLiteral("false")));

    return true;
}
