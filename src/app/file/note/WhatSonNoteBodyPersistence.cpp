#include "WhatSonNoteBodyPersistence.hpp"

#include "WhatSonNoteHeaderCreator.hpp"
#include "WhatSonNoteHeaderParser.hpp"
#include "WhatSonNoteHeaderStore.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace
{
    constexpr auto kNoteTimestampFormat = "yyyy-MM-dd-hh-mm-ss";

    QString escapeXmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString normalizePath(QString path)
    {
        path = path.trimmed();
        if (path.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(path);
    }

    QString currentNoteTimestamp()
    {
        return QDateTime::currentDateTime().toString(QString::fromLatin1(kNoteTimestampFormat));
    }

    bool writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage = nullptr)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write file: %1").arg(filePath);
            }
            return false;
        }

        if (file.write(text.toUtf8()) < 0)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write file bytes: %1").arg(filePath);
            }
            return false;
        }
        return true;
    }

    QString serializeBodyDocument(const QString& noteId, const QString& plainText)
    {
        const QString normalizedId = noteId.trimmed().isEmpty()
                                         ? QStringLiteral("note")
                                         : escapeXmlText(noteId.trimmed());
        const QStringList lines = plainText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);

        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"") + normalizedId + QStringLiteral("\">\n");
        text += QStringLiteral("  <body>\n");
        if (plainText.isEmpty())
        {
            text += QStringLiteral("  </body>\n");
            text += QStringLiteral("</contents>\n");
            return text;
        }

        for (const QString& line : lines)
        {
            text += QStringLiteral("    <paragraph>") + escapeXmlText(line) + QStringLiteral("</paragraph>\n");
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    void updateHeaderLastModified(const QString& headerPath, QString* outLastModifiedAt = nullptr)
    {
        if (headerPath.isEmpty() || !QFileInfo(headerPath).isFile())
        {
            return;
        }

        QFile headerFile(headerPath);
        if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }

        const QString rawHeaderText = QString::fromUtf8(headerFile.readAll());
        headerFile.close();

        WhatSonNoteHeaderStore headerStore;
        WhatSonNoteHeaderParser headerParser;
        QString parseError;
        if (!headerParser.parse(rawHeaderText, &headerStore, &parseError))
        {
            Q_UNUSED(parseError);
            return;
        }

        const QString nextTimestamp = currentNoteTimestamp();
        headerStore.setLastModifiedAt(nextTimestamp);

        WhatSonNoteHeaderCreator headerCreator(QFileInfo(headerPath).absolutePath(), QString());
        const QString nextHeaderText = headerCreator.createHeaderText(headerStore);
        if (!writeUtf8File(headerPath, nextHeaderText))
        {
            return;
        }

        if (outLastModifiedAt != nullptr)
        {
            *outLastModifiedAt = nextTimestamp;
        }
    }
} // namespace

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return text;
    }

    QString firstLineFromBodyPlainText(const QString& text)
    {
        const QString normalizedText = normalizeBodyPlainText(text);
        const QStringList lines = normalizedText.split(QLatin1Char('\n'));
        for (const QString& line : lines)
        {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
            {
                return trimmed;
            }
        }
        return {};
    }

    QString resolveBodyPath(const QString& noteDirectoryPath)
    {
        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemBodyPath = noteDir.filePath(noteStem + QStringLiteral(".wsnbody"));
            if (QFileInfo(stemBodyPath).isFile())
            {
                return QDir::cleanPath(stemBodyPath);
            }
        }

        const QString canonicalBodyPath = noteDir.filePath(QStringLiteral("note.wsnbody"));
        if (QFileInfo(canonicalBodyPath).isFile())
        {
            return QDir::cleanPath(canonicalBodyPath);
        }

        const QFileInfoList bodyCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnbody")},
            QDir::Files,
            QDir::Name);
        QString draftBodyPath;
        for (const QFileInfo& fileInfo : bodyCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftBodyPath.isEmpty())
                {
                    draftBodyPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftBodyPath.isEmpty())
        {
            return QDir::cleanPath(draftBodyPath);
        }

        if (!noteStem.isEmpty())
        {
            return QDir::cleanPath(noteDir.filePath(noteStem + QStringLiteral(".wsnbody")));
        }
        return QDir::cleanPath(noteDir.filePath(QStringLiteral("note.wsnbody")));
    }

    QString resolveHeaderPath(const QString& noteHeaderPath, const QString& noteDirectoryPath)
    {
        const QString directPath = normalizePath(noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDir.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDir.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        return {};
    }

    bool persistBodyPlainText(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& noteHeaderPath,
        const QString& bodyPlainText,
        QString* outNormalizedBodyText,
        QString* outLastModifiedAt,
        QString* errorMessage)
    {
        const QString bodyPath = resolveBodyPath(noteDirectoryPath);
        if (bodyPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to resolve .wsnbody path.");
            }
            return false;
        }

        const QString normalizedBodyText = normalizeBodyPlainText(bodyPlainText);
        const QString bodyDocumentText = serializeBodyDocument(noteId, normalizedBodyText);
        if (!writeUtf8File(bodyPath, bodyDocumentText, errorMessage))
        {
            return false;
        }

        if (outNormalizedBodyText != nullptr)
        {
            *outNormalizedBodyText = normalizedBodyText;
        }

        QString updatedLastModifiedAt;
        updateHeaderLastModified(resolveHeaderPath(noteHeaderPath, noteDirectoryPath), &updatedLastModifiedAt);
        if (outLastModifiedAt != nullptr)
        {
            *outLastModifiedAt = updatedLastModifiedAt;
        }
        return true;
    }
} // namespace WhatSon::NoteBodyPersistence
