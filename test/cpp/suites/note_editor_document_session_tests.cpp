#include "test/cpp/whatson_cpp_regression_tests.hpp"

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
} // namespace

void WhatSonCppRegressionTests::noteEditorDocumentSession_mountsParsedSourceFileAndPersistsBodyDocument()
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
    QVERIFY(!session.readOnly());

    const QString mountedEditorSource = readUtf8FileForNoteEditorSessionTest(editorFilePath);
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<?xml")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<contents")));
    QCOMPARE(
        mountedEditorSource,
        QStringLiteral("Visible line\n<resource path=\"asset.png\" />"));

    const QString editedSource = QStringLiteral("Changed line\n<resource path=\"asset.png\" />");
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, editedSource));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(editorFilePath));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(bodyPath);
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<contents id=\"session-note\">")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph>Changed line</paragraph>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<resource path=\"asset.png\" />")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        editedSource);
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
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, QStringLiteral("Unsaved session line")));

    QVERIFY(session.openNoteForEditing(QStringLiteral("reselected-note"), noteDirectoryPath));
    QCOMPARE(session.editorFilePath(), editorFilePath);
    QCOMPARE(readUtf8FileForNoteEditorSessionTest(editorFilePath), QStringLiteral("Unsaved session line"));
    QCOMPARE(loadedSpy.count(), 1);
}
