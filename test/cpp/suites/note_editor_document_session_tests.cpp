#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

namespace
{
    QString readUtf8FileForNoteEditorSessionTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    bool writeUtf8FileForNoteEditorSessionTest(const QString& path, const QString& text)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        file.write(text.toUtf8());
        return true;
    }

    int calloutChromeHeightForNoteEditorSessionTest(const QString& editorDocumentText)
    {
        static const QRegularExpression heightPattern(
            QStringLiteral("data-callout-frame-chrome-height=\"(\\d+)\""));
        const QRegularExpressionMatch match = heightPattern.match(editorDocumentText);
        return match.hasMatch() ? match.captured(1).toInt() : 0;
    }

    bool writeFilesystemBodyForIdlePullSessionTest(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& bodySourceText,
        const QString& lastModifiedAt,
        QString* errorMessage)
    {
        WhatSonLocalNoteFileStore fileStore;
        WhatSonLocalNoteDocument document;
        WhatSonLocalNoteFileStore::ReadRequest readRequest;
        readRequest.noteId = noteId;
        readRequest.noteDirectoryPath = noteDirectoryPath;
        if (!fileStore.readNote(readRequest, &document, errorMessage))
        {
            return false;
        }

        document.bodyPlainText = bodySourceText;
        document.bodySourceText = bodySourceText;

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = document;
        updateRequest.persistHeader = false;
        updateRequest.persistBody = true;
        updateRequest.touchLastModified = true;
        updateRequest.resolveTimestampConflicts = false;
        updateRequest.incomingLastModifiedAt = lastModifiedAt;
        return fileStore.updateNote(updateRequest, nullptr, errorMessage);
    }

    QVariantList importClipboardImageEntriesForNoteEditorSessionTest(
        const QString& hubPath,
        QString* errorMessage)
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }

        QImage clipboardImage(QSize(1600, 900), QImage::Format_ARGB32_Premultiplied);
        clipboardImage.fill(qRgba(18, 110, 190, 255));

        InAppClipboardManager clipboard;
        if (!clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = clipboard.lastError();
            }
            return {};
        }
        clipboard.setCurrentHubPath(hubPath);

        const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
        if (importedEntries.isEmpty() && errorMessage != nullptr)
        {
            *errorMessage = clipboard.lastError();
        }
        return importedEntries;
    }
} // namespace

void WhatSonCppRegressionTests::noteEditorDocumentSession_mountsEditorHtmlFileAndPersistsBodyDocument()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("session-note"),
        QStringLiteral("Visible line\n<resource path=\"asset.png\" />"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("session-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorFilePath = session.editorFilePath();
    QVERIFY(!editorFilePath.isEmpty());
    QVERIFY(!editorFilePath.endsWith(QStringLiteral(".wsnbody")));
    QVERIFY(QFileInfo(editorFilePath).isFile());
    QCOMPARE(session.activeNoteId(), QStringLiteral("session-note"));
    QCOMPARE(session.activeNoteDirectoryPath(), QDir::cleanPath(noteDirectoryPath));
    QCOMPARE(session.parsedLineCount(), 2);
    QVERIFY(!session.readOnly());

    const QString mountedEditorSource = readUtf8FileForNoteEditorSessionTest(editorFilePath);
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<?xml")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<contents")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("Visible line")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("&lt;resource path=&quot;asset.png&quot; /&gt;")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("Visible line\n<resource")));

    const QString editedSource = QStringLiteral("Changed line\nInserted line\n<resource path=\"asset.png\" />");
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, editedSource));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(editorFilePath));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);
    QCOMPARE(session.parsedLineCount(), 3);

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(bodyPath);
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<contents id=\"session-note\">")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph>Changed line</paragraph>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph>Inserted line</paragraph>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<resource path=\"asset.png\" />")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        editedSource);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_pushesSurfaceTextToRawOnIdleRequest()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("idle-raw-push-note"),
        QStringLiteral("Before idle"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("idle-raw-push-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(
        session.editorFilePath(),
        QStringLiteral("After idle push"));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("After idle push"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_pushesQtSerializedCalloutToRawOnIdleRequest()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("idle-callout-raw-push-note"),
        QStringLiteral("<callout>Before idle callout</callout>"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("idle-callout-raw-push-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("idle-callout-raw-push-note"),
        QStringLiteral("<callout>After idle <bold>callout</bold></callout>"));
    QTextDocument roundTrippedEditorDocument;
    roundTrippedEditorDocument.setHtml(editorHtml);
    const QString roundTrippedEditorHtml = roundTrippedEditorDocument.toHtml();
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("whatson-callout-source:")));
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("data-callout-content")));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(
        session.editorFilePath(),
        roundTrippedEditorHtml);
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QVERIFY(persistedBodyDocument.contains(
        QStringLiteral("<paragraph><callout>After idle <bold>callout</bold></callout></paragraph>")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("<callout>After idle <bold>callout</bold></callout>"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_pushesSurfaceTextToRawOnModifiedCountIncrease()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("modified-raw-push-note"),
        QStringLiteral("Before modified count"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("modified-raw-push-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(
        session.editorFilePath(),
        1,
        QStringLiteral("After modified count push"));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("After modified count push"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_pushesSurfaceTextToRawOnNoteDeparture()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("departure-raw-push-note"),
        QStringLiteral("Before departure"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("departure-raw-push-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));
    session.requestEditorModifiedCountRawPush(
        session.editorFilePath(),
        1,
        QStringLiteral("After departure push"));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.clearEditor());
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("After departure push"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_emitsHubFilesystemMutationForVersionDiffPush()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("session-sync-note"),
        QStringLiteral("Before sync"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("session-sync-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    QVERIFY(writeUtf8FileForNoteEditorSessionTest(
        session.editorFilePath(),
        QStringLiteral("After sync")));

    QSignalSpy filesystemMutationSpy(&session, &NoteEditorDocumentSession::hubFilesystemMutated);
    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(session.editorFilePath()));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);
    QTRY_COMPARE_WITH_TIMEOUT(filesystemMutationSpy.count(), 1, 3000);

    const QString versionPath = QDir(noteDirectoryPath).filePath(QStringLiteral("session-sync-note.wsnversion"));
    const QString versionText = readUtf8FileForNoteEditorSessionTest(versionPath);
    QVERIFY(versionText.contains(QStringLiteral("\"generatedAtUtc\"")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_routesOpenPullThroughSyncController()
{
    const QString sessionHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/NoteEditorDocumentSession.hpp"));
    const QString sessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/NoteEditorDocumentSession.cpp"));

    QVERIFY(sessionHeader.contains(QStringLiteral("WhatSonEditorRawPullController")));
    QVERIFY(sessionSource.contains(QStringLiteral("m_rawPullController.setRawPullCallback(")));
    QVERIFY(sessionSource.contains(QStringLiteral("m_rawPullController.requestNoteOpenPull(")));
    QVERIFY(sessionSource.contains(QStringLiteral("m_rawPullController.setActiveNoteForIdlePull(")));
    QVERIFY(sessionSource.contains(QStringLiteral("m_rawPullController.requestActiveIdlePull(")));
    QVERIFY(sessionSource.contains(QStringLiteral("isTimestampNewer(lastModifiedAt")));
    QVERIFY(!sessionSource.contains(
        QStringLiteral("m_pendingLoadSequence = m_noteManagementCoordinator.loadNoteBodyTextForNote(")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_pullsOnlyNewerFilesystemBodyOnIdle()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("idle-pull-session-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("Base body"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    const QString editorFilePath = session.editorFilePath();
    QVERIFY(readUtf8FileForNoteEditorSessionTest(editorFilePath).contains(QStringLiteral("Base body")));

    QString updateError;
    QVERIFY2(
        writeFilesystemBodyForIdlePullSessionTest(
            noteId,
            noteDirectoryPath,
            QStringLiteral("Filesystem newer body"),
            QStringLiteral("2099-05-01-12-00-00"),
            &updateError),
        qPrintable(updateError));

    QSignalSpy pulledSpy(&session, &NoteEditorDocumentSession::editorDocumentTextPulled);
    QVERIFY(session.requestActiveNoteIdleRawPull() != 0);
    QTRY_COMPARE_WITH_TIMEOUT(pulledSpy.count(), 1, 3000);
    QVERIFY(readUtf8FileForNoteEditorSessionTest(editorFilePath).contains(
        QStringLiteral("Filesystem newer body")));

    QVERIFY2(
        writeFilesystemBodyForIdlePullSessionTest(
            noteId,
            noteDirectoryPath,
            QStringLiteral("Filesystem stale body"),
            QStringLiteral("2000-05-01-12-00-00"),
            &updateError),
        qPrintable(updateError));

    QSignalSpy ignoredSpy(&session, &NoteEditorDocumentSession::editorFilesystemPullIgnored);
    QVERIFY(session.requestActiveNoteIdleRawPull() != 0);
    QTRY_COMPARE_WITH_TIMEOUT(ignoredSpy.count(), 1, 3000);
    QCOMPARE(ignoredSpy.takeFirst().at(1).toString(), QStringLiteral("not-newer"));
    QCOMPARE(pulledSpy.count(), 1);
    QVERIFY(readUtf8FileForNoteEditorSessionTest(editorFilePath).contains(
        QStringLiteral("Filesystem newer body")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_keepsSessionSourceWhenSameNoteIsReselected()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("reselected-note"),
        QStringLiteral("Persisted line"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("reselected-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorFilePath = session.editorFilePath();
    QVERIFY(!editorFilePath.isEmpty());
    QCOMPARE(session.parsedLineCount(), 1);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, QStringLiteral("Unsaved session line")));

    QVERIFY(session.openNoteForEditing(QStringLiteral("reselected-note"), noteDirectoryPath));
    QCOMPARE(session.editorFilePath(), editorFilePath);
    QCOMPARE(readUtf8FileForNoteEditorSessionTest(editorFilePath), QStringLiteral("Unsaved session line"));
    QCOMPARE(loadedSpy.count(), 1);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_incrementsOpenCountAfterSuccessfulOpen()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("opened-session-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("Open count body"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    auto readHeader = [&fileStore, &readRequest]() -> WhatSonNoteHeaderStore
    {
        WhatSonLocalNoteDocument document;
        QString readError;
        if (!fileStore.readNote(readRequest, &document, &readError))
        {
            return {};
        }
        return document.headerStore;
    };
    auto openCount = [&readHeader]() -> int
    {
        return readHeader().openCount();
    };

    QTRY_COMPARE_WITH_TIMEOUT(openCount(), 1, 3000);
    QVERIFY(!readHeader().lastOpenedAt().isEmpty());
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_buildsInlineFormatSourceInsertion()
{
    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        QStringLiteral("Alpha Beta\nGamma"));

    const auto verifyInlineInsertion =
        [&session, &editorHtml](const QString& tagName, const QString& expectedSource, const QString& expectedHtml)
    {
        const QVariantMap result = session.insertFormatTagIntoSource(
            tagName,
            editorHtml,
            6,
            4);

        QVERIFY(result.value(QStringLiteral("valid")).toBool());
        QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
        QCOMPARE(result.value(QStringLiteral("bodySourceText")).toString(), expectedSource);
        QVERIFY(result.value(QStringLiteral("editorDocumentText")).toString().contains(expectedHtml));
        QCOMPARE(
            result.value(QStringLiteral("sourceCursorPosition")).toInt(),
            expectedSource.left(expectedSource.indexOf(QLatin1Char('\n'))).size());
        QCOMPARE(
            result.value(QStringLiteral("cursorPosition")).toInt(),
            QStringLiteral("Alpha Beta").size());
    };

    verifyInlineInsertion(
        QStringLiteral("bold"),
        QStringLiteral("Alpha <bold>Beta</bold>\nGamma"),
        QStringLiteral("<strong style=\"font-weight:900;\">Beta</strong>"));
    verifyInlineInsertion(
        QStringLiteral("italic"),
        QStringLiteral("Alpha <italic>Beta</italic>\nGamma"),
        QStringLiteral("<span style=\"font-style:italic;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("underline"),
        QStringLiteral("Alpha <underline>Beta</underline>\nGamma"),
        QStringLiteral("<span style=\"text-decoration: underline;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("strikethrough"),
        QStringLiteral("Alpha <strikethrough>Beta</strikethrough>\nGamma"),
        QStringLiteral("<span style=\"text-decoration: line-through;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("highlight"),
        QStringLiteral("Alpha <highlight>Beta</highlight>\nGamma"),
        QStringLiteral("<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">Beta</span>"));

    const QVariantMap selectedCalloutResult = session.insertFormatTagIntoSource(
        QStringLiteral("callout"),
        editorHtml,
        6,
        4);
    QVERIFY(selectedCalloutResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        selectedCalloutResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <callout>Beta</callout>\nGamma"));
    QVERIFY(selectedCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(selectedCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(selectedCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("Beta")));
    QVERIFY(!selectedCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("<callout>")));
    QCOMPARE(
        selectedCalloutResult.value(QStringLiteral("sourceCursorPosition")).toInt(),
        QStringLiteral("Alpha <callout>Beta</callout>").size());

    const QVariantMap emptyCalloutResult = session.insertFormatTagIntoSource(
        QStringLiteral("callout"),
        editorHtml,
        5,
        0);
    QVERIFY(emptyCalloutResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        emptyCalloutResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha<callout></callout> Beta\nGamma"));
    QVERIFY(emptyCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(emptyCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(!emptyCalloutResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("<callout>")));
    QCOMPARE(
        emptyCalloutResult.value(QStringLiteral("sourceCursorPosition")).toInt(),
        QStringLiteral("Alpha<callout>").size());

    const QString formattedEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        QStringLiteral("<bold>Alpha</bold> Beta Gamma"));
    const QVariantMap offsetMappedResult = session.insertFormatTagIntoSource(
        QStringLiteral("italic"),
        formattedEditorHtml,
        6,
        4);
    QVERIFY(offsetMappedResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        offsetMappedResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold>Alpha</bold> <italic>Beta</italic> Gamma"));
    QCOMPARE(offsetMappedResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha Beta").size());

    const QVariantMap nestedFormatResult = session.insertFormatTagIntoSource(
        QStringLiteral("italic"),
        formattedEditorHtml,
        0,
        5);
    QVERIFY(nestedFormatResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        nestedFormatResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold><italic>Alpha</italic></bold> Beta Gamma"));
    QCOMPARE(nestedFormatResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha").size());

    const QVariantMap toggledFormatResult = session.insertFormatTagIntoSource(
        QStringLiteral("bold"),
        formattedEditorHtml,
        0,
        5);
    QVERIFY(toggledFormatResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        toggledFormatResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha Beta Gamma"));
    QCOMPARE(toggledFormatResult.value(QStringLiteral("toggledOff")).toBool(), true);
    QCOMPARE(toggledFormatResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha").size());

    const QVariantMap breakResult = session.insertFormatTagIntoSource(
        QStringLiteral("break"),
        editorHtml,
        5,
        0);

    QVERIFY(breakResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        breakResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha\n</break>\n Beta\nGamma"));
    QVERIFY(breakResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("<br/>")));

    const QVariantMap titleStyleResult = session.insertStyleTagIntoSource(
        QStringLiteral("Title"),
        editorHtml,
        6,
        4);
    QVERIFY(titleStyleResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        titleStyleResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <style style=\"Title\">Beta</style>\nGamma"));
    QVERIFY(titleStyleResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("font-size:26px;")));
    QCOMPARE(
        titleStyleResult.value(QStringLiteral("sourceCursorPosition")).toInt(),
        QStringLiteral("Alpha <style style=\"Title\">Beta</style>").size());

    const QVariantMap currentLineStyleResult = session.insertStyleTagIntoSource(
        QStringLiteral("Subtitle"),
        editorHtml,
        2,
        0);
    QVERIFY(currentLineStyleResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        currentLineStyleResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<style style=\"Subtitle\">Alpha Beta</style>\nGamma"));
    QVERIFY(currentLineStyleResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("font-size:15px;")));
    QVERIFY(!currentLineStyleResult.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("<style style=\"Subtitle\"></style>")));
    QCOMPARE(currentLineStyleResult.value(QStringLiteral("editorSelectionStart")).toInt(), 0);
    QCOMPARE(currentLineStyleResult.value(QStringLiteral("editorSelectionLength")).toInt(), QStringLiteral("Alpha Beta").size());

    const QVariantMap bodyStyleResult = session.insertStyleTagIntoSource(
        QStringLiteral("Body"),
        editorHtml,
        6,
        4);
    QVERIFY(bodyStyleResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        bodyStyleResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <style>Beta</style>\nGamma"));
    QVERIFY(bodyStyleResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("font-size:12px;")));
    QVERIFY(!bodyStyleResult.value(QStringLiteral("openingToken")).toString().contains(QStringLiteral("style=\"Body\"")));

    const QVariantMap invalidStyleResult = session.insertStyleTagIntoSource(
        QStringLiteral("Unknown"),
        editorHtml,
        6,
        4);
    QVERIFY(!invalidStyleResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        invalidStyleResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha Beta\nGamma"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_buildsStyleFontSourceInsertion()
{
    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("style-font-note"),
        QStringLiteral("Alpha Beta\nGamma"));

    const QVariantMap selectedFontResult = session.insertStyleFontTagIntoSource(
        QStringLiteral("Menlo"),
        editorHtml,
        6,
        4);
    QVERIFY(selectedFontResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        selectedFontResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <style font=\"Menlo\">Beta</style>\nGamma"));
    QVERIFY(selectedFontResult.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("font-family:Menlo;")));
    QCOMPARE(selectedFontResult.value(QStringLiteral("fontFamily")).toString(), QStringLiteral("Menlo"));

    const QVariantMap currentLineFontResult = session.insertStyleFontTagIntoSource(
        QStringLiteral("Noto Sans CJK KR"),
        editorHtml,
        2,
        0);
    QVERIFY(currentLineFontResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        currentLineFontResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<style font=\"Noto Sans CJK KR\">Alpha Beta</style>\nGamma"));
    QVERIFY(currentLineFontResult.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("font-family:Noto Sans CJK KR;")));
    QCOMPARE(currentLineFontResult.value(QStringLiteral("editorSelectionStart")).toInt(), 0);
    QCOMPARE(currentLineFontResult.value(QStringLiteral("editorSelectionLength")).toInt(), QStringLiteral("Alpha Beta").size());

    const QVariantMap escapedFontResult = session.insertStyleFontTagIntoSource(
        QStringLiteral("A&B \"Serif\""),
        editorHtml,
        6,
        4);
    QVERIFY(escapedFontResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(escapedFontResult.value(QStringLiteral("bodySourceText")).toString().contains(
        QStringLiteral("<style font=\"A&amp;B &quot;Serif&quot;\">Beta</style>")));
    QVERIFY(escapedFontResult.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("font-family:A&amp;B &quot;Serif&quot;;")));

    const QVariantMap invalidFontResult = session.insertStyleFontTagIntoSource(
        QStringLiteral(""),
        editorHtml,
        6,
        4);
    QVERIFY(!invalidFontResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        invalidFontResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha Beta\nGamma"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_backspaceAtCalloutInitRemovesCalloutWrapper()
{
    NoteEditorDocumentSession session;
    const QString sourceText =
        QStringLiteral("Before\n"
                       "<callout>Inside</callout>\n"
                       "After");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-boundary-note"),
        sourceText);
    const int calloutInitCursor = QStringLiteral("Before\n").size();

    const QVariantMap result = session.handleCalloutBoundaryKeyInSource(
        editorHtml,
        calloutInitCursor,
        0,
        Qt::Key_Backspace);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QVERIFY(result.value(QStringLiteral("handled")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Before\nInside\nAfter"));
    QCOMPARE(result.value(QStringLiteral("cursorPosition")).toInt(), calloutInitCursor);
    QVERIFY(!result.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("whatson-callout")));

    const QString emptySourceText =
        QStringLiteral("Before\n"
                       "<callout></callout>\n"
                       "After");
    const QString emptyEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("empty-callout-boundary-note"),
        emptySourceText);
    const QVariantMap emptyResult = session.handleCalloutBoundaryKeyInSource(
        emptyEditorHtml,
        calloutInitCursor,
        0,
        Qt::Key_Backspace);

    QVERIFY(emptyResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(emptyResult.value(QStringLiteral("handled")).toBool());
    QCOMPARE(emptyResult.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(
        emptyResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Before\nAfter"));
    QCOMPARE(emptyResult.value(QStringLiteral("cursorPosition")).toInt(), calloutInitCursor);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_calloutFrameChromeDoesNotCreateExtraEditorLine()
{
    const QString sourceText =
        QStringLiteral("Before\n"
                       "<callout>Inside</callout>\n"
                       "After");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-enter-before-chrome-note"),
        sourceText);
    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QString editorPlainText = editorDocument.toPlainText();
    editorPlainText.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(editorPlainText, QStringLiteral("Before\nInside\nAfter"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_enterInsideCalloutMovesCursorOutside()
{
    NoteEditorDocumentSession session;
    const QString sourceText =
        QStringLiteral("Before\n"
                       "<callout>Inside</callout>\n"
                       "After");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-enter-note"),
        sourceText);
    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    const QString editorPlainText = editorDocument.toPlainText();
    QString editorVisiblePlainText = editorPlainText;
    editorVisiblePlainText.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(editorVisiblePlainText, QStringLiteral("Before\nInside\nAfter"));
    const int renderedCalloutTextEndCursor =
        editorPlainText.indexOf(QStringLiteral("Inside")) + QStringLiteral("Inside").size();

    const QVariantMap result = session.handleCalloutBoundaryKeyInSource(
        editorHtml,
        renderedCalloutTextEndCursor,
        0,
        Qt::Key_Return);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QVERIFY(result.value(QStringLiteral("handled")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), false);
    QCOMPARE(result.value(QStringLiteral("bodySourceText")).toString(), sourceText);
    QTextDocument resultEditorDocument;
    resultEditorDocument.setHtml(result.value(QStringLiteral("editorDocumentText")).toString());
    QCOMPARE(
        result.value(QStringLiteral("cursorPosition")).toInt(),
        resultEditorDocument.toPlainText().indexOf(QStringLiteral("After")));

    const QString trailingCalloutSourceText = QStringLiteral("<callout>Inside</callout>");
    const QString trailingCalloutEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("trailing-callout-enter-note"),
        trailingCalloutSourceText);
    QTextDocument trailingCalloutEditorDocument;
    trailingCalloutEditorDocument.setHtml(trailingCalloutEditorHtml);
    const QString trailingEditorPlainText = trailingCalloutEditorDocument.toPlainText();
    QString trailingEditorVisiblePlainText = trailingEditorPlainText;
    trailingEditorVisiblePlainText.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(trailingEditorVisiblePlainText, QStringLiteral("Inside"));
    const QVariantMap trailingResult = session.handleCalloutBoundaryKeyInSource(
        trailingCalloutEditorHtml,
        trailingEditorPlainText.indexOf(QStringLiteral("Inside")) + QStringLiteral("Inside").size(),
        0,
        Qt::Key_Enter);

    QVERIFY(trailingResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(trailingResult.value(QStringLiteral("handled")).toBool());
    QCOMPARE(trailingResult.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(
        trailingResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<callout>Inside</callout>\n"));
    QTextDocument trailingResultEditorDocument;
    trailingResultEditorDocument.setHtml(trailingResult.value(QStringLiteral("editorDocumentText")).toString());
    const QString trailingResultPlainText = trailingResultEditorDocument.toPlainText();
    QString trailingResultVisiblePlainText = trailingResultPlainText;
    trailingResultVisiblePlainText.remove(QChar::ObjectReplacementCharacter);
    trailingResultVisiblePlainText.remove(QChar(0x200B));
    QCOMPARE(trailingResultVisiblePlainText, QStringLiteral("Inside\n"));
    QCOMPARE(
        trailingResult.value(QStringLiteral("cursorPosition")).toInt(),
        trailingResultPlainText.indexOf(QChar(0x200B)));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_projectsBreakSourceLineWithoutLiteralTagText()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("break-projection-note"),
        QStringLiteral("Alpha\n</break>\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("break-projection-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString mountedEditorSource = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QTextDocument mountedEditorDocument;
    mountedEditorDocument.setHtml(mountedEditorSource);
    QCOMPARE(mountedEditorDocument.toPlainText(), QStringLiteral("Alpha\n\nBeta"));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("</break>")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("&lt;/break&gt;")));
    QCOMPARE(session.parsedLineCount(), 3);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_usesSelectedTextToRepairDriftedFormatSelection()
{
    NoteEditorDocumentSession session;
    const QString selectedParagraph =
        QStringLiteral("병렬 에이전트 운용에서 가장 실용적인 최소 세트는 다음이다.");
    const QString bodySource = selectedParagraph + QStringLiteral("\n\ngit checkout main");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        bodySource);

    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        editorHtml,
        QStringLiteral("병렬 에이전").size(),
        selectedParagraph.size(),
        selectedParagraph);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<highlight>병렬 에이전트 운용에서 가장 실용적인 최소 세트는 다음이다.</highlight>\n\ngit checkout main"));
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), 0);
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), selectedParagraph.size());
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("git </highlight>")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_mapsLogicalSelectionAgainstLoadedBodySourceBreaks()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("logical-selection-note"),
        QStringLiteral("Alpha Beta Gamma"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(
        bodyPath,
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=\"logical-selection-note\">\n"
            "  <body>\n"
            "    <paragraph>Alpha <next />Beta Gamma</paragraph>\n"
            "  </body>\n"
            "</contents>\n")));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("logical-selection-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("logical-selection-note"),
        QStringLiteral("Alpha <next />Beta Gamma"));
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        editorHtml,
        QStringLiteral("Alpha \n").size(),
        QStringLiteral("Beta").size(),
        QStringLiteral("Beta"));

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <next /><highlight>Beta</highlight> Gamma"));
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), QStringLiteral("Alpha <next />").size());
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), QStringLiteral("Beta").size());
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("B<highlight>eta ")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_formatsSelectionAgainstBodySourceWhenEditorHtmlDropsBlankLines()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("blank-line-selection-note"),
        QStringLiteral("Intro\n\n여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.\n\ngit checkout main"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("blank-line-selection-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QCOMPARE(session.activeNoteId(), QStringLiteral("blank-line-selection-note"));
    QVERIFY(session.hasActiveNote());
    QCOMPARE(session.parsedLineCount(), 5);

    const QString selectedLine =
        QStringLiteral("여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.");
    const QString collapsedEditorHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /></head>"
        "<body><p>Intro</p><p>%1</p><p>git checkout main</p></body></html>").arg(selectedLine);
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("underline"),
        collapsedEditorHtml,
        QStringLiteral("Intro\n\n").size(),
        selectedLine.size());

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), QStringLiteral("Intro\n\n").size());
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), selectedLine.size());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Intro\n\n<underline>여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.</underline>\n\ngit checkout main"));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("여러 <underline>에이전트")));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("git </underline>")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_formatsAgainstLoadedBodySourceWhenEditorProjectionDropsRawTags()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("active-source-note"),
        QStringLiteral("<bold>Alpha</bold> Beta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("active-source-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString projectionWithoutRawTags = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("active-source-note"),
        QStringLiteral("Alpha Beta"));
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        projectionWithoutRawTags,
        0,
        QStringLiteral("Alpha").size(),
        QStringLiteral("Alpha"));

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold><highlight>Alpha</highlight></bold> Beta"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_buildsStandaloneResourceSourceInsertion()
{
    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("resource-note"),
        QStringLiteral("Alpha\nBeta"));

    QVariantMap importedResource;
    importedResource.insert(QStringLiteral("resourceId"), QStringLiteral("capture-1"));
    importedResource.insert(
        QStringLiteral("resourcePath"),
        QStringLiteral("Workspace.wsresources/capture-1.wsresource"));
    importedResource.insert(QStringLiteral("type"), QStringLiteral("image"));
    importedResource.insert(QStringLiteral("format"), QStringLiteral(".png"));
    importedResource.insert(QStringLiteral("bucket"), QStringLiteral("Image"));

    const QVariantMap result = session.insertImportedResourcesIntoSource(
        editorHtml,
        5,
        0,
        QVariantList{importedResource});

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(
        result.value(QStringLiteral("insertedText")).toString(),
        QStringLiteral(
            "\n<resource type=\"image\" format=\".png\" "
            "path=\"Workspace.wsresources/capture-1.wsresource\" id=\"capture-1\" />"));
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral(
            "Alpha\n<resource type=\"image\" format=\".png\" "
            "path=\"Workspace.wsresources/capture-1.wsresource\" id=\"capture-1\" />\nBeta"));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("&lt;resource")));
    const QString editorDocumentText = result.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(editorDocumentText.contains(QStringLiteral("Alpha")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource type=&quot;image&quot;")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("Beta")));
    const QString roundTrippedBodySourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("resource-note"),
            editorDocumentText);
    QVERIFY(roundTrippedBodySourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(roundTrippedBodySourceText.contains(QStringLiteral("<resource type=\"image\" format=\".png\"")));
    QVERIFY(roundTrippedBodySourceText.contains(QStringLiteral("Beta")));
    QCOMPARE(session.parsedLineCount(), 3);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_rendersImportedClipboardImageResourceFrame()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardFrameHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-frame-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QImage clipboardImage(QSize(1600, 900), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(18, 110, 190, 255));

    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    clipboard.setCurrentHubPath(hubPath);
    const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
    QVERIFY2(importedEntries.size() == 1, qPrintable(clipboard.lastError()));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-frame-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString editorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap result = session.insertImportedResourcesIntoSource(
        editorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    const QString bodySourceText = result.value(QStringLiteral("bodySourceText")).toString();
    const QString editorDocumentText = result.value(QStringLiteral("editorDocumentText")).toString();
    const QVariantMap importedResource = importedEntries.constFirst().toMap();

    QVERIFY(bodySourceText.contains(QStringLiteral("<resource type=\"image\" format=\".png\"")));
    QVERIFY(bodySourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-figma-node-id=\"292:50\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-resource-type-label")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-resource-file-name")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-resource-preview=\"image-only-frame\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-resource-preview=\"structured-frame\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-resource-preview=\"single-object-raster\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("resourceHeader")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("resourceToolbar")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("whatson-resource-more")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("whatson-resource-type-display")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("whatson-resource-filename-display")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-display-role=\"resource-type\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("data-display-role=\"resource-file-name\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral(">Image<")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral(">...<")));
    QVERIFY(!editorDocumentText.contains(
        QStringLiteral(">%1<").arg(QFileInfo(importedResource.value(QStringLiteral("resourcePath")).toString()).fileName())));
    QVERIFY(!editorDocumentText.contains(QFileInfo(importedResource.value(QStringLiteral("resourcePath")).toString()).fileName()));
    QVERIFY(editorDocumentText.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("<table")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("width=\"960\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("height=\"540\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("max-width:100%")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-display-left=\"0\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-display-top=\"0\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-frame-render-width=\"960\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-media-alignment=\"center\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral(" width=\"480\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("width=\"338\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("height=\"352\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("vertical-align:top")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("object-fit:contain")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("background-color:#1E1F20")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("border:1px")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("border-radius")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-max-width-height-ratio=\"1:1\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("<input")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("<textarea")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("contenteditable")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("font-weight:700")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("cellpadding=\"6\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource")));
    QTextDocument editorPlainDocument;
    editorPlainDocument.setHtml(editorDocumentText);
    const QString resourceObjectLine(QChar::ObjectReplacementCharacter);
    const QString editorPlainText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(editorPlainDocument.toPlainText());
    QVERIFY(editorPlainText.contains(QStringLiteral("Alpha\n") + resourceObjectLine));
    QVERIFY2(
        !editorPlainText.contains(QStringLiteral("\n\n") + resourceObjectLine),
        qPrintable(editorPlainText));

    const QVariantMap reprojectedResult = session.reprojectResourceFramesForEditorWidth(editorDocumentText, 1200);
    QVERIFY(reprojectedResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(reprojectedResult.value(QStringLiteral("changed")).toBool());
    const QString reprojectedEditorText = reprojectedResult.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("width=\"1200\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("height=\"540\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-left=\"120\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-frame-render-width=\"1200\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(!reprojectedEditorText.contains(QStringLiteral("data-display-height=\"675\"")));
    QVERIFY(!reprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"675\"")));

    const QVariantMap narrowerReprojectedResult =
        session.reprojectResourceFramesForEditorWidth(reprojectedEditorText, 720);
    QVERIFY(narrowerReprojectedResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(narrowerReprojectedResult.value(QStringLiteral("changed")).toBool());
    const QString narrowerReprojectedEditorText =
        narrowerReprojectedResult.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(narrowerReprojectedEditorText.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(narrowerReprojectedEditorText.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(narrowerReprojectedEditorText.contains(QStringLiteral("data-display-left=\"-120\"")));
    QVERIFY(narrowerReprojectedEditorText.contains(QStringLiteral("data-frame-render-width=\"720\"")));
    QVERIFY(narrowerReprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(!narrowerReprojectedEditorText.contains(QStringLiteral("data-display-height=\"405\"")));
    QVERIFY(!narrowerReprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"405\"")));
    const QString roundTrippedBodySourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("clipboard-frame-note"),
            editorDocumentText);
    QVERIFY(roundTrippedBodySourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(roundTrippedBodySourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(roundTrippedBodySourceText.contains(QStringLiteral("Beta")));

    QTextDocument richTextDocument;
    richTextDocument.setHtml(editorDocumentText);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(session.editorFilePath(), richTextDocument.toHtml()));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(session.editorFilePath()));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QVERIFY(persistedBodyDocument.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    const QString persistedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument);
    QVERIFY(persistedSourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(persistedSourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(persistedSourceText.contains(QStringLiteral("Beta")));

    QString editedBelowFrameDocumentText = editorDocumentText;
    const QString typedBelowFrameText = QStringLiteral(
        "Typed below frame 01\n"
        "Typed below frame 02\n"
        "Typed below frame 03\n"
        "Typed below frame 04");
    QString typedBelowFrameHtml = typedBelowFrameText.toHtmlEscaped();
    typedBelowFrameHtml.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    QVERIFY(editedBelowFrameDocumentText.replace(
        QStringLiteral("</body>"),
        QStringLiteral("<p>%1</p></body>").arg(typedBelowFrameHtml))
        != editorDocumentText);

    QSignalSpy modifiedPersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(
        session.editorFilePath(),
        1,
        editedBelowFrameDocumentText);
    QTRY_COMPARE_WITH_TIMEOUT(modifiedPersistedSpy.count(), 1, 3000);
    QCOMPARE(modifiedPersistedSpy.takeFirst().at(1).toBool(), true);

    const QString modifiedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    const QString modifiedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(modifiedBodyDocument);
    QVERIFY(modifiedSourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(modifiedSourceText.contains(QStringLiteral("Typed below frame 04")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_keepsImportedResourceWhenLeavingNoteWithStaleSessionFile()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardNavigationHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-navigation-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));
    const QString otherNoteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-navigation-other-note"),
        QStringLiteral("Other"),
        &createError);
    QVERIFY2(!otherNoteDirectoryPath.isEmpty(), qPrintable(createError));

    QString importError;
    const QVariantList importedEntries = importClipboardImageEntriesForNoteEditorSessionTest(hubPath, &importError);
    QVERIFY2(importedEntries.size() == 1, qPrintable(importError));
    const QString resourcePath =
        importedEntries.constFirst().toMap().value(QStringLiteral("resourcePath")).toString();
    QVERIFY(!resourcePath.isEmpty());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-navigation-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        originalEditorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QVERIFY(insertion.value(QStringLiteral("bodySourceText")).toString().contains(resourcePath));

    QVERIFY(writeUtf8FileForNoteEditorSessionTest(session.editorFilePath(), originalEditorHtml));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.openNoteForEditing(
        QStringLiteral("clipboard-navigation-other-note"),
        otherNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    const QString persistedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument);
    QVERIFY(persistedSourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(persistedSourceText.contains(resourcePath));
    QVERIFY(persistedSourceText.contains(QStringLiteral("Beta")));

    QTRY_VERIFY_WITH_TIMEOUT(loadedSpy.count() >= 2, 3000);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-navigation-note"), noteDirectoryPath));
    QTRY_VERIFY_WITH_TIMEOUT(loadedSpy.count() >= 3, 3000);
    const QString remountedEditorSource = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QVERIFY(remountedEditorSource.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(remountedEditorSource.contains(QStringLiteral("whatson-resource-source")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_discardsStalePendingPushWhenImportedResourceIsInserted()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardPendingPushHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-pending-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));
    const QString otherNoteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-pending-other-note"),
        QStringLiteral("Other"),
        &createError);
    QVERIFY2(!otherNoteDirectoryPath.isEmpty(), qPrintable(createError));

    QString importError;
    const QVariantList importedEntries = importClipboardImageEntriesForNoteEditorSessionTest(hubPath, &importError);
    QVERIFY2(importedEntries.size() == 1, qPrintable(importError));
    const QString resourcePath =
        importedEntries.constFirst().toMap().value(QStringLiteral("resourcePath")).toString();
    QVERIFY(!resourcePath.isEmpty());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-pending-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    session.requestEditorModifiedCountRawPush(
        session.editorFilePath(),
        1,
        originalEditorHtml);

    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        originalEditorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QVERIFY(insertion.value(QStringLiteral("bodySourceText")).toString().contains(resourcePath));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.openNoteForEditing(
        QStringLiteral("clipboard-pending-other-note"),
        otherNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    const QString persistedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument);
    QVERIFY(persistedSourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(persistedSourceText.contains(resourcePath));
    QVERIFY(persistedSourceText.contains(QStringLiteral("Beta")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_preservesTextTypedImmediatelyAfterResourceObjectOnRawPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardObjectTextHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-object-text-note"),
        QStringLiteral("이젠 텍스트마저 사라졌다.\n아래에 이미지를 삽입할 것이다."),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QString importError;
    const QVariantList importedEntries = importClipboardImageEntriesForNoteEditorSessionTest(hubPath, &importError);
    QVERIFY2(importedEntries.size() == 1, qPrintable(importError));
    const QString resourcePath =
        importedEntries.constFirst().toMap().value(QStringLiteral("resourcePath")).toString();
    QVERIFY(!resourcePath.isEmpty());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-object-text-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        originalEditorHtml,
        QStringLiteral("이젠 텍스트마저 사라졌다.\n아래에 이미지를 삽입할 것이다.").size(),
        0,
        importedEntries);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());

    const QString focusSyncEditorText =
        QStringLiteral("이젠 텍스트마저 사라졌다.\n"
                       "아래에 이미지를 삽입할 것이다.\n")
        + QString(QChar::ObjectReplacementCharacter)
        + QStringLiteral("그 밑에 텍스트를 입력했다.\n"
                         "그러나 하단의 텍스트에 대해서는 거터가 할당되지 않고 논리적으로 무시된다.\n"
                         "그 이후의 텍스트들에 대해서는 거터 라인 넘버가 한 칸씩 밀린 체로 생성되지만\n"
                         "정체불명의 배경색 프레임이 함께 존재하게 되었다.");

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(session.editorFilePath(), focusSyncEditorText);
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    const QString persistedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument);
    const QString expectedSourceText =
        QStringLiteral("이젠 텍스트마저 사라졌다.\n"
                       "아래에 이미지를 삽입할 것이다.\n")
        + insertion.value(QStringLiteral("bodySourceText")).toString().section(QLatin1Char('\n'), 2, 2)
        + QStringLiteral("\n그 밑에 텍스트를 입력했다.\n"
                         "그러나 하단의 텍스트에 대해서는 거터가 할당되지 않고 논리적으로 무시된다.\n"
                         "그 이후의 텍스트들에 대해서는 거터 라인 넘버가 한 칸씩 밀린 체로 생성되지만\n"
                         "정체불명의 배경색 프레임이 함께 존재하게 되었다.");

    QCOMPARE(persistedSourceText, expectedSourceText);
    QVERIFY(persistedSourceText.contains(resourcePath));
    QCOMPARE(session.parsedLineCount(), 7);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_rejectsTransientEmptyEditorTextBeforeRawPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        QStringLiteral("empty-raw-guard-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("empty-raw-guard-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(session.editorFilePath(), QString());
    QTest::qWait(50);
    QCOMPARE(persistedSpy.count(), 0);

    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 1, QString());
    QTest::qWait(50);
    QCOMPARE(persistedSpy.count(), 0);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha\nBeta"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_rejectsEmptyRichTextSessionPayloadBeforeRawPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("empty-rich-text-guard-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        noteId,
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString emptyEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        noteId,
        QString());
    QVERIFY(!emptyEditorHtml.trimmed().isEmpty());
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(noteId, emptyEditorHtml),
        QString());

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(session.editorFilePath(), emptyEditorHtml);
    QVERIFY(session.clearEditor());
    QTest::qWait(50);
    QCOMPARE(persistedSpy.count(), 0);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha\nBeta"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_stagesStyleInsertionInSessionFileBeforeRawPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("style-session-stage-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        noteId,
        QStringLiteral("Alpha Beta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    const QVariantMap insertion = session.insertStyleTagIntoSource(
        QStringLiteral("Title"),
        originalEditorHtml,
        0,
        0);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        insertion.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>"));

    const QString stagedEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QVERIFY(stagedEditorHtml.contains(QStringLiteral("whatson-style-source")));
    QVERIFY(stagedEditorHtml.contains(QStringLiteral("font-size:26px;")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(noteId, stagedEditorHtml),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>"));

    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);
    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>"));

    const QString plainEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        noteId,
        QStringLiteral("Alpha Beta"));
    QVERIFY(!plainEditorHtml.contains(QStringLiteral("whatson-style-source")));

    QSignalSpy plainRoundTripPersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 1, plainEditorHtml);
    QTRY_COMPARE_WITH_TIMEOUT(plainRoundTripPersistedSpy.count(), 1, 3000);
    QCOMPARE(plainRoundTripPersistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedAfterPlainRoundTrip = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedAfterPlainRoundTrip),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_enterInsideStyleMovesCursorOutside()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("style-enter-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        noteId,
        QStringLiteral("Alpha Beta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QSignalSpy stylePersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    const QVariantMap insertion = session.insertStyleTagIntoSource(
        QStringLiteral("Title"),
        originalEditorHtml,
        QStringLiteral("Alpha Beta").size(),
        0);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QTRY_COMPARE_WITH_TIMEOUT(stylePersistedSpy.count(), 1, 3000);
    QCOMPARE(stylePersistedSpy.takeFirst().at(1).toBool(), true);

    const QVariantMap enterResult = session.handleStyleBoundaryKeyInSource(
        insertion.value(QStringLiteral("editorDocumentText")).toString(),
        QStringLiteral("Alpha Beta").size(),
        0,
        Qt::Key_Return);
    QVERIFY(enterResult.value(QStringLiteral("handled")).toBool());
    QVERIFY(enterResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        enterResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>\n"));
    QCOMPARE(
        enterResult.value(QStringLiteral("sourceCursorPosition")).toInt(),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>\n").size());
    QVERIFY(enterResult.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("whatson-style-source")));
    QVERIFY(enterResult.value(QStringLiteral("editorDocumentText")).toString().contains(
        QStringLiteral("font-size:26px;")));

    const QString plainEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        noteId,
        QStringLiteral("Alpha Beta\n"));
    QVERIFY(!plainEditorHtml.contains(QStringLiteral("whatson-style-source")));

    QSignalSpy enterPersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 1, plainEditorHtml);
    QTRY_COMPARE_WITH_TIMEOUT(enterPersistedSpy.count(), 1, 3000);
    QCOMPARE(enterPersistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>\n"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_boundaryEnterUsesCurrentEditorSnapshotAfterBackspace()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QStringList sourceLines;
    for (int lineNumber = 1; lineNumber < 10; ++lineNumber)
    {
        sourceLines.push_back(QStringLiteral("Line %1").arg(lineNumber));
    }
    sourceLines.push_back(QStringLiteral("<style style=\"Title\">Line 10 \\</style>"));

    const QString noteId = QStringLiteral("style-backspace-enter-note");
    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        noteId,
        sourceLines.join(QLatin1Char('\n')),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    QStringList editorLines = sourceLines;
    editorLines.last() = QStringLiteral("<style style=\"Title\">Line 10 </style>");
    const QString editorHtmlAfterBackspace = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        noteId,
        editorLines.join(QLatin1Char('\n')));
    const int cursorAfterBackspace = QStringLiteral(
        "Line 1\n"
        "Line 2\n"
        "Line 3\n"
        "Line 4\n"
        "Line 5\n"
        "Line 6\n"
        "Line 7\n"
        "Line 8\n"
        "Line 9\n"
        "Line 10 ").size();

    const QVariantMap enterResult = session.handleStyleBoundaryKeyInSource(
        editorHtmlAfterBackspace,
        cursorAfterBackspace,
        0,
        Qt::Key_Return);

    QVERIFY(enterResult.value(QStringLiteral("handled")).toBool());
    QVERIFY(enterResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(!enterResult.value(QStringLiteral("bodySourceText")).toString().contains(QLatin1Char('\\')));

    editorLines.last() = QStringLiteral("<style style=\"Title\">Line 10 </style>\n");
    QCOMPARE(
        enterResult.value(QStringLiteral("bodySourceText")).toString(),
        editorLines.join(QLatin1Char('\n')));

    QStringList calloutSourceLines;
    for (int lineNumber = 1; lineNumber < 10; ++lineNumber)
    {
        calloutSourceLines.push_back(QStringLiteral("Callout line %1").arg(lineNumber));
    }
    calloutSourceLines.push_back(QStringLiteral("<callout>Line 10 \\</callout>"));

    const QString calloutNoteId = QStringLiteral("callout-backspace-enter-note");
    const QString calloutNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        calloutNoteId,
        calloutSourceLines.join(QLatin1Char('\n')),
        &createError);
    QVERIFY2(!calloutNoteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession calloutSession;
    calloutSession.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy calloutLoadedSpy(&calloutSession, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(calloutSession.openNoteForEditing(calloutNoteId, calloutNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(calloutLoadedSpy.count(), 1, 3000);
    QVERIFY(calloutSession.markEditorSessionFileReadyForRawPush(calloutSession.editorFilePath()));

    QStringList calloutEditorLines = calloutSourceLines;
    calloutEditorLines.last() = QStringLiteral("<callout>Line 10 </callout>");
    const QString calloutEditorHtmlAfterBackspace = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        calloutNoteId,
        calloutEditorLines.join(QLatin1Char('\n')));
    QTextDocument calloutEditorDocument;
    calloutEditorDocument.setHtml(calloutEditorHtmlAfterBackspace);
    const QString calloutEditorPlainText = calloutEditorDocument.toPlainText();
    const int calloutCursorAfterBackspace =
        calloutEditorPlainText.indexOf(QStringLiteral("Line 10 "))
        + QStringLiteral("Line 10 ").size();

    const QVariantMap calloutEnterResult = calloutSession.handleCalloutBoundaryKeyInSource(
        calloutEditorHtmlAfterBackspace,
        calloutCursorAfterBackspace,
        0,
        Qt::Key_Return);

    QVERIFY(calloutEnterResult.value(QStringLiteral("handled")).toBool());
    QVERIFY(calloutEnterResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(!calloutEnterResult.value(QStringLiteral("bodySourceText")).toString().contains(QLatin1Char('\\')));

    calloutEditorLines.last() = QStringLiteral("<callout>Line 10</callout>\n");
    QCOMPARE(
        calloutEnterResult.value(QStringLiteral("bodySourceText")).toString(),
        calloutEditorLines.join(QLatin1Char('\n')));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversNativeEmptyLineInsertion()
{
    const auto recoveredSourceAfterNativeEmptyLine =
        [](const QString& noteId, const QString& sourceText) -> QString
        {
            const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(noteId, sourceText);
            QTextDocument editorDocument;
            editorDocument.setHtml(editorHtml);
            QTextCursor editorCursor(&editorDocument);
            editorCursor.movePosition(QTextCursor::End);
            editorCursor.insertBlock();
            return WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
                noteId,
                editorDocument.toHtml());
        };

    QCOMPARE(
        recoveredSourceAfterNativeEmptyLine(
            QStringLiteral("plain-native-empty-line-note"),
            QStringLiteral("Alpha\n")),
        QStringLiteral("Alpha\n\n"));
    QCOMPARE(
        recoveredSourceAfterNativeEmptyLine(
            QStringLiteral("style-native-empty-line-note"),
            QStringLiteral("<style style=\"Title\">Title</style>\n")),
        QStringLiteral("<style style=\"Title\">Title</style>\n\n"));
    QCOMPARE(
        recoveredSourceAfterNativeEmptyLine(
            QStringLiteral("callout-native-empty-line-note"),
            QStringLiteral("<callout>Inside</callout>\n")),
        QStringLiteral("<callout>Inside</callout>\n\n"));
    const QString mixedSourceText =
        QStringLiteral("첫 번째 줄에 텍스트 입력\n"
                       "세 번째 줄에 이미지를 놓을 것이다.\n"
                       "무슨 여기에 텍스트를 입력하면, 아 되는구\n"
                       "<resource type=\"image\" format=\".png\" path=\"untitled.wsresources/capture.wsresource\" id=\"capture\" />\n"
                       "그 밑에<bold> 다시 텍스트를 두었다 </bold>\n"
                       "<style style=\"Title\">\n"
                       "타이틀</style>\n"
                       "텍스트 입력 <style font=\"American Typewriter\">이 부분의 폰트는 바뀌었다</style> \n"
                       "아래로 줄 이동 <style font=\"Apple Color Emoji\">여기도 폰트가 다르다</style>\n");
    QCOMPARE(
        recoveredSourceAfterNativeEmptyLine(
            QStringLiteral("mixed-native-empty-line-note"),
            mixedSourceText),
        mixedSourceText + QLatin1Char('\n'));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_reprojectPreservesNativeEmptyLineAfterCallout()
{
    const QString sourceText = QStringLiteral("<callout>Inside</callout>\n");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-native-empty-line-note"),
        sourceText,
        142);

    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QTextCursor editorCursor(&editorDocument);
    editorCursor.movePosition(QTextCursor::End);
    editorCursor.insertBlock();

    NoteEditorDocumentSession session;
    const QVariantMap result = session.reprojectResourceFramesForEditorWidth(
        editorDocument.toHtml(),
        142);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<callout>Inside</callout>\n\n"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_preservesStyleBoundariesDuringPlainWhitespaceRawPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("style-plain-whitespace-note");
    const QString fontSource =
        QStringLiteral("Alpha <style font=\"American Typewriter\">Styled</style>Omega");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        noteId,
        fontSource,
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString shiftedFontEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        noteId,
        QStringLiteral("Alpha <style font=\"American Typewriter\">Styled </style>Omega"));
    QSignalSpy fontPersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 1, shiftedFontEditorHtml);
    QTRY_COMPARE_WITH_TIMEOUT(fontPersistedSpy.count(), 1, 3000);
    QCOMPARE(fontPersistedSpy.takeFirst().at(1).toBool(), true);

    QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha <style font=\"American Typewriter\">Styled</style> Omega"));

    const QString titleNoteId = QStringLiteral("style-plain-whitespace-title-note");
    const QString titleSource = QStringLiteral("<style style=\"Title\">Title</style>");
    const QString titleNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        titleNoteId,
        titleSource,
        &createError);
    QVERIFY2(!titleNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QSignalSpy reloadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(titleNoteId, titleNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(reloadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString shiftedTitleEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        titleNoteId,
        QStringLiteral("<style style=\"Title\">Title\n</style>"));
    QSignalSpy titlePersistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 2, shiftedTitleEditorHtml);
    QTRY_COMPARE_WITH_TIMEOUT(titlePersistedSpy.count(), 1, 3000);
    QCOMPARE(titlePersistedSpy.takeFirst().at(1).toBool(), true);

    persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(titleNoteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("<style style=\"Title\">Title</style>\n"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_resetsRawPushReadinessWhenReopeningSessionFile()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString firstNoteId = QStringLiteral("reopen-readiness-first-note");
    const QString firstNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        firstNoteId,
        QStringLiteral("Alpha body"),
        &createError);
    QVERIFY2(!firstNoteDirectoryPath.isEmpty(), qPrintable(createError));

    const QString secondNoteId = QStringLiteral("reopen-readiness-second-note");
    const QString secondNoteDirectoryPath = createLocalNoteForRegression(
        workspaceDirectory.path(),
        secondNoteId,
        QStringLiteral("Beta body"),
        &createError);
    QVERIFY2(!secondNoteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(firstNoteId, firstNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    const QString firstEditorFilePath = session.editorFilePath();
    QVERIFY(session.markEditorSessionFileReadyForRawPush(firstEditorFilePath));

    QVERIFY(session.openNoteForEditing(secondNoteId, secondNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 2, 3000);

    QVERIFY(session.openNoteForEditing(firstNoteId, firstNoteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 3, 3000);
    QCOMPARE(session.editorFilePath(), firstEditorFilePath);

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorIdleRawPush(
        session.editorFilePath(),
        QStringLiteral("Stale editor payload before fresh readFinished"));
    QVERIFY(session.clearEditor());
    QTest::qWait(50);
    QCOMPARE(persistedSpy.count(), 0);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(firstNoteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha body"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_persistsStagedRawResourceAfterTransientEmptyPush()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardRawGuardHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-raw-guard-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QString importError;
    const QVariantList importedEntries = importClipboardImageEntriesForNoteEditorSessionTest(hubPath, &importError);
    QVERIFY2(importedEntries.size() == 1, qPrintable(importError));
    const QString resourcePath =
        importedEntries.constFirst().toMap().value(QStringLiteral("resourcePath")).toString();
    QVERIFY(!resourcePath.isEmpty());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-raw-guard-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        originalEditorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QVERIFY(insertion.value(QStringLiteral("bodySourceText")).toString().contains(resourcePath));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    session.requestEditorModifiedCountRawPush(session.editorFilePath(), 1, QString());
    QTest::qWait(50);
    QCOMPARE(persistedSpy.count(), 0);

    QVERIFY(session.clearEditor());
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    const QString persistedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument);
    QVERIFY(persistedSourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(persistedSourceText.contains(resourcePath));
    QVERIFY(persistedSourceText.contains(QStringLiteral("Beta")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_mergesIdlePullDiffIntoDirtySession()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardDirtyPullGuardHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteId = QStringLiteral("clipboard-dirty-pull-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        noteId,
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QString importError;
    const QVariantList importedEntries = importClipboardImageEntriesForNoteEditorSessionTest(hubPath, &importError);
    QVERIFY2(importedEntries.size() == 1, qPrintable(importError));
    const QString resourcePath =
        importedEntries.constFirst().toMap().value(QStringLiteral("resourcePath")).toString();
    QVERIFY(!resourcePath.isEmpty());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());
    session.setEditorViewportWidth(960);

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(noteId, noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QVERIFY(session.markEditorSessionFileReadyForRawPush(session.editorFilePath()));

    const QString originalEditorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        originalEditorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);
    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    QVERIFY(insertion.value(QStringLiteral("bodySourceText")).toString().contains(resourcePath));

    QString updateError;
    QVERIFY2(
        writeFilesystemBodyForIdlePullSessionTest(
            noteId,
            noteDirectoryPath,
            QStringLiteral("Alpha\nRemote filesystem line\nBeta"),
            QStringLiteral("2099-05-01-12-00-00"),
            &updateError),
        qPrintable(updateError));

    QSignalSpy pulledSpy(&session, &NoteEditorDocumentSession::editorDocumentTextPulled);
    QSignalSpy ignoredSpy(&session, &NoteEditorDocumentSession::editorFilesystemPullIgnored);
    QVERIFY(session.requestActiveNoteIdleRawPull() != 0);
    QTRY_COMPARE_WITH_TIMEOUT(pulledSpy.count(), 1, 3000);
    QCOMPARE(ignoredSpy.count(), 0);

    const QString mergedEditorText = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QVERIFY(mergedEditorText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(mergedEditorText.contains(QStringLiteral("Remote filesystem line")));

    const QString mergedSourceText =
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(noteId, mergedEditorText);
    QVERIFY(mergedSourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(mergedSourceText.contains(QStringLiteral("Remote filesystem line")));
    QVERIFY(mergedSourceText.contains(resourcePath));
    QVERIFY(mergedSourceText.contains(QStringLiteral("Beta")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_reprojectsCalloutFrameChromeOnTextChange()
{
    const QString shortSource = QStringLiteral("<callout>Short callout</callout>");
    const QString longContent = QStringLiteral(
        "Callout text should grow the leading indicator as the user keeps typing inside the frame. "
        "This deliberately long sentence wraps across several narrow editor rows.");
    const QString shortEditorText = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-reproject-note"),
        shortSource,
        142);
    const int shortChromeHeight = calloutChromeHeightForNoteEditorSessionTest(shortEditorText);
    QVERIFY(shortChromeHeight >= 14);

    QString editedEditorText = shortEditorText;
    QVERIFY(editedEditorText.replace(
        QStringLiteral("Short callout<!--/whatson-callout-content-->"),
        longContent + QStringLiteral("<!--/whatson-callout-content-->")) != shortEditorText);

    NoteEditorDocumentSession session;
    const QVariantMap result = session.reprojectResourceFramesForEditorWidth(editedEditorText, 142);
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QVERIFY(result.value(QStringLiteral("changed")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<callout>") + longContent + QStringLiteral("</callout>"));

    const QString reprojectedEditorText = result.value(QStringLiteral("editorDocumentText")).toString();
    const int reprojectedChromeHeight = calloutChromeHeightForNoteEditorSessionTest(reprojectedEditorText);
    QVERIFY2(
        reprojectedChromeHeight >= shortChromeHeight + 24,
        qPrintable(QStringLiteral("Callout chrome height must grow with edited wrapped text. before=%1 after=%2")
            .arg(shortChromeHeight)
            .arg(reprojectedChromeHeight)));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_persistsBackspacedResourceFrameAsComponentDeletion()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("resource-frame-delete-note"),
        QStringLiteral("Alpha\n%1\nBeta").arg(resourceTag),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("resource-frame-delete-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString mountedEditorSource = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    QVERIFY(mountedEditorSource.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("data-resource-file-name")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("capture.wsresource")));

    QTextDocument mountedEditorDocument;
    mountedEditorDocument.setHtml(mountedEditorSource);
    const QString mountedPlainText = mountedEditorDocument.toPlainText();
    QVERIFY(mountedPlainText.contains(QChar::ObjectReplacementCharacter));
    QVERIFY(!mountedPlainText.contains(QStringLiteral("Image")));
    QVERIFY(!mountedPlainText.contains(QStringLiteral("capture.wsresource")));

    QString editorTextAfterSingleObjectDeletion = mountedPlainText;
    editorTextAfterSingleObjectDeletion.remove(QChar::ObjectReplacementCharacter);
    editorTextAfterSingleObjectDeletion.replace(QStringLiteral("\n\n"), QStringLiteral("\n"));
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(
        session.editorFilePath(),
        editorTextAfterSingleObjectDeletion));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(session.editorFilePath()));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);
    QCOMPARE(session.parsedLineCount(), 2);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha\nBeta"));
    QVERIFY(!persistedBodyDocument.contains(QStringLiteral("<resource")));
    QVERIFY(!persistedBodyDocument.contains(QStringLiteral("capture.wsresource")));
}
