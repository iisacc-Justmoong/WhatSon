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
    QVERIFY(mountedEditorSource.contains(QStringLiteral("Visible line<br/>")));
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
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(
        session.editorFilePath(),
        QStringLiteral("After departure push")));

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
    QCOMPARE(mountedEditorSource, QStringLiteral("Alpha<br/><br/>Beta"));
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
    QVERIFY(editorDocumentText.contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource type=&quot;image&quot;")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("<br/>Beta")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("resource-note"),
            editorDocumentText),
        result.value(QStringLiteral("bodySourceText")).toString());
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
    QVERIFY(editorDocumentText.contains(QStringLiteral("<table")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("width=\"100%\"")));
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
    QVERIFY(editorDocumentText.contains(QStringLiteral("height:auto")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("object-fit:contain")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("data-max-width-height-ratio=\"1:1\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("<input")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("<textarea")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("contenteditable")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("font-weight:700")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("cellpadding=\"6\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource")));

    const QVariantMap reprojectedResult = session.reprojectResourceFramesForEditorWidth(editorDocumentText, 1200);
    QVERIFY(reprojectedResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(reprojectedResult.value(QStringLiteral("changed")).toBool());
    const QString reprojectedEditorText = reprojectedResult.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("height:auto")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-display-left=\"120\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-frame-render-width=\"1200\"")));
    QVERIFY(reprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(!reprojectedEditorText.contains(QStringLiteral("data-display-height=\"675\"")));
    QVERIFY(!reprojectedEditorText.contains(QStringLiteral("data-frame-display-height=\"675\"")));
    QVERIFY(!reprojectedEditorText.contains(QStringLiteral(" width=\"1200\"")));

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
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("clipboard-frame-note"),
            editorDocumentText),
        bodySourceText);

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
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        bodySourceText);
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
