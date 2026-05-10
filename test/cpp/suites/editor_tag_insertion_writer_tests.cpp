#include "test/cpp/whatson_cpp_regression_tests.hpp"

namespace
{
    QString readUtf8FileForTagInsertionWriterTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }
} // namespace

void WhatSonCppRegressionTests::editorTagInsertionWriter_writesHeaderTagIntoLocalWsnbody()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("tag-writer-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    TagInsertionWriter writer;
    QSignalSpy finishedSpy(&writer, &TagInsertionWriter::tagWriteFinished);
    const QVariantMap result = writer.insertNamedTagIntoNote(
        QStringLiteral("header"),
        QStringLiteral("tag-writer-note"),
        noteDirectoryPath,
        0,
        5);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QVERIFY(result.value(QStringLiteral("changed")).toBool());
    QCOMPARE(result.value(QStringLiteral("tagName")).toString(), QStringLiteral("header"));
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<header>Alpha</header>\nBeta"));
    QCOMPARE(
        result.value(QStringLiteral("cursorPosition")).toInt(),
        QStringLiteral("<header>Alpha</header>").size());
    QCOMPARE(finishedSpy.count(), 1);
    QVERIFY(finishedSpy.takeFirst().at(0).toMap().value(QStringLiteral("valid")).toBool());

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    const QString persistedBodyDocument = readUtf8FileForTagInsertionWriterTest(bodyPath);
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph><header>Alpha</header></paragraph>")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("<header>Alpha</header>\nBeta"));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = QStringLiteral("tag-writer-note");
    readRequest.noteDirectoryPath = noteDirectoryPath;
    WhatSonLocalNoteDocument persistedDocument;
    QString readError;
    QVERIFY2(fileStore.readNote(readRequest, &persistedDocument, &readError), qPrintable(readError));
    QCOMPARE(persistedDocument.headerStore.modifiedCount(), 1);
}

void WhatSonCppRegressionTests::editorTagInsertionWriter_writesStandaloneResourceAsBodyNode()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("resource-writer-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    TagInsertionWriter writer;
    const QVariantMap result = writer.insertNamedTagIntoNote(
        QStringLiteral("resource"),
        QStringLiteral("resource-writer-note"),
        noteDirectoryPath,
        5,
        0);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha\n<resource />\nBeta"));

    const QString persistedBodyDocument = readUtf8FileForTagInsertionWriterTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("    <resource />\n")));
    QVERIFY(!persistedBodyDocument.contains(QStringLiteral("<paragraph><resource />")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        QStringLiteral("Alpha\n<resource />\nBeta"));
}

void WhatSonCppRegressionTests::editorTagInsertionWriter_rejectsUnsupportedTagWithoutChangingBody()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("rejected-tag-note"),
        QStringLiteral("Alpha"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    const QString bodyBefore = readUtf8FileForTagInsertionWriterTest(bodyPath);

    TagInsertionWriter writer;
    QSignalSpy finishedSpy(&writer, &TagInsertionWriter::tagWriteFinished);
    const QVariantMap result = writer.insertNamedTagIntoNote(
        QStringLiteral("script"),
        QStringLiteral("rejected-tag-note"),
        noteDirectoryPath,
        0,
        0);

    QVERIFY(!result.value(QStringLiteral("valid")).toBool());
    QVERIFY(!result.value(QStringLiteral("changed")).toBool());
    QVERIFY(result.value(QStringLiteral("errorMessage")).toString().contains(
        QStringLiteral("Unsupported static body tag")));
    QCOMPARE(readUtf8FileForTagInsertionWriterTest(bodyPath), bodyBefore);
    QCOMPARE(finishedSpy.count(), 1);
    QVERIFY(!finishedSpy.takeFirst().at(0).toMap().value(QStringLiteral("valid")).toBool());
}
