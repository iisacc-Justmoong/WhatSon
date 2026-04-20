#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteFileStatSupport_incrementsOpenCountAndPersistsLastOpenedAt()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("selection-note"),
        QStringLiteral("selection body"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    const QString headerPath = WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath);
    QVERIFY(QFileInfo::exists(headerPath));

    const QDateTime beforeIncrementUtc = QDateTime::currentDateTimeUtc();
    QString incrementError;
    QVERIFY2(
        WhatSon::NoteFileStatSupport::incrementOpenCountForNoteHeader(
            QStringLiteral("selection-note"),
            noteDirectoryPath,
            &incrementError),
        qPrintable(incrementError));

    QFile headerFile(headerPath);
    QVERIFY(headerFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    QVERIFY(headerText.contains(QStringLiteral("<lastOpened>")));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &headerStore, &parseError), qPrintable(parseError));

    QCOMPARE(headerStore.openCount(), 1);
    QCOMPARE(headerStore.lastModifiedAt(), QStringLiteral("2026-04-18-00-00-00"));
    QVERIFY(!headerStore.lastOpenedAt().isEmpty());

    const QDateTime lastOpenedUtc = QDateTime::fromString(headerStore.lastOpenedAt(), Qt::ISODate).toUTC();
    QVERIFY(lastOpenedUtc.isValid());
    QVERIFY(lastOpenedUtc >= beforeIncrementUtc.addSecs(-1));
    QVERIFY(lastOpenedUtc <= QDateTime::currentDateTimeUtc().addSecs(1));
}
