#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::unusedNoteSensors_filterNoteIdsByLastOpenedWindow()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));

    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    const auto rewriteHeader = [](
                                   const QString& noteDirectoryPath,
                                   const QString& createdAt,
                                   const QString& lastModifiedAt,
                                   const QString& lastOpenedAt,
                                   const int openCount)
    {
        const QString headerPath = WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath);
        QFile headerFile(headerPath);
        if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }

        const QString headerText = QString::fromUtf8(headerFile.readAll());
        headerFile.close();

        WhatSonNoteHeaderStore headerStore;
        WhatSonNoteHeaderParser parser;
        QString parseError;
        if (!parser.parse(headerText, &headerStore, &parseError))
        {
            return false;
        }

        headerStore.setCreatedAt(createdAt);
        headerStore.setLastModifiedAt(lastModifiedAt);
        headerStore.setLastOpenedAt(lastOpenedAt);
        headerStore.setOpenCount(openCount);

        WhatSonNoteHeaderCreator creator(noteDirectoryPath, QString());
        QFile writeFile(headerPath);
        if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }

        const QByteArray encoded = creator.createHeaderText(headerStore).toUtf8();
        const bool success = writeFile.write(encoded) == encoded.size();
        writeFile.close();
        return success;
    };

    QString createError;
    const QString monthlyOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("monthly-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!monthlyOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        monthlyOldNoteDirectoryPath,
        nowUtc.addDays(-90).toString(Qt::ISODate),
        nowUtc.addDays(-60).toString(Qt::ISODate),
        nowUtc.addDays(-40).toString(Qt::ISODate),
        4));

    createError.clear();
    const QString weeklyOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("weekly-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!weeklyOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        weeklyOldNoteDirectoryPath,
        nowUtc.addDays(-20).toString(Qt::ISODate),
        nowUtc.addDays(-10).toString(Qt::ISODate),
        nowUtc.addDays(-8).toString(Qt::ISODate),
        2));

    createError.clear();
    const QString recentNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("recent-note"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!recentNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        recentNoteDirectoryPath,
        nowUtc.addDays(-6).toString(Qt::ISODate),
        nowUtc.addDays(-4).toString(Qt::ISODate),
        nowUtc.addDays(-2).toString(Qt::ISODate),
        7));

    createError.clear();
    const QString neverOpenedOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("never-opened-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!neverOpenedOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        neverOpenedOldNoteDirectoryPath,
        nowUtc.addDays(-50).toString(Qt::ISODate),
        nowUtc.addDays(-45).toString(Qt::ISODate),
        QString(),
        0));

    createError.clear();
    const QString neverOpenedRecentNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("never-opened-recent"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!neverOpenedRecentNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        neverOpenedRecentNoteDirectoryPath,
        nowUtc.addDays(-3).toString(Qt::ISODate),
        nowUtc.addDays(-2).toString(Qt::ISODate),
        QString(),
        0));

    WeeklyUnusedNote weeklySensor;
    MonthlyUnusedNote monthlySensor;
    QSignalSpy weeklyChangedSpy(&weeklySensor, &WeeklyUnusedNote::unusedNotesChanged);
    QSignalSpy monthlyChangedSpy(&monthlySensor, &MonthlyUnusedNote::unusedNotesChanged);

    weeklySensor.setHubPath(hubPath);
    monthlySensor.setHubPath(hubPath);

    QCOMPARE(weeklySensor.lastError(), QString());
    QCOMPARE(monthlySensor.lastError(), QString());
    const QStringList expectedWeeklyNoteIds{
        QStringLiteral("monthly-old"),
        QStringLiteral("never-opened-old"),
        QStringLiteral("weekly-old")
    };
    const QStringList expectedMonthlyNoteIds{
        QStringLiteral("monthly-old"),
        QStringLiteral("never-opened-old")
    };
    QCOMPARE(weeklySensor.unusedNoteIds(), expectedWeeklyNoteIds);
    QCOMPARE(monthlySensor.unusedNoteIds(), expectedMonthlyNoteIds);
    QCOMPARE(weeklySensor.unusedNoteCount(), 3);
    QCOMPARE(monthlySensor.unusedNoteCount(), 2);
    QCOMPARE(weeklyChangedSpy.count(), 1);
    QCOMPARE(monthlyChangedSpy.count(), 1);

    const QVariantMap weeklyNeverOpenedEntry = weeklySensor.unusedNotes().at(1).toMap();
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("never-opened-old"));
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("activitySource")).toString(), QStringLiteral("createdAt"));
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("lastOpenedAt")).toString(), QString());
}
