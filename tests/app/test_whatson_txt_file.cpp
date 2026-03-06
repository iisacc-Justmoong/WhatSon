#include "txt/WhatSonTxtFileCreator.hpp"
#include "txt/WhatSonTxtFileParser.hpp"
#include "txt/WhatSonTxtFileStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

namespace
{
    QString readUtf8File(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }
} // namespace

class WhatSonTxtFileTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void store_sanitizesAssignedValues();
    void parser_normalizesPlainTextBody();
    void creator_createsUniqueTxtFilesInsideLibraryRoot();
};

void WhatSonTxtFileTest::store_sanitizesAssignedValues()
{
    WhatSonTxtFileStore store;
    store.setFilePath(QStringLiteral("  /tmp/Example.txt  "));
    store.setBodyPlainText(QStringLiteral("\r\nAlpha\r\nBeta\r\n"));

    QCOMPARE(store.filePath(), QStringLiteral("/tmp/Example.txt"));
    QCOMPARE(store.bodyPlainText(), QStringLiteral("Alpha\nBeta"));
}

void WhatSonTxtFileTest::parser_normalizesPlainTextBody()
{
    WhatSonTxtFileParser parser;
    WhatSonTxtFileStore store;
    QString errorMessage;

    QVERIFY2(
        parser.parse(QStringLiteral("\ufeff\r\n  \r\nAlpha line\r\nBeta line\r\n"), &store, &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(store.bodyPlainText(), QStringLiteral("Alpha line\nBeta line"));
}

void WhatSonTxtFileTest::creator_createsUniqueTxtFilesInsideLibraryRoot()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString libraryRootPath = QDir(tempDir.path()).filePath(QStringLiteral("Dynamic.wslibrary"));
    WhatSonTxtFileCreator creator(libraryRootPath);

    QString firstFilePath;
    QString errorMessage;
    QVERIFY2(creator.createFile(QString(), QStringLiteral("Alpha"), &firstFilePath, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(firstFilePath.endsWith(QStringLiteral(".txt")));
    QCOMPARE(QFileInfo(firstFilePath).absolutePath(), libraryRootPath);
    QCOMPARE(readUtf8File(firstFilePath), QStringLiteral("Alpha"));

    QString secondFilePath;
    QVERIFY2(creator.createFile(QString(), QString(), &secondFilePath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(secondFilePath.endsWith(QStringLiteral(".txt")));
    QVERIFY(firstFilePath != secondFilePath);
    QVERIFY(QFileInfo(secondFilePath).isFile());
}

QTEST_APPLESS_MAIN(WhatSonTxtFileTest)

#include "test_whatson_txt_file.moc"
